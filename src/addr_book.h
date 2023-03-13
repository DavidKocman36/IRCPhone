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
 * 
 * Author: David Kocman
 * 
 */

#ifndef ADDR_BOOK_H
#define ADDR_BOOK_H

#include <stdio.h>
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include "my_string.h"

using namespace std;

/**
 * Enum used for recognizing what kind of data user wants to work with in functions.
*/
enum Option
{
    Contact = 0,
    Registrar = 1
};

/**
 * Struct used for getting data from database.
*/
struct Data{
    string name;
    string uri;
};

class addr_book
{
public:
    /* The sqlite database object */
    sqlite3 *db;
    /* Feedback message displayed to the user. */
    string dbMessage;

    /* Strings used for getting data when "-a" option is present */
    string contactUri;
    string regUri;
    string passw;

    /* Vector used for storing data from database */
    vector<Data> dbData;

    /* Destructor and constructor */
    addr_book();
    ~addr_book();

    /**
     * The main address book function. Used for recognizing each command.
     * 
     * @param command The command.
     * @param messages Vector containing the PRIVMSG from IRC.
     * @returns 0 in all cases when command is matched, 1 if its not
    */
    int addr_book_iterate(string &command, vector<string> messages);

    /**
     * Function opens a new database handle and passes the ref to the db object.
     * 
     * @returns 0 on success, 1 on failure
    */
    int addr_book_open(); //

    /**
     * Closes the database handle.
    */
    void addr_book_close(); //

    /**
     * Inserts a new record to the database.
     * 
     * @param name Unique name of the record.
     * @param uri The sip uri.
     * @param passw Password if user adds record to the proxy auth database.
     * @param opt Which kind of data is being inserted.
     * @returns 0 on success, 1 if contact already exists
    */
    int addr_book_insert(string &name, string &uri, string &passw, Option opt); 

    /**
     * Deletes a record from the database.
     * 
     * @param name Unique name of the record.
     * @param opt Which kind of data is being deleted.
     * @returns 0 on success, 1 if contact does not exist.
    */
    int addr_book_delete(string &name, Option opt); 

    /**
     * Updates a record in the database.
     * If you wish to not update an attribute in the proxy auth table, substitute it with "-.
     * 
     * @param name Unique name of the record.
     * @param uri The sip uri.
     * @param passw Password if user updates record in the proxy auth database.
     * @param opt Which kind of data is being updated.
     * @returns 0 on success, 1 if contact does not exist.
    */
    int addr_book_update(string &name, string &uri, string &passw, Option opt);
    
    /**
     * Gets the sip uri from the database based on the given name attribute.
     * The uri is written to addr_book::contactUri class attribute.
     * 
     * @param name Unique name of the record.
     * @returns 0 on success, 1 if contact does not exist.
    */
    int addr_book_get_contact(string &name);
    
    /**
     * Gets the sip uri and password from proxy auth table. Used for registernig to a proxy.
     * The uri and pasword is written to addr_book::regUri and addr_book::passw class attributes.
     * 
     * @param name Unique name of the record.
     * @returns 0 on success, 1 if info does not exist.
    */
    int addr_book_get_registrar(string &name);

    /**
     * Checks whether the record exists or does not exist.
     * 
     * @param name Unique name of the record.
     * @param opt Which kind of data is being checked.
     * @returns 0 if doesnt exist, 1 if exists.
    */
    int addr_book_check(string &name, Option opt); //

    /**
     * Creates the db tables.
     * 
     * @returns 0 on success, 1 on failure.
    */
    int addr_book_create(); //

    /**
     * Drops the db tables.
    */
    void addr_book_drop(); //

    /**
     * Gets data from database for printing.
     * 
     * @param messages The vector containing PRIVMSG from IRC.
     * @param opt Which kind of data is being retrieved.
     * @returns 0 on success, 1 on failure.
    */
    int addr_book_get_data(vector<string> messages, Option opt);

    /**
     * Performs a DNS NAPTR ENUM lookup.
     * 
     * @param messages The vector containing PRIVMSG from IRC.
     * @returns Sip uri corresponding to the given number or an empty string on failure. 
    */
    string get_enum_uri(vector<string> messages);
};

#endif