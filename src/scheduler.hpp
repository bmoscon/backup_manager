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

#include "schedulable.hpp"


class Scheduler {
public:
    Scheduler() {};
    ~Scheduler();

    void start();
    void add(std::string&, Schedulable*);
    void remove(std::string);
    void stop();

private:
    typedef std::unordered_map<std::string, Schedulable*>::const_iterator cmap_it;
    std::unordered_map<std::string, Schedulable*> _s_map;
    std::thread _scheduler_thread;
    bool _running;
};


#endif
