#include "irc_bot.h"

/* TODO: All classes moved to global scope so it might cause problems? */
irc_bot bot;
irc_bot_core core;
irc_bot_call call;
irc_bot_proxy proxy;
irc_bot_message callChat;

static void stop(int signum){
	bot.bot_terminate(&core, &proxy);
    exit(0);
}

int main(int argc, char *argv[]){

    // ./irc_bot {ip} {channel} {user} {password} | [port]
    // (mozna port)

    if(argc != 5){
        cout << "Not enogh aruments!" << endl;
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

    while(42)
    {
        core.iterate();
        if(!incomingCallMessage.empty())
        {
            string msg = "PRIVMSG " + bot.user_nick + " :" + incomingCallMessage + "\r\n";
            bot.send_com(msg);
            incomingCallMessage.clear();
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
                bot.bot_terminate(&core, &proxy);
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
                    bot.bot_terminate(&core, &proxy);
                    //dealloc if needed
                    return 0;
                }
                else if(command == ":call")
                {
                    string msg;
                    if(messages.size() < 5)
                    {
                        cout << "No sip uri provided!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :No sip uri provided!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(messages.size() > 5)
                    {
                        cout << "Wrong usage!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Wrong usage!\r\n";
                        bot.send_com(msg);
                        continue;
                    }

                    /* TODO: regex na check adresy */

                    string uri = messages[4];
                    //TODO: Udelat check jestli je user zaregistrovany!
                    msg = "PRIVMSG " + bot.user_nick + " :Calling " + uri + "\r\n";
                    bot.send_com(msg);

                    /* TODO: NAT routing v configs! */
                    int err = call.call_invite(core._core, uri);
                    if(err == 1)
                    {
                        msg = "PRIVMSG " + bot.user_nick + " :An error occured when calling " + uri + "!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    bot.outgoingCallee = uri;
                    int ret = -1;
                    bool aux = true;
                    
                    linphone_call_set_speaker_muted(call._call, false);

                    /* TODO: Hodit do funkce */
                    while (!(linphone_call_get_state(call._call) == LinphoneCallReleased) && !(linphone_call_get_state(call._call) == LinphoneCallStateError))
                    {
                        core.iterate();
                        if(linphone_call_get_state(call._call) == LinphoneCallConnected && aux)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :A call with " + uri + " is established!\r\n";
                            bot.send_com(msg);
                            callChat.create_call_chat_room(call._call);
                            if(callChat.chat_room == nullptr)
                            {
                                cout << "Error creating chat romm!" << endl;
                            }
                            msg = "PRIVMSG " + bot.user_nick + " :Chat room established with " + uri + "!\r\n";
                            bot.send_com(msg);
                            linphone_call_set_speaker_muted(call._call, false);
                            aux = false;
                        }
                        ret = bot.check_messages_during_call(&call, &core, &callChat);
                        if(ret == 1)
                        {
                            break;
                        }
                    }
                    
                    msg = "PRIVMSG " + bot.user_nick + " :A call with " + uri + " ended!\r\n";
                    bot.send_com(msg);
                }
                else if(command == ":register")
                {
                    string msg;
                    if(messages.size() != 6)
                    {
                        cout << "Wrong usage!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Wrong usage!\r\n";
                        bot.send_com(msg);
                        continue;
                    }

                    if(proxy.proxy_cfg != nullptr && linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationOk)
                    {
                        cout << "Already registered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Already registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }

                    /* TODO: Check na uri */

                    msg = "PRIVMSG " + bot.user_nick + " :Registering " + messages[4] + "\r\n";
                    bot.send_com(msg);

                    proxy.set_credentials(messages[4], messages[5]);
                    bot.sipUsername = messages[4];

                    proxy.bot_register(core._core);

                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationOk)){
                        core.iterate();
                        usleep(20000);
                    }
                    
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationOk)
                    {
                        cout << "Succesfully registered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Succesfully registered!\r\n";
                        bot.send_com(msg);
                    }
                    else
                    {
                        cout << "Registration failed!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Registration failed!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":unregister")
                {
                    string msg;
                    if(proxy.proxy_cfg == nullptr)
                    {
                        cout << "Not registered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) != LinphoneRegistrationOk)
                    {
                        cout << "Not registered now!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    msg = "PRIVMSG " + bot.user_nick + " :Unregistering" + bot.sipUsername +"\r\n";
                    bot.send_com(msg);
                    
                    proxy.bot_unregister(core._core);
                    while((linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy.proxy_cfg) !=  LinphoneRegistrationCleared)){
                        core.iterate();
                        usleep(20000);
                    }
                    
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) == LinphoneRegistrationCleared)
                    {
                        cout << "Succesfully unregistered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Succesfully unregistered!\r\n";
                        bot.send_com(msg);
                        bot.sipUsername.clear();
                    }
                    else
                    {
                        cout << "Unegistration failed!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Unegistration failed!\r\n";
                        bot.send_com(msg);
                    }

                }
                else if(command == ":status")
                {
                    string msg;
                    if(proxy.proxy_cfg == nullptr)
                    {
                        cout << "Not registered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    if(linphone_proxy_config_get_state(proxy.proxy_cfg) != LinphoneRegistrationOk)
                    {
                        cout << "Not registered!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Not registered!\r\n";
                        bot.send_com(msg);
                        continue;
                    }
                    else{
                        cout << "Registered to:...!" << endl;
                        msg = "PRIVMSG " + bot.user_nick + " :Registered to " + bot.sipUsername + "!\r\n";
                        bot.send_com(msg);
                    }
                    /* TODO: Seznam Hovoru */
                }
                else if(command == ":accept")
                {
                    string msg;
                    if(incomingCall != nullptr)
                    {
                        int ret = linphone_call_accept(incomingCall);
                        if(ret == 0)
                        {
                            cout << "ACCEPTED!" << endl;
                            msg = "PRIVMSG " + bot.user_nick + " :Accepted call from ...!\r\n";
                            bot.send_com(msg);
                        }
                        else
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :An error accepting call!\r\n";
                            bot.send_com(msg);
                            continue;
                        }
                        
                        call._call = incomingCall;
                        incomingCall = nullptr;
                        linphone_call_ref(call._call);
                        
                        linphone_call_set_speaker_muted(call._call, false);
                        bool aux = true;
                        string msg;
                        //call._calls.push_back(call._call);
                        
                        int exit = -1;
                        /* TODO: Remote adresy */
                        while (!(linphone_call_get_state(call._call) == LinphoneCallReleased) && !(linphone_call_get_state(call._call) == LinphoneCallStateError))
                        {
                            core.iterate();
                            if(linphone_call_get_state(call._call) == LinphoneCallConnected && aux)
                            {
                                msg = "PRIVMSG " + bot.user_nick + " :A call with is established!\r\n";
                                bot.send_com(msg);
                                callChat.create_call_chat_room(call._call);
                                if(callChat.chat_room == nullptr)
                                {
                                    cout << "Error creating chat romm!" << endl;
                                }
                                aux = false;
                            }
                            exit = bot.check_messages_during_call(&call, &core, &callChat);
                            if(exit == 1)
                            {
                                break;
                            }
                        }
                        msg = "PRIVMSG " + bot.user_nick + " :A call with ended!\r\n";
                        bot.send_com(msg);
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
                        int ret = linphone_call_decline(incomingCall, LinphoneReasonDeclined);
                        if(ret == 1)
                        {
                            msg = "PRIVMSG " + bot.user_nick + " :Declined call from ...!\r\n";
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
                        msg = "PRIVMSG " + bot.user_nick + " :There is no call to be declined!\r\n";
                        bot.send_com(msg);
                    }
                }
                else{
                    string msg = "PRIVMSG " + bot.user_nick + " :Unknown command!\r\n";
                    bot.send_com(msg);
                }
                //TODO: adresar
                /* adresar things */
            }
            
            messages.clear();
        }
    }

    return 0;
}
