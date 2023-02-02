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

    #include "linphone++/linphone.hh"
    #include <linphone/linphonecore.h>
    #include <linphone/conference.h>
    
    
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
        int sockfd;

        //send command method
        void send_com(string command);
        int check_messages_during_call();

        irc_bot();
    };

#endif