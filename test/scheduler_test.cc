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
    void worker() { sleep(1); _state = STOP; }
    void init() { _state = INIT; }
    void run()
    {
	_state = RUN;
	if (t.joinable()) {
	    t.join();
	}
	t = std::thread(&Test::worker, this);
    }
    void shutdown() { _state = SHUTDOWN; }
    void stop() { _state = STOP; }
    state_e get_state() const { return (_state); }

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
	case STOP:
	    std::cout << "STOP";
	    break;
	}
	std::cout << std::endl;
    }
};


static bool test_start_stop()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    Test *t = new Test();
    sch.configure(RUN_STOP);
    sch.add("TEST", t);
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 120) {
	state_e s = t->get_state();
	if (states.find(s) == states.end()) {
	    states.insert(s);
	}

	if (s == SHUTDOWN) {
	    break;
	}
	++count;
	sleep(1);
    }

    sch.stop();
    delete t;

    return (states.find(RUN) != states.end() &&
	    states.find(STOP) != states.end() &&
	    states.find(SHUTDOWN) != states.end());
}


static bool test_start_wait()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    Test *t = new Test();
    sch.configure(RUN_WAIT, "00:01");
    sch.add("TEST", t);
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 120) {
	state_e s = t->get_state();
	if (states.find(s) == states.end()) {
	    states.insert(s);
	}
	++count;
	sleep(1);
    }

    sch.stop();
    delete t;

    return (states.find(RUN) != states.end() &&
	    states.find(STOP) != states.end() &&
	    states.find(SHUTDOWN) == states.end());

}


bool test_run_always()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    Test *t = new Test();
    sch.configure(RUN_ALWAYS);
    sch.add("TEST", t);
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 30) {
	state_e s = t->get_state();
	if (states.find(s) == states.end()) {
	    states.insert(s);
	}
	++count;
	sleep(1);
    }

    sch.stop();
    delete t;

    return (states.find(RUN) != states.end() &&
	    states.find(STOP) != states.end() &&
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

static bool test_run_window()
{
    std::unordered_set<uint8_t> states;
    
    Scheduler sch;
    Test *t = new Test();
    sch.configure(WINDOW, get_time(1), get_time(2));
    sch.add("TEST", t);
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 240) {
	state_e s = t->get_state();
	t->print_state();
	if (states.find(s) == states.end()) {
	    states.insert(s);
	}
	++count;
	sleep(1);
    }

    sch.stop();
    delete t;

    return (states.find(RUN) != states.end() &&
	    states.find(STOP) != states.end() &&
	    states.find(SHUTDOWN) == states.end()); 
    
}


int main()
{
    std::cout << "These tests will take several minutes (5+) to run" << std::endl;
    std::cout << "Testing RUN_STOP scheduler ..." << std::endl;
    assert(test_start_stop());

    std::cout << "Testing RUN_WAIT scheduler ..." << std::endl;
    assert(test_start_wait());

    std::cout << "Testing RUN_ALWAYS scheduler ..." << std::endl;
    assert(test_run_always());

    std::cout << "Testing RUN_WINDOW scheduler ..." << std::endl;
    assert(test_run_window());
    

    std::cout << "**** PASS ****" << std::endl;
    return (0);
}
