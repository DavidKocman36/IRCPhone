#ifndef IRC_BOT_CORE_H
#define IRC_BOT_CORE_H

#include <linphone/linphonecore.h>

using namespace std;

/* Message used to notify user when someone is calling */
extern string incomingCallMessage;
extern string incomingChatMessage;
/*  */
extern LinphoneCall *incomingCall;

class irc_bot_core
{
public:
    LinphoneCoreVTable vTable = {0}; //might not be needed
    LinphoneFactory *_factory;
    LinphoneCore *_core;
    LinphoneCoreCbs *_cbs;
    LinphoneNatPolicy *_nat;
    irc_bot_core();
    ~irc_bot_core();
    void core_create();
    void core_destroy();
    void iterate();
    void create_nat_policy();
};

#endif