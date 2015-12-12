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

#include "scheduler.hpp"


class Test : public Schedulable {
public:
    std::thread t;
    void worker() { sleep(5); _state = STOP; }
    void init() { _state = INIT; }
    void run()
    {
	_state = RUN;
	if (t.joinable()) {
	    t.join();
	}
	t = std::thread(&Test::worker, this);
    }
    void pause() { _state = PAUSE; }
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
	case PAUSE:
	    std::cout << "PAUSE";
	    break;
	case STOP:
	    std::cout << "STOP";
	    break;
	}
	std::cout << std::endl;
    }
};


int main()
{
    Scheduler sch;
    Test *t = new Test();
    sch.configure(RUN_WAIT, "00:01");
    sch.add("TEST", t);
    t->print_state();
    sch.start();
    sleep(1);
    uint32_t count = 0;
    while (count < 120) {
	t->print_state();
	sleep(1);
    }


    return (0);
}
