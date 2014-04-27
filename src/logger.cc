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

#include "logger.hpp"


Logger::Logger(const char *f) : _file(f, std::ios::out | std::ios::app), 
				_log(_file),
				_level(INFO),
				_line_level(VERBOSE)
{
    assert(_file.is_open());
}


Logger::Logger(const std::string& f) : _file(f.c_str(), std::ios::out | std::ios::app), 
				       _log(_file),
				       _level(INFO),
				       _line_level(VERBOSE)
{
    assert(_file.is_open());
}


Logger::~Logger()
{
    if (_file.is_open()) {
	_log.flush();
	_file.close();
    }
}


void Logger::set_level(const logger_level& level)
{
    _level = level;
}  


void Logger::flush()
{
    if (_line_level >= _level) {
	_log << get_time() << " -- [" << level_str(_line_level) << "] -- " << str();
	_log.flush();
	str("");
    }
    _line_level = VERBOSE;
}


Logger& Logger::operator<<(const logger_level& level)
{
    _line_level = level;
    return (*this);
}


Logger& Logger::operator<<(LoggerManip m)
{ 
    return m(*this);
}


std::string Logger::get_time()
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


inline const char* Logger::level_str(const logger_level& level)
{
    switch (level) {
    case DEBUG:
	return ("DEBUG");
    case INFO:
	return ("INFO ");
    case WARNING:
	return ("WARN ");
    case ERROR:
	return ("ERROR");
    default:
	assert(false);
    } 
}
