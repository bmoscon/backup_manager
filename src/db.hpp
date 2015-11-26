/* Backup Manager DB Interface
 * 
 * Copyright (c) 2012-2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 10/05/2014 - Initial open source release
 * 11/26/2015 - Improvements to queries
 *
 */

#ifndef __BACKUPMANAGER_DB__
#define __BACKUPMANAGER_DB__

#include <string>

// MySQL CPP Connector Library Includes
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

#include "logger.hpp"
#include "file.hpp"


class BackupManagerDB {

public:
    BackupManagerDB(const std::string&, const std::string&, const std::string&, Logger*);
    ~BackupManagerDB();
    
    Directory get(const Directory&);
    void insert(const Directory&);
    bool exists(const Directory&);
    void insert(const File&);
    bool exists(const File&);
    void drop_tables();
     
private:
    void init_tables();
    uint32_t get_dir_id(const std::string&);
    
    Logger *_log;
    sql::Driver *_driver;
    sql::Connection *_conn;
    sql::Statement *_stmt;
    sql::ResultSet *_res;
    
};

#endif

