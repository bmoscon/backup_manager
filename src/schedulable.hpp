/* Interface for a Schedulable Object
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

#ifndef __SCHEDULABLE__
#define __SCHEDULABLE__

#include <mutex>


typedef enum state_e : uint8_t {
    NONE = 0,
    INIT,
    RUN,
    WAIT,
    SHUTDOWN
} state_e;

class Schedulable {
public:
    Schedulable() : _state(NONE), _prev_state(NONE) {}
    virtual ~Schedulable() {}
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void wait() = 0;
    virtual void shutdown() = 0;
    
    virtual state_e get_state() const
    {
	return (_state);
    }

    virtual state_e get_prev_state() const
    {
	return (_prev_state);
    }
    
    virtual void set_state(const state_e& s)
    {
	if (_state == SHUTDOWN) {
	    return;
	}
	
	_state_lock.lock();
	if (_state != WAIT) {
	    _prev_state = _state;
	}
	_state = s;
	_state_lock.unlock();
    }

    void state_lock() { _state_lock.lock(); }
    void state_unlock() { _state_lock.unlock(); }
    
protected:
    state_e _state;
    state_e _prev_state;
    std::recursive_mutex _state_lock;
};


#endif
