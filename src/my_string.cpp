/**
 * A cpp file containing all string methonds not present as a standard c++ feature
*/

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

//split string
//src: https://stackoverflow.com/questions/289347/using-strtok-with-a-stdstring
void split(const string& str, const string& delim, vector<string>& parts) {
    size_t start, end = 0;
    while (end < str.size()) {
        start = end;
        while (start < str.size() && (delim.find(str[start]) != string::npos)) {
            start++;  // skip initial whitespace
        }
        end = start;
        while (end < str.size() && (delim.find(str[end]) == string::npos)) {
            end++; // skip to end of word
        }
        if (end-start != 0) {  // just ignore zero-length strings.
            parts.push_back(string(str, start, end-start));
        }
    }
}