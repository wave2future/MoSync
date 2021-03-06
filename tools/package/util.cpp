/* Copyright (C) 2010 MoSync AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "util.h"
#include "filelist/filelist.h"

using namespace std;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void beGood(const ios& i) {
	if(!i.good()) {
		printf("File I/O error.\n");
		exit(1);
	}
}

// writes a copy of in to out.
static void writeCopy(istream& in, ostream& out) {
	beGood(in);
	beGood(out);

	size_t size;
	in.seekg(0, ios_base::end);
	size = in.tellg();
	in.seekg(0, ios_base::beg);
	beGood(in);

	size_t pos = 0;
	while(pos < size) {
		char buffer[64*1024];	// arbitrary size
		size_t todo = MIN(size - pos, sizeof(buffer));
		in.read(buffer, todo);
		beGood(in);
		out.write(buffer, todo);
		beGood(out);
		pos += todo;
	}
}

void copyFile(const char* dst, const char* src) {
	ifstream in(src, ios_base::binary);
	ofstream out(dst, ios_base::binary);
	writeCopy(in, out);
}

void appendFile(const char* dst, const char* src) {
	ifstream in(src, ios_base::binary);
	ofstream out(dst, ios_base::binary | ios_base::app);
	writeCopy(in, out);
}

size_t getFileSize(const char* filename) {
	ifstream in(filename, ios_base::binary);
	beGood(in);

	size_t size;
	in.seekg(0, ios_base::end);
	size = in.tellg();
	beGood(in);
	return size;
}

string readFileToString(const char* src) {
	ifstream in(src);
	beGood(in);

	size_t size;
	in.seekg(0, ios_base::end);
	size = in.tellg();
	in.seekg(0, ios_base::beg);
	beGood(in);

	ostringstream oss;
	oss << in.rdbuf();
	beGood(in);
	return oss.str();
}

void* readBinaryFile(const char* src, size_t& size) {
	ifstream in(src, ios_base::binary);
	beGood(in);

	in.seekg(0, ios_base::end);
	size = in.tellg();
	in.seekg(0, ios_base::beg);
	beGood(in);

	void* buf = malloc(size);
	in.read((char*)buf, size);
	beGood(in);
	return buf;
}

void writeFile(const char* dst, const void* src, size_t size) {
	ofstream out(dst, ios_base::binary);
	beGood(out);
	out.write((char*)src, size);
	beGood(out);
}

void applyTemplate(const char* dst, const char* src, const TemplateMap& tm) {
	ifstream in(src);
	beGood(in);
	ofstream out(dst);
	beGood(out);

	while(true) {
		int c = in.get();
		if(in.eof())
			break;
		if(c == '%') {	// template argument start marker
			int d = in.get();
			if(d == '%') {	//literal percent sign
				out << '%';
			} else {
				// find the end
				string tempName;
				while(!in.eof()) {
					if(d == '%') {	//template argument end marker
						break;
					} else if(isspace(d)) {
						printf("Broken template file (space in template).\n");
						exit(1);
					} else {
						tempName += (char)d;
					}
					d = in.get();
				}
				// apply the template argument
				TemplateMap::const_iterator itr = tm.find(tempName);
				if(itr == tm.end()) {
					printf("Missing template argument: %s\n", tempName.c_str());
					exit(1);
				}
				out << itr->second;
			}
		} else {
			out.put(c);
		}
	}
}

void toDir(std::string& str) {
	if(str[str.size()-1] != '/')
		str += '/';
}

string fullpathString(const char* name) {
	char* res = fullpath(name);
	if(!res) {
		printf("Could not resolve path: '%s'\n", name);
		exit(1);
	}
	string s = res;
	free(res);
	return s;
}
