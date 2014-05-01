/* Logger Tester
 * 
 * Copyright (c) 2014 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/29/2014 - Initial open source release
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cassert>
#include "logger.hpp"

#define LOG "test_log.log"

static int count_lines()
{
    std::ifstream ifile(LOG);
    if (!ifile) {
	return (-1);
    }

    int num_lines = 0;
    std::string line;

    while (std::getline(ifile, line)) {
        ++num_lines;
    }

    return (num_lines);
}

int main()
{
    remove(LOG);
    Logger l(LOG);
    l.set_level(ERROR);
    l << "Hello" << std::endl;
    l << INFO << "Hello" << std::endl;
    l << WARNING << "Hello" << std::endl;
    assert(count_lines() == 0);
    l << ERROR << "Hello" << std::endl;
    assert(count_lines() == 1);


    std::cout << "**** PASS ****" << std::endl;
    remove(LOG);
    return (0);
}
