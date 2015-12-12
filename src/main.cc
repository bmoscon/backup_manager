/* Backup Manager Entry Point
 * 
 * Copyright (c) 2012-2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 09/20/2014 - Initial open source release
 * 09/26/2014 - Logger Support in config file
 *            - Add logging in main
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>

#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#include "config_parse.hpp"
#include "defines.hpp"
#include "logger.hpp"
#include "backup_manager.hpp"


#define LOCK_FILE "/var/run/backup_manager.pid"


static void usage();
static void send_stop();
static bool lock_file(int&);
static void stop_handler(int);
static void set_state(const manager_state_e);
static void worker_function(thread_data_st, int);
static bool in_window(const std::string&, const std::string&);


manager_state_e state = RUN;
std::mutex lock;
std::string log_path;


int main(int argc, char* argv[])
{
    pid_t pid = 0;
    pid_t sid = 0;
    int fd = 0;
    
    if (argc < 2 || argc > 3 || !strcmp(argv[1], "help")) {
	usage();
	return (EXIT_SUCCESS);
    }
    
    if (!strcmp(argv[1], "stop")) {
	send_stop();
	return (EXIT_SUCCESS);
    } else if ((strcmp(argv[1], "start") != 0) || (argc != 3)) {
	usage();
	return (EXIT_SUCCESS);
    }
    
    // fork a child and kill the parent
    if ((pid = fork()) < 0) {
	perror("fork failed");
	exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
	// parent process - kill to make child a daemon
	exit(EXIT_SUCCESS);
    }
    
    umask(0);
    
    if ((sid = setsid()) < 0) {
	perror("setsid failed");
	exit(EXIT_FAILURE);
    }
    
    // check if another instance is running. If not, lock the pid file
    if(!lock_file(fd)) {
	std::cerr << "Backup Manager already running" << std::endl;
	exit(EXIT_SUCCESS);
    }
    
    // set up a signal handler so we can respond to stop commands
    signal(SIGTERM, stop_handler);
    
    if (chdir("/") < 0) {
	perror("chdir failed");
	exit(EXIT_FAILURE);
    }
    
   
    

    // main program loop
    // 1. load config
    // 2. start a thread per set of mirrored disks
    // 3. periodically check if we are in our run window, if not, set state to 
    //    waiting. 
    // 4. if state changes to stop, wait for threads to finish and exit
    
    std::vector<thread_data_st> thread_work;
    std::string start_time;
    std::string stop_time;
    logger_level log_level;

    std::string db_ip, db_pass, db_user;
    
    try {
	ConfigParse config(argv[2]);
	
	int num_sets = std::stoi(config.getValue("Settings", "stores"));
	start_time = config.getValue("Settings", "start_time");
	stop_time = config.getValue("Settings", "stop_time");
	log_path = config.getValue("Settings", "log_path");
	std::string level = config.getValue("Settings", "log_level");
	uint64_t interval = std::stoll(config.getValue("Settings", "completion_interval"));
	db_ip = config.getValue("Settings", "db_ip");
	db_pass = config.getValue("Settings", "db_pass");
	db_user = config.getValue("Settings", "db_user");
	
	
	if (level.compare("DEBUG") == 0) {
	    log_level = DEBUG;
	} else if (level.compare("INFO") == 0) {
	    log_level = INFO;
	} else if (level.compare("WARNING") == 0) {
	    log_level = WARNING;
	} else if (level.compare("ERROR") == 0) {
	    log_level = ERROR;
	} else {
	    log_level = INFO;
	}
	
	for (int i = 0; i < num_sets; ++i) {
	    std::string store_name = "Store " + std::to_string(i + 1);
	    ConfigParse::const_iterator it = config.begin(store_name);
	    std::vector<std::string> disk_list;
	    
	    for (; it != config.end(store_name); ++it) {
		disk_list.push_back(it->second);
	    }
	    
	    thread_data_st data;
	    data.disks = disk_list;
	    data.log_level = log_level;
	    data.interval = interval;
	    data.db_user = db_user;
	    data.db_pass = db_pass;
	    data.db_ip = db_ip;
	    thread_work.push_back(data);
	}
	    
    } catch (ConfigParseEx& e) {
	std::cerr << e.what() << std::endl;
	exit(EXIT_FAILURE);
    } 
    Logger log(log_path + "/backup_manager.main");
    log << INFO << "Config parsed, starting " << thread_work.size() << " worker threads" 
	<< std::endl;

    std::vector<std::thread> workers;
    for (uint32_t i = 0; i < thread_work.size(); ++i) {
	workers.push_back(std::thread(worker_function, thread_work[i], i));
    }
    
    // all output via logging now
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    // the main thread loop does the following:
    // 1. checks to see if we are in the time window; if not, pause work
    // 2. checks config file for config updates
    while (state != STOP) {
	if (!in_window(start_time, stop_time)) {
	    log << INFO << "Out of time window, setting state to WAIT" << std::endl;
	    set_state(WAIT);
	} else {
	    log << INFO << "In time window, setting state to RUN" << std::endl;
	    set_state(RUN);
	}

	
	try {
	    ConfigParse config(argv[2]);
	    
	    start_time = config.getValue("Settings", "start_time");
	    stop_time = config.getValue("Settings", "stop_time");
	} catch (ConfigParseEx& e) {
	    // nothing to do here. we still have the old values for start/stop time
	    // maybe the user will have fixed the config file the next time around
	    log << WARNING << "Config parse failed with error " << e.what() << std::endl;
	} 
	
	sleep(30);
    }

    log << INFO << "Exiting main run loop, waiting for worker threads to exit" << std::endl;
    
    for (uint32_t i = 0; i < workers.size(); ++i) {
	workers[i].join();
    }

    log << INFO << "Workers all exited, cleaning up and shutting down" << std::endl;
    
    // We shouldnt make it here until manager stops, 
    // which wont happen unless we get a SIGTERM.
    // Unlock the pid file and remove it
    struct flock file_lock = {F_UNLCK, SEEK_SET, 0, 0, 0};
    fcntl(fd, F_SETLK, &file_lock);
    close(fd);
    remove(LOCK_FILE);
    
    
    return (EXIT_SUCCESS);
}


static void usage()
{
    std::cout << "usage: backup_manager [start <config file> | stop | help]" << std::endl;
}


static void send_stop()
{
    FILE *fp;
    pid_t pid;
    
    fp = fopen(LOCK_FILE,"r");
    
    if (!fp) {
	std::cerr << "Failed to open lock file - is backup manager running?" << std::endl;
	exit(EXIT_FAILURE);
    }
    
    if (fscanf(fp, "%d\n", &pid) != 1) {
	perror("Read of pid file failed");
	fclose(fp);
	exit(EXIT_FAILURE);
    }
    
    fclose(fp);
    
    if (kill(pid, SIGTERM) < 0) {
	std::cerr << "Failed to stop - are you root?" << std::endl;
    }
}


static void stop_handler(int signo)
{
    set_state(STOP);
}


static bool lock_file(int& fd)
{
    struct flock file_lock = {F_WRLCK, SEEK_SET, 0, 0, 0};
    char buf[16];

    fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  
    if (fd < 0) {
	perror("Failed to open lock file");
	return (false);
    }
  
    if (fcntl(fd, F_SETLK, &file_lock) < 0) {
	if (errno == EACCES || errno == EAGAIN) {
	    close(fd);
	    return (false);
	}
	perror("Failed to lock file");
	return (false);
    }
  
    if (ftruncate(fd, 0) < 0) {
	perror("ftruncate failed");
	return (false);
    }

    snprintf(buf, sizeof(buf), "%ld", (long)getpid());
    if (write(fd, buf, strlen(buf)+1) < 0) {
	perror("write to lock file failed");
	return (false);
    }
    
    return (true);
}


static void set_state(const manager_state_e s)
{
    lock.lock();
    // once we've been stopped we do not want to overwrite that state
    if (state != STOP) {
	state = s;
    }
    lock.unlock();
    
}

 static void worker_function(thread_data_st data, int id)
{
    std::string log = log_path + "/backup_manager." + std::to_string(id);
    BackupManager b(data.disks, log, data.log_level, data.interval, data.db_user, 
		    data.db_pass, data.db_ip);
    
    // each thread shares the state with the main thread
    // and will exit or pause as appropriate
    b.run(state);
}
