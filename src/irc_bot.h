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

#ifndef IRC_BOT_H
#define IRC_BOT_H
    #include <iostream>
    #include <string>
    #include <cstring>
    #include <sys/types.h> 
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <vector>
    #include <unistd.h>
    #include <signal.h>
    #include <cmath>

    #include "my_string.h"
    #include "irc_bot_core.h"
    #include "irc_bot_call.h"
    #include "irc_bot_proxy.h"
    #include "irc_bot_message.h"
    #include "addr_book.h"

    #include <linphone/linphonecore.h>
    
    using namespace std;

    class irc_bot
    {
    public:
        /* The IRC server the bot connects to */
        string server;
        /* The channel on the server the bot joins */
        string channel;
        /* Nick of user the bot corresponds to */
        string user_nick;

        string password;
        /* Bots nickname */
        string nick;
        /* Uri of the current remote user in call */
        string outgoingCallee;

        /* The uri you are registered to a proxy with */
        string sipUsername;
        
        /* Order of the incoming calls */
        int order;
        
        /* Call paused by remote flag */
        bool pausedByRemote;
        /* Socket file descriptor */
        int sockfd;

        /**
         * Sends a message with a new line.
         * 
         * @param command The string that should be sent.
        */
        void send_com(string command);

        /**
         * Sends a message without new line.
         * 
         * @param command The string that should be sent.
        */
        void send_init_com(string command);


        /**
         * Function for checking new incoming messages from IRC server.
         * 
         * @param call Current call object
         * @param core Current core object
         * @param callChat Current call chat object
         * @param addrBook Address book object
         * @param proxy Current proxy conf object
         * @returns 1 on exit (hangup), 0 otherwise
        */
        int check_messages_during_call(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);
        
        /**
         * Unregisters, destroys core and quits server
         * 
         * @param core Current core object
         * @param proxy Current proxy conf object
        */
        void bot_terminate(irc_bot_core *core, irc_bot_proxy *proxy);
        
        /**
         * Prints your actual status, whether you are registered, your current uri, if you are
         * in a call, list of calls and whether STUN/TURN is enabled.
         * 
         * @param core Current core object
         * @param proxy Current proxy conf object
        */
        void print_status(irc_bot_core *core, irc_bot_proxy *proxy);

        /**
         * Prints help - all commands that may be used by bot.
         * 
         * @param messages The vector containing PRIVMSG from IRC.
        */
        void print_help(vector<string> messages);

        /**
         * Function for the current call's main loop.
         * 
         * @param call Current call object
         * @param core Current core object
         * @param callChat Current call chat object
         * @param addrBook Address book object
         * @param proxy Current proxy conf object
        */
        void call_loop(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);

        /**
         * Function for the call's error check, address book lookups and initial invite.
         * 
         * @param messages The vector containing PRIVMSG from IRC.
         * @param call Current call object
         * @param core Current core object
         * @param callChat Current call chat object
         * @param addrBook Address book object
         * @param proxy Current proxy conf object
        */
        void call(vector <string>messages, irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy, int opt);

        /**
         * Function for accepting a call. Checks all errors and declines all other 
         * incoming calls if there are any.
         * 
         * @param messages The vector containing PRIVMSG from IRC.
         * @param call Current call object
         * @param core Current core object
         * @param callChat Current call chat object
         * @param addrBook Address book object
         * @param proxy Current proxy conf object
        */
        void accept(vector <string>messages, irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);

        /**
         * Function declines all incoming call.
         * 
         * @returns an int on failure, 0 on success
        */
        int decline();

        /**
         * Function for calling decline function. Also checks for errors and sends response.
        */
        void decline_func();

        /**
         * Function for handling commands for page traversal and calling printing function. 
         * Also handles all errors.
         * 
         * @param messages The vector containing PRIVMSG from IRC.
         * @param core Current core object
         * @param addrBook Address book object
         * @returns The current index
        */
        int addr_book_print(vector<string> messages, addr_book &addrBook, irc_bot_core core);

        /**
         * Function for printing current page of the database browser.
         * The data are at addr_book::dbData vector.
         * 
         * @param n Index of the first record shown on the page
         * @param x Index of the last record shown on the page
         * @param i The index of current data
         * @param pact Number of the actual page
         * @param pmax Number of the pages
         * @param addrBook Address book object
         * @returns 0 on success
        */
        int print(int n, int x, int &i, int pact, int pmax, addr_book addrBook);

        /* Constructor */
        irc_bot();
    };

#endif