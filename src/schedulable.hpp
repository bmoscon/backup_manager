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

typedef enum state_e {
    INIT = 0,
    RUN,
    STOP,
    SHUTDOWN
} state_e;

class Schedulable {
public:
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void shutdown() = 0;
    virtual state_e get_state() const = 0;

protected:
    state_e _state;
};


#endif
