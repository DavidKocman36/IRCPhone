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

    #include "my_string.h"
    #include "irc_bot_core.h"
    #include "irc_bot_call.h"
    #include "irc_bot_proxy.h"
    #include "irc_bot_message.h"

    #include <linphone/linphonecore.h>
    
    using namespace std;

    //BOT bude koncipovan jako objekt
    class irc_bot
    {
    public:
        string server;
        string channel;
        string user_nick;
        string password;
        string nick;
        string outgoingCallee;

        string sipUsername;
        
        int sockfd;

        //send command method
        void send_com(string command);
        int check_messages_during_call(irc_bot_call *call, irc_bot_core *core, irc_bot_message *callChat);
        void bot_terminate(irc_bot_core *core, irc_bot_proxy *proxy);
        void send_init_com(string command);

        irc_bot();
    };

#endif