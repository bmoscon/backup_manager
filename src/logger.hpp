/*
 * Logger Library
 *
 *
 * Copyright (C) 2013-2014  Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 04/27/2014 - Initial open source release
 *
 */

#ifndef __LOGGER__
#define __LOGGER__


#include <fstream>
#include <cassert>
#include <ctime>
#include <sstream>


// Log levels
typedef enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR
} logger_level;



class Logger : public std::ostringstream {
public:
    Logger(const char *f);
    Logger(const std::string& f);
    Logger (const Logger&) = delete;
    Logger &operator= (const Logger&) = delete;
    ~Logger();
    
    
    void set_level(const logger_level& level);  
    void flush();
    template <typename T>
    Logger& operator<<(const T& t)
    {
	(*(std::ostringstream *)this) << t;
	return (*this);
    }
    
    Logger& operator<<(const logger_level& level);
    typedef Logger& (* LoggerManip)(Logger&);
    Logger& operator<<(LoggerManip m);
    
private:
    std::string get_time();
    inline const char* level_str(const logger_level& level);
    
    std::ofstream  _file;
    std::ostream&  _log; 
    logger_level   _level;
    logger_level   _line_level;
};


namespace std { 
    inline Logger& endl(Logger& out) 
    { 
	out.put('\n'); 
	out.flush(); 
	return (out); 
    } 
}


#endif
