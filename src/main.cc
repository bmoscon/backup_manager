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
 * 12/22/2015 - New design changes
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

#include "scheduler.hpp"
#include "backup_manager.hpp"
#include "config_parse.hpp"

#define LOCK_FILE "/var/run/backup_manager.pid"


static void usage();
static void send_stop();
static bool lock_file(int&);
static void stop_handler(int);

bool running = true;


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
    
   

    ConfigParse config(argv[2]);
    mode_e m;
    std::string mode = config.get_value("Settings", "mode");
    std::string time1, time2;
    if (!mode.compare("RUN_STOP")) {
	m = RUN_STOP;
    } else if (!mode.compare("RUN_ALWAYS")) {
	m = RUN_ALWAYS;
    } else if (!mode.compare("RUN_WAIT")) {
	m = RUN_WAIT;
	time1 = config.get_value("Settings", "wait_time");
    } else if (!mode.compare("WINDOW")) {
	m = WINDOW;
	time1 = config.get_value("Settings", "start_time");
	time2 = config.get_value("Settings", "stop_time");
    } else {
	std::cerr << "Invalid mode specified in config. Exiting" << std::endl;
	exit(EXIT_FAILURE);
    }
    
    
    Scheduler s;
    s.configure(m, time1, time2);
    s.add("BackupManager", new BackupManager(argv[2]));
    s.start();

    // all output via logging now
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    while (running) {	
	sleep(30);
    }

    s.stop();

   
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
    running = false;
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
