/* Interface for a Schedulable Object
 * 
 * Copyright (c) 2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 12/26/2015 - Initial open source release
 *
 */

#include "schedulable.hpp"


static std::string state_str[] = {
    "NONE",
    "INIT",
    "RUN",
    "WAIT",
    "SHUTDOWN"
};


std::string state_to_str(const state_e& s)
{
    return (state_str[s]);
}
