/*
 * config_parse.cc
 *
 *
 * Config Parse Method Definitions
 *
 *
 * Copyright (C) 2013-2015  Bryant Moscon - bmoscon@gmail.com
 * 
 * Please see the LICENSE file for the terms and conditions 
 * associated with this software.
 *
 */

#include <fstream>
#include <sstream>

#include "config_parse.hpp"


using namespace std;

ConfigParse::ConfigParse(const char *file)
{
  parse(file);
}

ConfigParse::ConfigParse(const string &file)
{
  parse(file.c_str());
}

ConfigParse::const_iterator ConfigParse::begin(const string &section) const
{
  unordered_map<string, unordered_map<string, string> >::const_iterator it;
  it = config.find(section);
  
  if (it != config.end()) {
    return (it->second.begin());
  } else {
    throw ConfigParseEx("Section \"" + section + "\" does not exist");
  }
}

ConfigParse::const_iterator ConfigParse::end(const string &section) const
{
  unordered_map<string, unordered_map<string, string> >::const_iterator it;
  it = config.find(section);
  
  if (it != config.end()) {
    return (it->second.end());
  } else {
    throw ConfigParseEx("Section \"" + section + "\" does not exist");
  }
}

string ConfigParse::get_value(const string &section, const string &name) const
{
  unordered_map<string, unordered_map<string, string> >::const_iterator it;
  unordered_map<string, string>::const_iterator section_it;

  it = config.find(section);
  
  if (it != config.end()) {
    section_it = it->second.find(name);
    if (section_it == it->second.end()) {
      return (string());
    } else {
      return (section_it->second);
    }
  } else {
    return (string());
  }
  
}

void ConfigParse::parse(const char *file)
{
  unsigned int line_number = 0;
  string line;
  ifstream f(file);
  
  if (!f.is_open()) {
    throw ConfigParseEx("Cannot open ini file");
    return;
  }
  
  pair<unordered_map<string, unordered_map<string, string> >::iterator, bool> inserted;
  inserted.second = false;
  
  while (getline(f, line)) {
    ++line_number;
    if (!line.size() || line[0] == ';') {
      continue;
    }
    
    if (line[0] == '[') {
      size_t pos = line.find(']');
      if (pos == string::npos) {
	f.close();
	throw ConfigParseEx("INI parse failed - improperly defined section heading", line_number);
	return;
      }
      string section(line, 1, pos-1);
      
      //check if section exists
      unordered_map<string, unordered_map<string, string> >::iterator it;
      it = config.find(section);
      if (it != config.end()) {
	f.close();
	throw ConfigParseEx("INI parse failed - duplicate section names", line_number);
	return;
      }
      
      inserted = config.insert(make_pair(section, unordered_map<string, string>()));
      
      if (!inserted.second) {
	f.close();
	throw ConfigParseEx("Unordered map insertion failed");
	return;
      }
    } else {
      if (inserted.second) {
	istringstream cleaner(line);
	string name, value;
	
	cleaner >> line;
	size_t pos = line.find('=');
	if (pos == string::npos) {
	  f.close();
	  throw ConfigParseEx("INI parse failed - improperly formed name/value pair", line_number);
	  return;
	}
	
	name = string(line, 0, pos);
	value = string(line, pos+1, line.size()-1);
	
	inserted.first->second.insert(make_pair(name, value));
      }
    }
  }
  
  
  f.close();
}

vector<string> ConfigParse::get_sections() const
{
  vector<string> sections;
  unordered_map<string, section>::const_iterator it;

  it = config.begin();

  while (it != config.end()) {
    sections.push_back(it->first);
    ++it;
  }
  
  return (sections);
}


ConfigParseEx::ConfigParseEx(const char* m) : msg(m) {}
ConfigParseEx::ConfigParseEx(const std::string& m) : msg(m) {}
ConfigParseEx::ConfigParseEx(const char* m, const unsigned int l)
{
  msg = m;
  msg += " on line " + std::to_string(l);
}

ConfigParseEx::ConfigParseEx(const std::string& m, const unsigned int l)
{
  msg = m;
  msg += " on line " + std::to_string(l);
}

const char* ConfigParseEx::what() const throw()
{
  return (msg.c_str());
}
