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

#include "scheduler.hpp"


Scheduler::~Scheduler()
{
    stop();
    _scheduler_thread.join();
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


