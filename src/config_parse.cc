/*
 * config_parse.cc
 *
 *
 * Config Parse Method Definitions
 *
 *
 * Copyright (C) 2013-2014  Bryant Moscon - bmoscon@gmail.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution, and in the same 
 *    place and form as other copyright, license and disclaimer information.
 *
 * 3. The end-user documentation included with the redistribution, if any, must 
 *    include the following acknowledgment: "This product includes software 
 *    developed by Bryant Moscon (http://www.bryantmoscon.org/)", in the same 
 *    place and form as other third-party acknowledgments. Alternately, this 
 *    acknowledgment may appear in the software itself, in the same form and 
 *    location as other such third-party acknowledgments.
 *
 * 4. Except as contained in this notice, the name of the author, Bryant Moscon,
 *    shall not be used in advertising or otherwise to promote the sale, use or 
 *    other dealings in this Software without prior written authorization from 
 *    the author.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
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

ConfigParse::iterator ConfigParse::begin(const string &section)
{
    unordered_map<string, unordered_map<string, string> >::iterator it;
    it = config.find(section);
    
    if (it != config.end()) {
	return (it->second.begin());
    } else {
	throw ConfigParseEx("Section \"" + section + "\" does not exist");
    }
}

ConfigParse::iterator ConfigParse::end(const string &section)
{
    unordered_map<string, unordered_map<string, string> >::iterator it;
    it = config.find(section);
    
    if (it != config.end()) {
	return (it->second.end());
    } else {
	throw ConfigParseEx("Section \"" + section + "\" does not exist");
    }
}

string ConfigParse::getValue(const string &section, const string &name)
{
    unordered_map<string, unordered_map<string, string> >::iterator it;
    unordered_map<string, string>::iterator section_it;
    
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
		throw ConfigParseEx("INI parse failed - improperly defined section heading", 
				    line_number);
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
		    throw ConfigParseEx("INI parse failed - improperly formed name/value pair", 
					line_number);
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

vector<string> ConfigParse::getSections() const
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
