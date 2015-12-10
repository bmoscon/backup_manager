/* Scheduler
 * 
 * Copyright (c) 2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 12/8/2015 - Initial open source release
 *
 */

#include <unistd.h>
#include <cassert>

#include "scheduler.hpp"


Scheduler::~Scheduler()
{
    stop();
    _scheduler_thread.join();
}


void Scheduler::configure(const mode_e& m,
			  const std::string& first="",
			  const std::string& second="")
{
    switch(m) {
    case ALWAYS_RUN:
	_mode = m;
	break;
    case RUN_WAIT:
    case RUN_STOP:
    case WINDOW:
	break;
    default:
	assert(false);
    }
}


void Scheduler::start()
{
    if (_running) {
	return;
    }

    _running = true;
    _scheduler_thread = std::thread(&Scheduler::main_thread, this);
}

void Scheduler::stop()
{
    _running = False;
    for (cmap_it it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	it->second.stop();
	int retry = 0;
	while (it->second.get_state() != STOP && retry < 30) {
	    sleep(1);
	    ++retry;
	}
	if (it->second.get_state() != STOP) {
	    // LOG ERROR
	}
    }
}


void Scheduler::main_thread()
{
    while (_running) {
	state_e s = run_state();
	for (cmap_it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	    if (it->second.get_state != s) {
		switch (s) {
		case RUN:
		    it->second.run();
		    break;
		case STOP:
		    it->second.stop();
		    break;
		case PAUSE:
		    it->second.pause();
		    break;
		default:
		    assert(false);
		}
	    }
	}
	sleep(10);
    }
}


state_e Scheduler::run_state() const
{
    if (!_running) {
	return STOP;
    }
    
    switch(_mode) {
    case ALWAYS_RUN:
	return RUN:
    default:
	return STOP;
    }
}
