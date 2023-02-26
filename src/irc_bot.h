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
        
        //vector<irc_bot_call>callsVector;
        int order;
        
        bool pausedByRemote;
        int sockfd;

        //send command method
        void send_com(string command);
        int check_messages_during_call(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);
        void bot_terminate(irc_bot_core *core, irc_bot_proxy *proxy);
        void send_init_com(string command);

        void print_status(irc_bot_core *core, irc_bot_proxy *proxy);
        void print_help(vector<string> messages);

        void call_loop(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);

        void call(vector <string>messages, irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy, int opt);

        void accept(vector <string>messages, irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy);

        int decline();
        void decline_func();

        int addr_book_print(vector<string> messages, addr_book &addrBook, irc_bot_core core);
        int print(int n, int x, int &i, int pact, int pmax, addr_book addrBook);

        irc_bot();
    };

#endif