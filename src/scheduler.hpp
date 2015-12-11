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

#ifndef __SCHEDULER__
#define __SCHEDULER__


#include <unordered_map>
#include <thread>
#include <mutex>

#include "schedulable.hpp"


typedef enum mode_e {
    ALWAYS_RUN = 0,
    RUN_WAIT,
    RUN_STOP,
    WINDOW
} mode_e;

    
class Scheduler {
public:
    Scheduler() : _running(false), _thread_running(false), _mode(ALWAYS_RUN) {};
    ~Scheduler();

    void configure(const mode_e& m, const std::string& first="", const std::string& second="");
    void start();
    void add(const std::string&, Schedulable*);
    void remove(const std::string&);
    void stop();

private:
    void main_thread();
    state_e run_state() const;
    
    typedef std::unordered_map<std::string, Schedulable*>::const_iterator cmap_it;
    typedef std::unordered_map<std::string, Schedulable*>::iterator map_it;
    std::unordered_map<std::string, Schedulable*> _s_map;
    std::thread _scheduler_thread;
    std::mutex _lock;
    bool _running;
    bool _thread_running;
    mode_e _mode;
    
};


#endif
