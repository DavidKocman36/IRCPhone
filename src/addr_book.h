#ifndef ADDR_BOOK_H
#define ADDR_BOOK_H

#include <stdio.h>
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include "my_string.h"

using namespace std;

enum Option
{
    Contact = 0,
    Registrar = 1
};

struct Data{
    string name;
    string uri;
};

class addr_book
{
public:
    sqlite3 *db;
    string dbMessage;

    string contactUri;
    string regUri;
    string passw;

    vector<Data> dbData;

    addr_book(/* args */);
    ~addr_book();

    int addr_book_iterate(string &command, vector<string> messages);

    int addr_book_open(); //
    void addr_book_close(); //

    int addr_book_insert(string &name, string &uri, string &passw, Option opt); //
    int addr_book_delete(string &name, Option opt); //
    int addr_book_update(string &name, string &uri, string &passw, Option opt);
    
    //get data (also printed on stderr for review)
    //just get URI
    int addr_book_get_contact(string &name);
    //just return uri and passw
    int addr_book_get_registrar(string &name);

    int addr_book_check(string &name, Option opt); //

    int addr_book_create(); //
    int addr_book_drop(); //

    int addr_book_get_data(vector<string> messages, Option opt);

    string get_enum_uri(vector<string> messages);
};


#endif