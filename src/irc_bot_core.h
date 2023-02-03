#ifndef IRC_BOT_CORE_H
#define IRC_BOT_CORE_H

#include "linphone++/linphone.hh"
#include <linphone/linphonecore.h>

using namespace std;

class irc_bot_core
{
public:
    LinphoneCoreVTable vTable = {0}; //might not be needed
    LinphoneFactory *_factory;
    LinphoneCore *_core;
    LinphoneCoreCbs *_cbs;
    irc_bot_core();
    ~irc_bot_core();
    void core_create();
    void core_destroy();
    void iterate();
};

#endif