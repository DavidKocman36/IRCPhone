/**
 * A cpp file including an address book database API
 * resource: https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
*/
#include "addr_book.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}

addr_book::addr_book()
{
    this->dbMessage = "Something went wrong, this message is not meant to be seen :/";
}

addr_book::~addr_book()
{
}

int addr_book::addr_book_open()
{
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

    if(opt == AddContact)
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

int addr_book::addr_book_drop()
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

    if(opt == AddContact)
    {
        if(this->addr_book_check(name, AddContact))
        {
            this->dbMessage = "Contact already exists!";
            return 1;
        }
        sql_str = "INSERT INTO CONTACTS (NAME,URI) "  \
        "VALUES ('"+ name + "', '" + uri + "');";
    }
    else
    {
        if(this->addr_book_check(name, AddRegistrar))
        {
            this->dbMessage = "Contact already exists!";
            return 1;
        }
        sql_str = "INSERT INTO REGISTRAR (NAME,URI,PASSW) "  \
        "VALUES ('"+ name + "', '" + uri + "', '" + passw + "');";
    }

    const char *sql = sql_str.c_str();
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

    if(opt == AddContact)
    {
        if(!this->addr_book_check(name, AddContact))
        {
            this->dbMessage = "Contact doesnt exist!";
            return 1;
        }
        sql_str = "DELETE from CONTACTS where NAME='" + name + "';";
    }
    else
    {
        if(!this->addr_book_check(name, AddRegistrar))
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

    if(opt == AddContact)
    {
        if(!this->addr_book_check(name, AddContact))
        {
            this->dbMessage = "Contact doesnt exist!";
            return 1;
        }
        sql_str = "UPDATE CONTACTS set URI='" + uri + "' where NAME='" + name + "';";
    }
    else
    {
        if(!this->addr_book_check(name, AddRegistrar))
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

    if(!this->addr_book_check(name, AddContact))
    {
        this->dbMessage = "Contact doesnt exist!";
        return 1;
    }

    sql_str = "SELECT URI from CONTACTS where NAME='" + name + "';";
    const char *sql = sql_str.c_str();

    int result = sqlite3_prepare_v2(db, sql, -1, &selectstmt, NULL);
    if(result == SQLITE_OK)
    {
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

    if(!this->addr_book_check(name, AddRegistrar))
    {
        this->dbMessage = "Contact doesnt exist!";
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

/**
 * Returns always 0 because the command is valid so no "Unknown command!" message is sent
*/
int addr_book::addr_book_iterate(string &command, vector<string> messages)
{
    /**
     * TODO: command for printing contacts
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
        this->addr_book_insert(messages[4], messages[5], aux1, AddContact);
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
        this->addr_book_insert(messages[4], messages[5], messages[6], AddRegistrar);
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
        this->addr_book_update(messages[4], messages[5], aux1, AddContact);
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
        this->addr_book_update(messages[4], messages[5], messages[6], AddRegistrar);
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
        this->addr_book_delete(messages[4], AddContact);
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
        this->addr_book_delete(messages[4], AddRegistrar);
        return 0;
    }
    //drop tables
    if(command == ":-d")
    {
        this->addr_book_drop();
        return 0;
    }

    return 1;
}