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
 * A cpp file including an address book database API
 * resource: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
 * 
 * Author: David Kocman
 * 
*/
#include "addr_book.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    /* Called each time after a statement is successfully executed.
    Not used, but needed. */
    return 0;
}

addr_book::addr_book()
{
    /* Initialization of the response message. It shall be always overwritten.  */
    this->dbMessage = "Something went wrong, this message is not meant to be seen :/";
}

addr_book::~addr_book()
{
}

int addr_book::addr_book_open()
{
    /* Open the database handle. The file shal be created when it does not exist. */
    return sqlite3_open("db/addr_book.db", &(this->db));
}

void addr_book::addr_book_close()
{
    sqlite3_close(this->db);
    this->dbMessage = "Database closed successfully!";
}

int addr_book::addr_book_check(string &name, Option opt)
{
    char *zErrMsg = 0;
    string sql_str;
    struct sqlite3_stmt *selectstmt;
    int result;

    /* Retrieve the appropriate data based on the option passed. */
    if(opt == Contact)
    {
        sql_str = "SELECT * from CONTACTS WHERE NAME='" + name + "'";
    }
    else
    {
        sql_str = "SELECT * from REGISTRAR WHERE NAME='" + name + "'";
    }

    /**
     * from:
     * https://stackoverflow.com/questions/37751325/how-to-check-if-record-is-present-in-sqlite-in-c
    */
    const char *sql = sql_str.c_str();
    
    result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
        /* If nothing went wrong we will check whether step contains a row. If it does that means the record exists already. */
        if (sqlite3_step(selectstmt) == SQLITE_ROW)
        {
            return 1;
        }
    }
    else
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }
    sqlite3_finalize(selectstmt);
    return 0;
}

int addr_book::addr_book_create()
{
    char *zErrMsg = 0;
    char *sql;
    int rc;
    string sql_str;
    struct sqlite3_stmt *selectstmt;
    int result;
    
    /* First check if the tables are already created. */
    sql_str = "SELECT name FROM sqlite_master WHERE type='table' AND name='CONTACTS' OR name='REGISTRAR';";
    const char *sql_select = sql_str.c_str();
    
    result = sqlite3_prepare_v2(this->db, sql_select, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
        if (sqlite3_step(selectstmt) == SQLITE_ROW)
        {
            this->dbMessage = "Tables already exist! Drop the db first using \"-dropdb\"!";
            return 1;
        }
    }
    else
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }
    sqlite3_finalize(selectstmt);

    /* If they are not created then create them. Here is also a second layer of security 
     * in the form of "IF NOT EXISTS" statement.
     */
    sql = "CREATE TABLE IF NOT EXISTS CONTACTS("  \
      "ID         INTEGER      PRIMARY KEY," \
      "NAME       CHAR(150)    NOT NULL," \
      "URI        CHAR(150)    NOT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    } 

    sql = "CREATE TABLE IF NOT EXISTS REGISTRAR("  \
      "ID           INTEGER       PRIMARY KEY," \
      "NAME         CHAR(150)     NOT NULL," \
      "URI          CHAR(150)     NOT NULL," \
      "PASSW        CHAR(150)     NOT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    } 
    this->dbMessage = "Database created successfully!";
    return 0;
}

void addr_book::addr_book_drop()
{
    char *zErrMsg = 0;
    char *sql;
    int rc;

    sql = "DROP TABLE IF EXISTS CONTACTS; \
            DROP TABLE IF EXISTS REGISTRAR;";
    /* Execute SQL statement */
    rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
    } 
    else
    {
        this->dbMessage = "Database dropped successfully!";
    }
}

int addr_book::addr_book_insert(string &name, string &uri, string &passw, Option opt)
{
    char *zErrMsg = 0;
    string sql_str;

    /* Based on what kind of data we want to insert, the SELECT statement is modified. */
    if(opt == Contact)
    {
        /* Do not insert if a record with the same name already exists. */
        if(this->addr_book_check(name, Contact))
        {
            this->dbMessage = "Contact already exists!";
            return 1;
        }
        sql_str = "INSERT INTO CONTACTS (NAME,URI) "  \
        "VALUES ('"+ name + "', '" + uri + "');";
    }
    else
    {
        if(this->addr_book_check(name, Registrar))
        {
            this->dbMessage = "Identity already exists!";
            return 1;
        }
        sql_str = "INSERT INTO REGISTRAR (NAME,URI,PASSW) "  \
        "VALUES ('"+ name + "', '" + uri + "', '" + passw + "');";
    }

    const char *sql = sql_str.c_str();
    /* Insert */
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK )
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
    }
    else
    {
        this->dbMessage = "Record created successfully!";
    }
    return 0;
}

