/*
 * This file is part of IRCPhone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MY_STRING_H
#define MY_STRING_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

/**
 * Function that splits a string based on given delimeter
 * 
 * @param str The input string
 * @param delim The given delimeter
 * @param parts The vector containing splitted string
*/
void split(const string& str, const string& delim, vector<string>& parts);

/**
 * Function for replacing all occurences of a string in a string.
 * 
 * @param data The input string
 * @param toSearch String or character to search and replace
 * @param replaceStr String to replace the searched string withs.
*/
void findAndReplaceAll(string & data, string toSearch, string replaceStr);

/**
 * Function for determining whether a string is a number or not.
 * 
 * @param str Input string
 * @returns true if string is a number, false when not
*/
bool strIsDigit(string &str);

#endif