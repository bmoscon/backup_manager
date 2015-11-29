/* DB Test Code
 * 
 * Copyright (c) 2015 Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 *
 * 11/28/2015- Initial open source release
 */

#include <cassert>
#include <iostream>
#include <exception>

#include "db.hpp"
#include "disk.hpp"

static void usage()
{
    std::cout << "db_test [log path] [DB IP] [DB User] [DB Pass] [Dir path]" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 6) {
	usage();
	return (1);
    }
    
    Logger l(argv[1]);
    BackupManagerDB db(argv[2], argv[3], argv[4], &l);
    Disk disk(argv[5], &l);
    Directory dir;

    db.set_db("backup_manager_test", "Directories", "Files");
    db.init_tables();

    try {
	while (true) {
	    dir = disk.next_directory();
	    if (!dir.valid()) {
		break;
	    }
	    db.insert(dir);
	    auto d = db.get(dir);
	    assert(d.identical(dir));
	    assert(d == dir);
	}
    } catch (std::exception& e) {
	std::cout << e.what() << std::endl;
	db.drop_tables();
	db.drop_db();
	return (1);
    }
    
    db.drop_tables();
    db.drop_db();

    std::cout << "**** PASS ****" << std::endl;
    return (0);
}