int addr_book::addr_book_delete(string &name, Option opt)
{
    char *zErrMsg = 0;
    string sql_str;

    if(opt == Contact)
    {
        if(!this->addr_book_check(name, Contact))
        {
            this->dbMessage = "Contact doesnt exist!";
            return 1;
        }
        sql_str = "DELETE from CONTACTS where NAME='" + name + "';";
    }
    else
    {
        if(!this->addr_book_check(name, Registrar))
        {
            this->dbMessage = "Record desn't exist!";
            return 1;
        }
        sql_str = "DELETE from REGISTRAR where NAME='" + name + "';";
    }

    const char *sql = sql_str.c_str();
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK )
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }
    else
    {
        this->dbMessage = "Record deleted successfully!";
    }
    return 0;
}

int addr_book::addr_book_update(string &name, string &uri, string &passw, Option opt)
{
    char *zErrMsg = 0;
    string sql_str;

    if(opt == Contact)
    {
        if(!this->addr_book_check(name, Contact))
        {
            this->dbMessage = "Contact doesnt exist!";
            return 1;
        }
        sql_str = "UPDATE CONTACTS set URI='" + uri + "' where NAME='" + name + "';";
    }
    else
    {
        /* Make the corresponding SELECT statement based on what attributes user wants to change. */
        if(!this->addr_book_check(name, Registrar))
        {
            this->dbMessage = "Record desn't exist!";
            return 1;
        }
        if(passw == "-" && uri != "-")
        {
            sql_str = "UPDATE REGISTRAR set URI='" + uri + "' where NAME='" + name + "';";
        }
        else if(passw != "-" && uri == "-")
        {
            sql_str = "UPDATE REGISTRAR set PASSW='" + passw + "' where NAME='" + name + "';";
        }
        else if(passw != "-" && uri != "-")
        {
            sql_str = "UPDATE REGISTRAR set URI='" + uri + "', PASSW='" + passw + "' where NAME='" + name + "';";
        }
        else
        {
            this->dbMessage = "Type at least one attribute to change!";
            return 1;
        }
    }

    const char *sql = sql_str.c_str();
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK )
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    } 
    else
    {
        this->dbMessage = "Record updated successfully!";
    }

    return 0;
}

int addr_book::addr_book_get_contact(string &name)
{
    string sql_str;
    struct sqlite3_stmt *selectstmt;
    char *zErrMsg = 0;

    /* Check, whether the contact does exist. */
    if(!this->addr_book_check(name, Contact))
    {
        this->dbMessage = "Contact doesnt exist!";
        return 1;
    }

    /* Construct the select statement. */
    sql_str = "SELECT URI from CONTACTS where NAME='" + name + "';";
    const char *sql = sql_str.c_str();

    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
        /* The SELECT shall always return only one row because the name is always unique. */
        if (sqlite3_step(selectstmt) == SQLITE_ROW)
        {
            this->contactUri = string(reinterpret_cast<const char*>(sqlite3_column_text(selectstmt, 0)));
        }
    }
    else
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }

    sqlite3_finalize(selectstmt);
    return 0;
}

int addr_book::addr_book_get_registrar(string &name)
{
    string sql_str;
    struct sqlite3_stmt *selectstmt;
    char *zErrMsg = 0;

    if(!this->addr_book_check(name, Registrar))
    {
        this->dbMessage = "Identity doesnt exist!";
        return 1;
    }

    sql_str = "SELECT URI,PASSW from REGISTRAR where NAME='" + name + "';";
    const char *sql = sql_str.c_str();

    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
       if (sqlite3_step(selectstmt) == SQLITE_ROW)
       {
          this->regUri = string(reinterpret_cast<const char*>(sqlite3_column_text(selectstmt, 0)));
          this->passw = string(reinterpret_cast<const char*>(sqlite3_column_text(selectstmt, 1)));
       }
    }
    else
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }
    sqlite3_finalize(selectstmt);
    return 0;
}

int addr_book::addr_book_iterate(string &command, vector<string> messages)
{
    /**
     * Returns always 0 because the command is valid so no "Unknown command!" message is sent
     */

    //insert
    if(command == ":-c")
    {
        this->addr_book_create();
        return 0;
    }
    //insert to contacts
    if(command == ":-ic")
    {
        if(messages.size() < 6)
        {
            this->dbMessage = "Wrong usage! -rc <name> <remote_uri>";
            return 0;
        }
        string aux1 = "";
        this->addr_book_insert(messages[4], messages[5], aux1, Contact);
        return 0;
    }
    //insert to registrar (proxy)
    if(command == ":-ir")
    {
        if(messages.size() < 7)
        {
            this->dbMessage = "Wrong usage! -rc <name> <proxy_uri> <password>";
            return 0;
        }
        this->addr_book_insert(messages[4], messages[5], messages[6], Registrar);
        return 0;
    }
    //update contacts
    if(command == ":-uc")
    {
        if(messages.size() < 6)
        {
            this->dbMessage = "Wrong usage! -uc <name> <remote_uri>";
            return 0;
        }
        string aux1 = "";
        this->addr_book_update(messages[4], messages[5], aux1, Contact);
        return 0;
    }
    //update registrar (proxy)
    if(command == ":-ur")
    {

        if(messages.size() < 7)
        {
            this->dbMessage = "Wrong usage! -ur <name> <remote_uri> <passw>. " \
            "If you wish to not update an attribute, type \"-\" instead e.g. \"-ur name - password\"";
            return 0;
        }
        this->addr_book_update(messages[4], messages[5], messages[6], Registrar);
        return 0;
    }
    //remove from contacts
    if(command == ":-rc")
    {
        if(messages.size() < 5)
        {
            this->dbMessage = "Wrong usage! -rc <name>";
            return 0;
        }
        this->addr_book_delete(messages[4], Contact);
        return 0;
    }
    //remove from (proxy)
    if(command == ":-rr")
    {
        if(messages.size() < 5)
        {
            this->dbMessage = "Wrong usage! -rr <name>";
            return 0;
        }
        this->addr_book_delete(messages[4], Registrar);
        return 0;
    }
    //drop tables
    if(command == ":-dropdb")
    {
        this->addr_book_drop();
        return 0;
    }

    return 1;
}

