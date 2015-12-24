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
#include <vector>

#include "scheduler.hpp"


Scheduler::~Scheduler()
{
    stop();
    if (_scheduler_thread.joinable()) {
	_scheduler_thread.join();
    }

    for (map_it it = _s_map.begin(); it != _s_map.end(); ++it) {
	delete it->second;
	it->second = NULL;
    }
}


void Scheduler::configure(const mode_e& m,
			  const std::string& first,
			  const std::string& second)
{
    _mode = m;
    switch(m) {
    case RUN_ALWAYS:
    case RUN_STOP:
	break;
    case RUN_WAIT:
	assert(!first.empty());
	_time1 = first;
	break;
    case WINDOW:
	assert(!first.empty() && !second.empty());
	_time1 = first;
	_time2 = second;
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
    
    if (_scheduler_thread.joinable()) {
	_scheduler_thread.join();
    }
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
	delete it->second;
	it->second = NULL;
	_s_map.erase(it);
    }
    
    _lock.unlock();
}


void Scheduler::stop()
{
    _running = false;
    _lock.lock();
    
    for (cmap_it it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	it->second->shutdown();
    }
    
    _lock.unlock();
}


void Scheduler::main_thread()
{
    std::vector<std::string> to_remove;
    while (_running) {
	_lock.lock();
	for (cmap_it it = _s_map.cbegin(); it != _s_map.cend(); ++it) {
	    it->second->state_lock();
	    
	    state_e c = it->second->get_state();
	    state_e p = it->second->get_prev_state();
	    state_e n = next_state(c, p);
	    
	    if (n != c) {
		switch (n) {
		case RUN:
		    it->second->run();
		    break;
		case WAIT:
		    it->second->wait();
		    break;
		case SHUTDOWN:
		    it->second->shutdown();
		    to_remove.push_back(it->first);
		    break;
		default:
		    assert(false);
		}
	    }
	    it->second->state_unlock();
	}
	_lock.unlock();
	for (uint32_t i = 0; i < to_remove.size(); ++i) {
	    remove(to_remove[i]);
	}
	to_remove.clear();
	sleep(5);
    }
}


state_e Scheduler::next_state(const state_e& current, const state_e& prev)
{
    if (!_running || current == SHUTDOWN) {
	return (SHUTDOWN);
    }

    switch (_mode) {
    case RUN_ALWAYS:
	if (current == WAIT && prev == RUN) {
	    return (INIT);
	}
    
	if (current == WAIT && prev == INIT) {
	    return (RUN);
	}
	break;
    case RUN_STOP:
	if (prev == INIT && current == WAIT) {
	    return (RUN);
	}
	if (current == WAIT && prev == RUN) {
	    return (SHUTDOWN);
	}
	break;
    case RUN_WAIT:
	if (current == WAIT && prev == INIT) {
	    return (RUN);
	}

	if (current == WAIT && _time2.empty()) {
	    _time2 = get_time();
	    return (current);
	}

	if (current == WAIT && !_time2.empty()) {
	    std::pair<int, int> end, wait, now;
	    end = parse_time(_time2);
	    wait = parse_time(_time1);
	    now = parse_time(get_time());
	    
	    if ((now.first - end.first >= wait.first) && (now.second - end.second >= wait.second)) {
		_time2.clear();
		return (INIT);
	    }
	}
	break;
    case WINDOW:
	if (!in_window()) {
	    return (WAIT);
	}

	if (current == WAIT && prev == INIT) {
	    return (RUN);
	}

	if (current == WAIT && prev == RUN) {
	    return (INIT);
	}
    }
    
    return (current);
}


std::string Scheduler::get_time() const
{
    struct tm *timeinfo;
    time_t rawtime;
    char *time_buf;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    time_buf = asctime(timeinfo);
    
    std::string ret(time_buf);
    if (!ret.empty() && ret[ret.length() - 1] == '\n') {
	ret.erase(ret.length()-1);
    }
    
    return (ret);
}


std::pair<int, int> Scheduler::parse_time(const std::string& time) const
{
    std::pair<int, int> ret;
    
    size_t pos = time.find(":");
    ret.first = std::stoi(time.substr(pos-2, 2));
    ret.second =  std::stoi(time.substr(pos+1, 2));
    
    return (ret);
}


bool Scheduler::in_window() const
{
    assert(_mode == WINDOW);
    std::pair<int, int> time, stop_time, start_time;
    time = parse_time(get_time());
    start_time = parse_time(_time1);
    stop_time = parse_time(_time2);
    
    return (time.first >= start_time.first && time.first <= stop_time.first &&
	    time.second >= start_time.second && time.second <= stop_time.second);
}
