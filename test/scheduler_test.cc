/* Scheduler Test Code
 * 
 * Copyright (c) 2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 12/10/2015- Initial open source release
 */

#include <iostream>
#include <unistd.h>
#include <unordered_set>
#include <cassert>

#include "scheduler.hpp"


class Test : public Schedulable {
public:
    std::thread t;
    Test() {}
    ~Test()
    {
	if (t.joinable()) {
	    t.join();
	}
    }
    void worker() { sleep(1); wait(); }
    void init() { _state = INIT; wait(); }
    void wait() { set_state(WAIT); }
    void run()
    {
	set_state(RUN);
	if (t.joinable()) {
	    t.join();
	}
	t = std::thread(&Test::worker, this);
    }
    void shutdown() { _state = SHUTDOWN; }

    void print_state()
    {
	std::cout << "State is: ";
	switch(_state) {
	case INIT:
	    std::cout << "INIT";
	    break;
	case RUN:
	    std::cout << "RUN";
	    break;
	case SHUTDOWN:
	    std::cout << "SHUTDOWN";
	    break;
	case WAIT:
	    std::cout << "WAIT";
	    break;
	default:
	    break;
	}
	std::cout << std::endl;
    }
};


static void test_start_stop()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    sch.configure(RUN_STOP);
    sch.add("TEST", new Test());
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 120) {
	auto s = sch.get_states();
	if (!s.size()) {
	    break;
	}
	
	for (auto i = s.begin(); i != s.end(); ++i) {
	    states.insert(i->second);
	}
	
	++count;
	sleep(1);
    }

    sch.stop();
    assert(states.find(RUN) != states.end() &&
	   states.find(WAIT) != states.end());
}


static void test_start_wait()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    sch.configure(RUN_WAIT, "00:01");
    sch.add("TEST", new Test());
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 120) {
	auto s = sch.get_states();
	if (!s.size()) {
	    break;
	}
	
	for (auto i = s.begin(); i != s.end(); ++i) {
	    states.insert(i->second);
	}
	
	++count;
	sleep(1);
    }

    sch.stop();
    
    assert(states.find(RUN) != states.end() &&
	   states.find(WAIT) != states.end() &&
	   states.find(SHUTDOWN) == states.end());
}


static void test_run_always()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    sch.configure(RUN_ALWAYS);
    sch.add("TEST", new Test());
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 30) {
	auto s = sch.get_states();
	if (!s.size()) {
	    break;
	}
	
	for (auto i = s.begin(); i != s.end(); ++i) {
	    states.insert(i->second);
	}
	
	++count;
	sleep(1);
    }

    sch.stop();

    assert(states.find(RUN) != states.end() &&
	   states.find(WAIT) != states.end() &&
	   states.find(SHUTDOWN) == states.end());
}


static std::string get_time(int plus_mins)
{
    struct tm *timeinfo;
    time_t rawtime;
    char *time_buf;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    timeinfo->tm_min += plus_mins;
    time_buf = asctime(timeinfo);
    
    std::string ret(time_buf);
    if (!ret.empty() && ret[ret.length() - 1] == '\n') {
	ret.erase(ret.length()-1);
    }
    
    return (ret);
}

static void test_run_window()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    sch.configure(WINDOW, get_time(1), get_time(2));
    sch.add("TEST", new Test());
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 240) {
	auto s = sch.get_states();
	if (!s.size()) {
	    break;
	}
	
	for (auto i = s.begin(); i != s.end(); ++i) {
	    states.insert(i->second);
	}
	
	++count;
	sleep(1);
    }

    sch.stop();
    assert(states.find(RUN) != states.end() &&
	   states.find(WAIT) != states.end() &&
	   states.find(SHUTDOWN) == states.end());
}


int main()
{
    std::cout << "These tests will take several minutes (5+) to run" << std::endl;
    std::cout << "Testing RUN_STOP scheduler ..." << std::endl;
    test_start_stop();

    std::cout << "Testing RUN_WAIT scheduler ..." << std::endl;
    test_start_wait();

    std::cout << "Testing RUN_ALWAYS scheduler ..." << std::endl;
    test_run_always();

    std::cout << "Testing RUN_WINDOW scheduler ..." << std::endl;
    test_run_window();
    

    std::cout << "**** PASS ****" << std::endl;
    return (0);
}
