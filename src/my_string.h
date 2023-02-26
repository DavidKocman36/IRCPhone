#ifndef MY_STRING_H
#define MY_STRING_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

void split(const string& str, const string& delim, vector<string>& parts);
void findAndReplaceAll(string & data, string toSearch, string replaceStr);

#endif