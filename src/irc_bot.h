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

    #include "my_string.cpp"
    #include "irc_bot_core.h"
    #include "irc_bot_call.h"
    #include "irc_bot_proxy.h"

    #include "linphone++/linphone.hh"
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
        int sockfd;

        //send command method
        void send_com(string command);
        int check_messages_during_call(irc_bot_call *call, irc_bot_core *core);

        irc_bot();
    };

#endif