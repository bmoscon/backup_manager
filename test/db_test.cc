#include <cassert>
#include <iostream>
#include <exception>

#include "db.hpp"
#include "disk.hpp"


int main()
{
    Logger l("/home/bryant/test.log");
    BackupManagerDB db("192.168.86.21", "bryant", "password", &l);
    Disk disk("/home/bryant/fileshare/open_source/backup_manager", &l);
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
