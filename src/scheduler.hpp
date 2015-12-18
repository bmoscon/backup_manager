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
    RUN_ALWAYS = 0,
    RUN_WAIT,
    RUN_STOP,
    WINDOW
} mode_e;

    
class Scheduler {
public:
    Scheduler() : _running(false), _mode(RUN_ALWAYS) {};
    ~Scheduler();

    void configure(const mode_e& m, const std::string& first="", const std::string& second="");
    void start();
    void add(const std::string&, Schedulable*);
    void remove(const std::string&);
    void stop();

private:
    void main_thread();
    state_e next_state(const state_e&);
    std::string get_time() const;
    std::pair<int, int> parse_time(const std::string&) const;
    bool in_window() const;
    
    typedef std::unordered_map<std::string, Schedulable*>::const_iterator cmap_it;
    typedef std::unordered_map<std::string, Schedulable*>::iterator map_it;
    std::unordered_map<std::string, Schedulable*> _s_map;
    std::thread _scheduler_thread;
    std::mutex _lock;
    bool _running;
    mode_e _mode;
    std::string _time1;
    std::string _time2;
    
};


#endif
