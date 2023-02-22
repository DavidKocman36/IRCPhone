#include "irc_bot.h"

irc_bot bot;
irc_bot_core core;
irc_bot_call call;
irc_bot_proxy proxy;
irc_bot_message callChat;
addr_book addrBook;
/* TEST */
irc_bot_message chatRoom;

static void stop(int signum){
    addrBook.addr_book_close();
	bot.bot_terminate(&core, &proxy);
    string quit_com = "QUIT\r\n";
    bot.send_init_com(quit_com);
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

    core.core_create();
    linphone_core_set_log_file(NULL);
	linphone_core_set_log_level(ORTP_MESSAGE);

    linphone_core_enable_adaptive_rate_control(core._core, true);
    linphone_core_enable_mic(core._core, true);
    linphone_core_set_audio_port_range(core._core, 7077, 7079);
    linphone_core_set_play_file(core._core, "./sounds/toy-mono.wav");
    linphone_core_set_ring(core._core, "./sounds/ringback.wav");
    //linphone_core_set_max_calls(core._core, 1);
    const char *primCont = linphone_core_get_primary_contact(core._core);

    int er = addrBook.addr_book_open();
    if(er) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(addrBook.db));
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

    //resolve hostname
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
    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    
    if (setsockopt (bot.sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    {
        perror("Setsockopt: \n");
    }
        
    struct sockaddr_in irc_server;
    irc_server.sin_family = AF_INET;
    irc_server.sin_port = htons(6667);
    memcpy(&irc_server.sin_addr, he->h_addr_list[0], he->h_length);

    int conn = connect(bot.sockfd, (sockaddr*)&irc_server, sizeof(irc_server));
    if(conn < 0){
        perror("Socket:");
        return 1;
    }
    else{
        cout << "Connected: " << bot.server << endl;
    }

    char buffer[4096];

    string channel_com = "JOIN " + bot.channel + "\r\n";
    string password_com = "PASS " + bot.password + "\r\n";
    string user_com = "USER " + bot.nick + " 0 * :" + bot.user_nick + "\r\n";
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
        if(!incomingCallMessage.empty())
        {
            bot.order += 1;
            /**
             *  TODO: Fix this message a bit 
            */
            string msg = "PRIVMSG " + bot.user_nick + " :" + incomingCallMessage + ". " + "Type \"accept " + to_string(bot.order) + "\" to accept this call!\r\n";
            bot.send_com(msg);
            bot.incomingCallsVector.push_back(incomingCall);
            incomingCallMessage.clear();
        }
        if(!incomingChatMessage.empty())
        {
            string msg = "PRIVMSG " + bot.user_nick + " :" + incomingChatMessage + "\r\n";
            bot.send_com(msg);
            incomingChatMessage.clear();
        }
        bytes_recieved = recv(bot.sockfd, buffer, 4096, 0);
        if(bytes_recieved > 0)
        {
            messages.clear();
            ircMsg = string(buffer, 0, bytes_recieved);

            // trim of the "\r\n" for better command handling
            ircMsg.resize(ircMsg.length() - 2);
            split(ircMsg, " ", messages);

            if(std::find(messages.begin(), messages.end(), "MODE") != messages.end()) 
            {
                bot.send_com(channel_com);
            }

            if(std::find(messages.begin(), messages.end(), "ERROR") != messages.end()) 
            {
                //dealloc if needed
                addrBook.addr_book_close();
                bot.bot_terminate(&core, &proxy);
                string quit_com = "QUIT\r\n";
                bot.send_init_com(quit_com);
                return 0;
            }

            if(messages[0] == "PING")
            {
                string pong = "PONG " + messages[1] + "\r\n";
                bot.send_init_com(pong);
            }
            if(messages[1] == "PRIVMSG")
            {
                
                cout << ircMsg << endl; 
                vector<string> aux;
                split(messages[0], "!", aux);
                string correct_nick = ":" + bot.user_nick;

                if((aux[0] != correct_nick) || (messages[2] != bot.nick))
                {
                    continue;
                }

                string command = messages[3];
                if(command == ":end") /*end*/
                {
                    addrBook.addr_book_close();
                    bot.bot_terminate(&core, &proxy);
                    //dealloc if needed
                    string quit_com = "QUIT\r\n";
                    bot.send_init_com(quit_com);
                    return 0;
                }
                /* set stun and turn */
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
                else if(command == ":mess")
                {
                    string msg;
                    string uri;
                    int start = 5;
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
                    chatRoom.create_chat_room(core._core, uri);

                    string chatMessage;
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
                    string msg;
                    string uri;
                    if(messages[4] == "-a")
                    {
                        if(messages.size() < 6)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :No sip uri provided! call <uri>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        if(!addrBook.addr_book_get_contact(messages[5]))
                        {
                            uri = addrBook.contactUri;
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
                            msg = "PRIVMSG " + bot.user_nick + " :No sip uri provided! call -a <name>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        uri = messages[4];
                    }

                    msg = "PRIVMSG " + bot.user_nick + " :Calling " + uri + "\r\n";
                    bot.send_com(msg);
                    int err = call.call_invite(core._core, uri);
                    if(err == 1)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :An error occured when calling " + uri + "!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    bot.outgoingCallee = uri;

                    bot.call_loop(&call, &core, &callChat, &addrBook, &proxy);
                }
                else if(command == ":register")
                {
                    string msg;

                    if(proxy.proxy_cfg != nullptr && linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationOk)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Already registered as " + bot.sipUsername + "!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(messages[4] == "-a")
                    {
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
                    proxy.bot_register(core._core);

                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationOk)){
                        core.iterate();
                        usleep(20000);
                    }
                    
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
                    msg = "PRIVMSG " + bot.user_nick + " :Unregistering" + bot.sipUsername +"\r\n";
                    bot.send_com(msg);
                    
                    proxy.bot_unregister(core._core);
                    bot.sipUsername.clear();
                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationCleared)){
                        core.iterate();
                        usleep(20000);
                    }
                    
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationCleared)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :Succesfully unregistered!\r\n";
                        bot.send_com(msg);
                        bot.sipUsername.clear();
                        id_com = "PRIVMSG " + bot.user_nick + " :You are now as " + primCont + "!\r\n";
                        bot.send_init_com(id_com);
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
                else if(command == ":accept")
                {
                    string msg;
                    if(incomingCall != nullptr)
                    {
                        if(messages.size() != 5)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Wrong usage! accept <number>\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        if(stoi(messages[4]) > bot.incomingCallsVector.size())
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :No incoming call on this position!\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        int index = stoi(messages[4]) - 1;
                        incomingCall = bot.incomingCallsVector[index];
                        if(incomingCall == nullptr)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :An error accepting call!\r\n";
                            bot.send_com(msg);
                            continue;
                        }

                        int ret = linphone_call_accept(incomingCall);
                        const LinphoneAddress *from = linphone_call_get_remote_address(incomingCall);
                        bot.outgoingCallee =  string(linphone_address_as_string(from));
                        if(ret == 0)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Accepted call from " + bot.outgoingCallee + "!\r\n";
                            bot.send_com(msg);
                        }
                        else
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :An error accepting call!\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        
                        call._call = incomingCall;
                        /* TODO: Do funkce */
                        incomingCall = nullptr;
                        for(int i = 0; i < bot.incomingCallsVector.size(); i++)
                        {
                            int ret = linphone_call_decline(bot.incomingCallsVector[i], LinphoneReasonDeclined);   
                        }
                        bot.incomingCallsVector.clear();
                        bot.order = 0;

                        linphone_call_ref(call._call);
                        
                        linphone_call_set_speaker_muted(call._call, false);
                        linphone_core_enable_mic(core._core, true);

                        bot.call_loop(&call, &core, &callChat, &addrBook, &proxy);
                    }
                    else
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :There is no call to be accepted!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":decline")
                {
                    string msg;
                    if(incomingCall != nullptr)
                    {
                        int ret;
                        /* TODO: Do funkce */
                        incomingCall = nullptr;
                        for(int i = 0; i < bot.incomingCallsVector.size(); i++)
                        {
                            ret = linphone_call_decline(bot.incomingCallsVector[i], LinphoneReasonDeclined);  
                            if(ret != 0)
                            {
                                break;
                            } 
                        }
                        bot.incomingCallsVector.clear();
                        bot.order = 0;
                        
                        if(ret == 0)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Declined all calls!\r\n";
                            bot.send_com(msg);
                        }
                        else
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :An error declining call!\r\n";
                            bot.send_com(msg);
                        }
                    }
                    else
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :There is no call(s) to be declined!\r\n";
                        bot.send_com(msg);
                    }
                }
                else{
                    if(!addrBook.addr_book_iterate(command, messages))
                    {
                        string msg = "PRIVMSG " + bot.user_nick + " :" + addrBook.dbMessage + "\r\n";
                        bot.send_com(msg);
                    }
                    else
                    {
                        string msg = "PRIVMSG " + bot.user_nick + " :Unknown command! Use help for some guidance.\r\n";
                        bot.send_com(msg);
                    }
                }
            }
            messages.clear();
        }
    }

    return 0;
}
