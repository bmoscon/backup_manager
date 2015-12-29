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

#include <cassert>

#include "db.hpp"


BackupManagerDB::BackupManagerDB(const std::string& ip, const std::string& user, 
				 const std::string& password, Logger* l) : _log(l)
{
    std::string new_ip = "tcp://" + ip + ":3306";
    try {
	_driver = get_driver_instance();
	_conn = _driver->connect(new_ip.c_str(), user.c_str(), password.c_str());
	_stmt = _conn->createStatement();
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }

    _db_name = "backup_maanger";
    _dir_table = "Directories";
    _file_table = "Files";
}


BackupManagerDB::~BackupManagerDB()
{
    _driver->threadEnd();
    delete _stmt;
    delete _conn;
}


void BackupManagerDB::set_db(const std::string& db, const std::string& dir, const std::string& file)
{
    _db_name = db;
    _dir_table = dir;
    _file_table = file;
}

void BackupManagerDB::init_tables()
{
    try{
	_stmt->execute("CREATE DATABASE IF NOT EXISTS " + _db_name);
	_stmt->execute("USE " + _db_name);
	
	_stmt->execute("CREATE TABLE IF NOT EXISTS " + _dir_table + " "
		       "(DirID INT AUTO_INCREMENT,"
		       "Path VARCHAR(4096),"
		       "Name VARCHAR(255),"
		       "PRIMARY KEY(DirID)) ENGINE=InnoDB");
	
	_stmt->execute("CREATE TABLE IF NOT EXISTS " + _file_table + " "
		       "(FileID INT AUTO_INCREMENT,"
		       "Dir INT,"
		       "Path VARCHAR(4096),"
		       "FileName VARCHAR(255),"
		       "FileModified BIGINT,"
		       "FileSize BIGINT,"
		       "CRC32 BIGINT,"
		       "LastChecked BIGINT,"
		       "PRIMARY KEY(FileID),"
		       "FOREIGN KEY(Dir) REFERENCES " + _dir_table + "(DirID)"
		       "ON DELETE CASCADE) ENGINE=InnoDB");
	
	_conn->commit();
    } catch (sql::SQLException& e) {
	(*_log) << ERROR << "Exception: " << e.what() << std::endl;
    }
}


void BackupManagerDB::drop_tables()
{
    _stmt->execute("DROP TABLE IF EXISTS " + _file_table + ";");
    _stmt->execute("DROP TABLE IF EXISTS " + _dir_table + ";");
    _conn->commit();		   
}


void BackupManagerDB::drop_db()
{
    _stmt->execute("DROP DATABASE IF EXISTS " + _db_name + ";");
    _conn->commit();
}


uint32_t BackupManagerDB::get_dir_id(const std::string& path)
{
    uint32_t id;
    
    try {
	_res = _stmt->executeQuery("SELECT DirID FROM " + _dir_table + " WHERE "
				   "Path = \"" + path + "\";");
	_res->next();
	id = _res->getInt(1);
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }	

    delete _res;
    return (id);
}


Directory BackupManagerDB::get(const Directory& dir)
{
    Directory ret;
    
    try {
	_res = _stmt->executeQuery("SELECT DirID FROM " + _dir_table + " WHERE "
				   "Path = \"" + dir.path + "\";");
	
	if (_res->next()) {
	    uint32_t id = _res->getInt(1);
	    ret.path = dir.path;
	    ret.name = dir.name;
	    
	    delete _res;
	    _res = _stmt->executeQuery("SELECT * FROM " + _file_table + " WHERE Dir = "
				       + std::to_string(id) + ";");
	    
	    while(_res->next()) {
		File f;
		f.path = _res->getString(3);
		f.name = _res->getString(4);
		f.modified = _res->getInt(5);
		f.size = _res->getInt(6);
		f.crc = _res->getInt(7);
		f.checked = _res->getInt(8);
		
		ret.files.insert(std::make_pair(f.name, f));
	    }
	    
	    delete _res;
	}
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
    
    return (ret);
}


bool BackupManagerDB::exists(const Directory& dir)
{
    try {
	_res = _stmt->executeQuery("SELECT DirID FROM " + _dir_table + " WHERE "
				   "Path = \"" + dir.path + "\";");
	if (_res->next()) {
	    delete _res;
	    return (true);
	}
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
    
    delete _res;
    return (false);
}


bool BackupManagerDB::exists(const File& file)
{
    try {
	_res = _stmt->executeQuery("SELECT FileID FROM " + _file_table + " WHERE "
				   "Path = \"" + file.path + "\" AND FileName = \""
				   + file.name + "\";");
	if (_res->next()) {
	    delete _res;
	    return (true);
	}
	
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
    
    delete _res;
    return (false);
}

void BackupManagerDB::insert(const Directory& dir)
{
    try {
	if (!exists(dir)) {
	    _stmt->execute("INSERT INTO " + _dir_table + " (Path, Name) VALUES (\"" +
			   dir.path + "\", \"" + dir.name + "\");");
	    _conn->commit();
	}
	
	for (file_cit i = dir.files.cbegin(); i != dir.files.cend(); ++i) {
	    insert(i->second);
	}
	_conn->commit();
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
}
    

void BackupManagerDB::insert(const File& file)
{
    try {
	if (!exists(file)) {
	    uint32_t id = get_dir_id(file.path);
	    _stmt->execute("INSERT INTO " + _file_table + " (Dir, Path, FileName, FileSize, "
			   "FileModified, CRC32, LastChecked)"
			   " VALUES (" + std::to_string(id) + ", \"" + file.path + "\", \""
			   + file.name + "\", " + std::to_string(file.size) + ", " +
			   std::to_string(file.modified) + ", " + std::to_string(file.crc) +
			   ", " + std::to_string(file.checked) + ");"); 
	}
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
}


void BackupManagerDB::update(const File& file)
{
    try {
	assert(exists(file));
	_stmt->execute("UPDATE " + _file_table + " SET "
		       " FileSize=" + std::to_string(file.size) + ", "
		       "FileModified=" + std::to_string(file.modified) + ", "
		       "CRC32=" + std::to_string(file.crc) + ", "
		       "LastChecked=" + std::to_string(file.checked) + " "
		       "WHERE Path=" + "\"" + file.path + "\"" + " AND FileName=" + "\"" +
		       file.name + "\";");
    }  catch (sql::SQLException& e) {
	(*_log) << ERROR << "DB Exception: " << e.what() << std::endl;
    }
}

