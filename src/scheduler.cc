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
    if (_thread_running) {
	_scheduler_thread.join();
    }
}


void Scheduler::configure(const mode_e& m,
			  const std::string& first,
			  const std::string& second)
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


void Scheduler::add(const std::string& name, Schedulable* s)
{
    _lock.lock();

    s->init();
    _s_map.insert(std::make_pair(name, s));
    
    _lock.unlock();
}


void Scheduler::remove(const std::string& name)
{
    _lock.lock();
    
    map_it it = _s_map.find(name);

    if (it != _s_map.end()) {
	it->second->stop();
	_s_map.erase(it);
    }
    
    _lock.unlock();
}


void Scheduler::stop()
{
    _running = false;
    _lock.lock();
    
    for (cmap_it it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	it->second->stop();
    }

    _lock.unlock();
}


void Scheduler::main_thread()
{
    _thread_running = true;
    while (_running) {
	state_e s = run_state();
	_lock.lock();
	for (cmap_it it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	    if (it->second->get_state() != s) {
		switch (s) {
		case RUN:
		    it->second->run();
		    break;
		case STOP:
		    it->second->stop();
		    break;
		case PAUSE:
		    it->second->pause();
		    break;
		default:
		    assert(false);
		}
	    }
	}
	_lock.unlock();
	sleep(5);
    }
    _thread_running = false;
}


state_e Scheduler::run_state() const
{
    if (!_running) {
	return STOP;
    }
    
    switch(_mode) {
    case ALWAYS_RUN:
	return RUN;
    case RUN_STOP:
    case RUN_WAIT:
    case WINDOW:
    default:
	return STOP;
    }
}