int addr_book::addr_book_get_data(vector<string> messages, Option opt)
{
    /* This function returns data for browsing the db. */
    string sql_str;
    struct sqlite3_stmt *selectstmt;
    char *zErrMsg = 0;
    Data data;

    /* Here is also the possibility of adding sqlite regex for better search */
    if(messages.size() > 4)
    {
        if(opt == Contact)
            sql_str = "SELECT NAME,URI from CONTACTS WHERE NAME LIKE '" + messages[4] + "';";
        else
            sql_str = "SELECT NAME,URI from REGISTRAR WHERE NAME LIKE '" + messages[4] + "';";
    }
    else
    {
        if(opt == Contact)
            sql_str = "SELECT NAME,URI from CONTACTS;";
        else
            sql_str = "SELECT NAME,URI from REGISTRAR;";
    }

    const char *sql = sql_str.c_str();

    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
        /* For both proxy and contact we only get name and uri. Password is not retreived. */
        while(sqlite3_step(selectstmt) == SQLITE_ROW)
        {
            data.name = string(reinterpret_cast<const char*>(sqlite3_column_text(selectstmt, 0)));
            data.uri = string(reinterpret_cast<const char*>(sqlite3_column_text(selectstmt, 1)));
            this->dbData.push_back(data);
        }
    }
    else
    {
        this->dbMessage = "SQL error: " + string(zErrMsg) + "!";
        sqlite3_free(zErrMsg);
        return 1;
    }
    sqlite3_finalize(selectstmt);
    return 0;
}

string addr_book::get_enum_uri(vector<string> messages)
{
    string number = messages[5];
    /* If number starts with 00 then zeros are trimmed. */
    if (number.rfind("00", 0) == 0)
    { 
        number.erase(0,2);
    }

    string revNum = "";
    int j = 0;
    /* Revert the number, get rid of all non numeric values and concatenate
     * e164.arpa. suffix.
     */
    for(int i = number.length() - 1; i >= 0; i--)
    {
        if(!isdigit(number[i]))
            continue;
        revNum.push_back(number[i]);
        revNum += ".";

    }
    revNum += "e164.arpa.";
    string uri;

    int size = 1000;
    union {
        HEADER hdr;              
        u_char buf[1000]; 
    } response;  
    
    /* Execute a DNS NAPTR search */
    res_init();
    ns_msg handle;
    const char *qry = revNum.c_str();
    int responseLen =res_search(qry, ns_c_in, ns_t_naptr,(u_char *)&response,sizeof(response));
    if (responseLen < 0)
        return "";

    if (ns_initparse(response.buf, responseLen, &handle)<0)
    {
        perror("Error: ");
        return "";
    }
    
    //https://www.rfc-editor.org/rfc/rfc2915#page-3
    //Methods are drawn from: https://docstore.mik.ua/orelly/networking_2ndEd/dns/ch15_02.htm

    ns_rr rr;
    int rrnum;
    ns_sect section=ns_s_an;
    char res[1000];
    string aux;
    bool found = false;

    /* Get the answer */
    for (rrnum=0;rrnum<(ns_msg_count(handle,section));rrnum++)
    {
        if (ns_parserr(&handle,ns_s_an,rrnum,&rr)<0)
        {
            fprintf(stderr, "ERROR PARSING RRs");
            return "";
        }   
        if (ns_rr_type(rr)==ns_t_naptr)
        {
            // retrieve the rdata
            memcpy(&res, ns_rr_rdata(rr) + 11, sizeof(res));
            aux = string(res);
            if (aux.find("sip") != string::npos) {
                /* We have found the SIP NAPTR RR */
                found = true;
                break;
            }
        }
    }

    if(!found)
        return "";
    
    /* Get regex parts of the NAPTR RR */
    vector<string> regexes;
    split(aux, "!", regexes); 
    /* Erase a redundant part of the response */
    regexes.erase(regexes.begin());

    /* Substitute the regex with C++ regex library */
    regex re(regexes[0], regex_constants::extended);
    uri = regex_replace(number, re, regexes[1], regex_constants::match_default | regex_constants::format_sed);

    return uri;
}
