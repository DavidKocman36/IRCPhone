#ifndef IRC_BOT_CORE_H
#define IRC_BOT_CORE_H

#include <linphone/linphonecore.h>
#include <algorithm>
#include <vector>
#include "irc_bot_call.h"

using namespace std;

struct IncCall
{
    /*Reference to the incoming call*/
    LinphoneCall *call;
    /*Whether still ringing or not*/
    int status;
};

/* Message used to notify user when someone is calling */
extern string incomingCallMessage;
extern string incomingChatMessage;
extern string remoteHungUp;
/*  */
extern LinphoneCall *incomingCall;

extern vector<irc_bot_call>callsVector;
extern vector<IncCall>incomingCallsVector;

class irc_bot_core
{
public:
    LinphoneFactory *_factory;
    LinphoneCore *_core;
    LinphoneCoreCbs *_cbs;
    LinphoneNatPolicy *_nat;
    LinphoneAuthInfo *_turn_cred;

    irc_bot_core();
    ~irc_bot_core();
    void core_create();
    void core_destroy();
    void iterate();
    void create_nat_policy();
    void enable_stun(string &address);
    void enable_turn(string &user, string &passw);
};

#endif