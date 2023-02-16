#ifndef IRC_BOT_MESSAGE_H
#define IRC_BOT_MESSAGE_H

#include <linphone/linphonecore.h>

using namespace std; 

class irc_bot_message
{
public:
    LinphoneChatRoom* chat_room;
    const char *uri;

    irc_bot_message(/* args */);
    ~irc_bot_message();

    void create_chat_room(LinphoneCore *lc, string &uri);
    void create_call_chat_room(LinphoneCall *call);
    void send_message(string &msg);
    
};

#endif