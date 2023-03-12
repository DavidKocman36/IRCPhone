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

#include "irc_bot.h"

irc_bot bot;
irc_bot_core core;
irc_bot_call currentCall;
irc_bot_proxy proxy;
irc_bot_message callChat;
addr_book addrBook;
irc_bot_message chatRoom;

/* The CTRL+C termination */
static void stop(int signum){
    addrBook.addr_book_close();
	bot.bot_terminate(&core, &proxy);
    exit(0);
}

int main(int argc, char *argv[]){
    // ./irc_bot {ip/server} {channel} {user} {password}
    if(argc != 5){
        cout << "Not enough arguments!" << endl;
        cout << "USAGE: ./irc_bot {server} {channel} {user_nick} {password}" << endl;
        return 1;
    }
    signal(SIGINT, stop);

    int coreErr = core.core_create();
    if(coreErr)
    {
        return 1;
    }

    const char *primCont = linphone_core_get_primary_contact(core._core);

    /* Open the database handle */
    int er = addrBook.addr_book_open();
    if(er) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addrBook.db));
        return 1;
    } 
    else{
        fprintf(stderr, "Opened database successfully\n");
    }
    sqlite3_busy_timeout(addrBook.db, 10);
    addrBook.addr_book_create();

    bot.server = argv[1];
    bot.channel = argv[2];

    if(bot.channel[0] != '#')
    {
        bot.channel = "#" + bot.channel;
    }

    bot.user_nick = argv[3];
    bot.password = argv[4];

    bot.nick = bot.user_nick + "_b";

    /* Resolve hostname */
    struct hostent *he;    
    if ( (he = gethostbyname(bot.server.c_str()) ) == NULL ) {
        exit(1); 
    }

    bot.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bot.sockfd < 0)
    {
        perror("Socket:");
        return 1;
    }
    /* Set receive timeout */
    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000;
    
    if (setsockopt (bot.sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    {
        perror("Setsockopt: \n");
    }
    /* Connect to the server */ 
    struct sockaddr_in irc_server;
    irc_server.sin_family = AF_INET;
    irc_server.sin_port = htons(6666);
    memcpy(&irc_server.sin_addr, he->h_addr_list[0], he->h_length);

    int conn = connect(bot.sockfd, (sockaddr*)&irc_server, sizeof(irc_server));
    if(conn < 0){
        perror("Socket:");
        return 1;
    }
    else{
        cout << "Connected: " << bot.server << endl;
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    /* Send all the initial commands */
    string channel_com = "JOIN " + bot.channel + "\r\n";
    string password_com = "PASS " + bot.password + "\r\n";
    string user_com = "USER " + bot.nick + " 0 * :" + bot.user_nick + "'s bot\r\n";
    string nick_com = "NICK " + bot.nick + "\r\n";
    vector<string> messages;
    string ircMsg;
    int bytes_recieved = 0;

    bot.send_init_com(password_com);
    bot.send_init_com(nick_com);
    bot.send_init_com(user_com);
    
    string hello_com = "PRIVMSG " + bot.user_nick + " :Hello!\r\n";
    bot.send_init_com(hello_com);
    string id_com = "PRIVMSG " + bot.user_nick + " :You are now as " + primCont + "!\r\n";
    bot.send_init_com(id_com);

    while(42)
    {
        core.iterate();
        /* Let the user know if an incoming call comes */
        if(!incomingCallMessage.empty())
        {
            bot.order += 1;
            string msg = "PRIVMSG " + bot.user_nick + " :" + incomingCallMessage + ". " + "Type \"accept " + to_string(bot.order) + "\" to accept this call!\r\n";
            bot.send_com(msg);
            incomingCallMessage.clear();
        }
        /* Let the user know if a message comes */
        if(!incomingChatMessage.empty())
        {
            string msg = "PRIVMSG " + bot.user_nick + " :" + incomingChatMessage + "\r\n";
            bot.send_com(msg);
            incomingChatMessage.clear();
        }
        /* Let the user know if remote or user hung up a call */
        if(!remoteHungUp.empty())
        {
            string msg = "PRIVMSG " + bot.user_nick + " :" + remoteHungUp + "\r\n";
            bot.send_com(msg);
            remoteHungUp.clear();
        }  

        /* Receive commands */
        bytes_recieved = recv(bot.sockfd, buffer, 1024, 0);
        if(bytes_recieved > 0)
        {
            messages.clear();
            ircMsg = string(buffer, 0, bytes_recieved);
            memset(buffer, 0, sizeof(buffer));

            // trim of the "\r\n" for better command handling
            ircMsg.resize(ircMsg.length() - 2);
            split(ircMsg, " ", messages);

            /* On some servers send JOIN command after MODE command */
            if(std::find(messages.begin(), messages.end(), "MODE") != messages.end()) 
            {
                bot.send_com(channel_com);
            }

            /* If error occurs end the bot */
            if(std::find(messages.begin(), messages.end(), "ERROR") != messages.end()) 
            {
                //dealloc if needed
                addrBook.addr_book_close();
                bot.bot_terminate(&core, &proxy);
                return 0;
            }

            /* Respond to PING */
            std::vector<std::string>::iterator in;
            if((in = std::find(messages.begin(), messages.end(), "PING")) != messages.end())
            {
                int index = in - messages.begin() + 1;
                string pong = "PONG " + messages[index] + "\r\n";
                bot.send_init_com(pong);
                messages.clear();
                memset(buffer, 0, sizeof(buffer));
            }
            if(messages[1] == "PRIVMSG")
            {
                vector<string> aux;
                split(messages[0], "!", aux);
                string correct_nick = ":" + bot.user_nick;
                /* Ignore other users */
                if((aux[0] != correct_nick) || (messages[2] != bot.nick))
                {
                    continue;
                }

                string command = messages[3];
                /* End the bot */
                if(command == ":end") 
                {
                    addrBook.addr_book_close();
                    bot.bot_terminate(&core, &proxy);
                    return 0;
                }
                /* Set stun and turn */
                else if(command == ":-s")
                {
                    string msg;
                    if(messages.size() < 5)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! -s <server> [-t <user> <password>]\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    string address = messages[4];
                    core.enable_stun(address);
                    if(messages.size() > 5)
                    {
                        if(messages[5] == "-t")
                        {
                            if(messages.size() < 6)
                            {
                                msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! -s <server> [-t <user> <password>]\r\n";
                                bot.send_com(msg);
                                continue;
                            }
                            string user = messages[6];
                            string passw = messages[7];
                            core.enable_turn(user, passw);   
                        }
                    }
                }
                /* Send a message */
                else if(command == ":mess")
                {
                    string msg;
                    string uri;
                    int start = 5;
                    /* Send to a remote in contacts */
                    if(messages[4] == "-a")
                    {
                        if(messages.size() < 6)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! mess -a <name> <text>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        if(!addrBook.addr_book_get_contact(messages[5]))
                        {
                            uri = addrBook.contactUri;
                            start = 6;
                        }   
                        else
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :" + addrBook.dbMessage + "\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                    }
                    else
                    {
                        if(messages.size() < 5)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! mess <uri> <text>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        uri = messages[4];
                        start = 5;
                    }
                    if(start >= messages.size())
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! Text is missing!\r\n";
                        bot.send_com(msg);
                        continue;
                    }

                    chatRoom.create_chat_room(core._core, uri);

                    string chatMessage;
                    /* Create and send the message */
                    
                    for(int i = start; i < messages.size(); i++)
                    {
                        chatMessage = chatMessage + " " + messages[i];
                    }
                    chatRoom.send_message(chatMessage);
                    msg = "PRIVMSG " + bot.user_nick + " :Sending a message\r\n";
                    bot.send_init_com(msg);
                }
                else if(command == ":call")
                {
                    bot.call(messages, currentCall, core, callChat, addrBook, proxy, 0);
                    callsVector.clear();
                }
                else if(command == ":register")
                {
                    /* Register at a proxy */
                    string msg;

                    if(proxy.proxy_cfg != nullptr && linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationOk)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Already registered as " + bot.sipUsername + "!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(messages[4] == "-a")
                    {
                        /* Register with credentials stored in database */
                        if(messages.size() != 6)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! register -a <name>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        if(!addrBook.addr_book_get_registrar(messages[5]))
                        {
                            proxy.set_credentials(addrBook.regUri, addrBook.passw);
                            bot.sipUsername = addrBook.regUri;
                        }
                        else
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :" + addrBook.dbMessage + "\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                    }
                    else
                    {
                        if(messages.size() != 6)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! register <uri> <password>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        proxy.set_credentials(messages[4], messages[5]);
                        bot.sipUsername = messages[4];
                    }
                    int err = proxy.bot_register(core._core);
                    if(err)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :" + proxy.user + " is not a valid sip uri, must be like sip:toto@sip.linphone.org!\r\n";
                        bot.send_com(msg);
                        continue;
                    }

                    /* Register */
                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationOk)){
                        core.iterate();
                        usleep(20000);
                    }
                    
                    /* Confirm the registration */
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationOk)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Succesfully registered as " + bot.sipUsername + "!\r\n";
                        bot.send_com(msg);
                    }
                    else
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Registration failed!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":unregister")
                {
                    /* Unregister from a proxy */
                    string msg;
                    if(proxy.proxy_cfg == nullptr)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) != LinphoneRegistrationOk)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    
                    proxy.bot_unregister(core._core);
                    bot.sipUsername.clear();
                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationCleared)){
                        core.iterate();
                        usleep(20000);
                    }
                    
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationCleared)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Succesfully unregistered!\r\n";
                        bot.send_init_com(msg);
                        bot.sipUsername.clear();
                        id_com = "PRIVMSG " + bot.user_nick + " :You are now as " + primCont + "!\r\n";
                        bot.send_com(id_com);
                    }
                    else
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Unegistration failed!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":status")
                {
                    bot.print_status(&core, &proxy);
                }
                else if(command == ":help")
                {
                    bot.print_help(messages);
                }
                else if(command == ":hangup")
                {
                    /* Invalid hangup */
                    string msg;
                    msg = "PRIVMSG " + bot.user_nick + " :Not in a call!\r\n";
                    bot.send_com(msg);
                }
                else if(command == ":accept")
                {
                    /* Accept a call if exists */
                    string msg;
                    if(incomingCall != nullptr)
                    {
                        bot.accept(messages, currentCall, core, callChat, addrBook, proxy);
                    }
                    else
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :There is no call to be accepted!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":decline")
                {
                    bot.decline_func();
                }
                else{
                    /* Address book commands */
                    if(!addrBook.addr_book_iterate(command, messages))
                    {
                        string msg = "PRIVMSG " + bot.user_nick + " :" + addrBook.dbMessage + "\r\n";
                        bot.send_com(msg);
                    }
                    /* Browse database */
                    else if(command == ":-con")
                    {
                        addrBook.addr_book_get_data(messages, Contact);
                        bot.addr_book_print(messages, addrBook, core);
                    }
                    else if(command == ":-reg")
                    {
                        addrBook.addr_book_get_data(messages, Registrar);
                        bot.addr_book_print(messages, addrBook, core);
                    }
                    else
                    {
                        string msg = "PRIVMSG " + bot.user_nick + " :Unknown command! Use \"help\" for some guidance.\r\n";
                        bot.send_com(msg);
                    }
                }
            }
            messages.clear();
        }
    }

    return 0;
}
