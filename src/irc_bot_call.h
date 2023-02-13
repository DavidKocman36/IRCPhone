#ifndef IRC_BOT_CALL_H
#define IRC_BOT_CALL_H

#include <vector>

#include "linphone++/linphone.hh"
#include <linphone/linphonecore.h>
#include <linphone/conference.h>


using namespace std;

class irc_bot_call
{
private:
    /* data */
public:
    //actual call
    LinphoneCall *_call;

    vector<LinphoneCall *> _calls; 

    irc_bot_call();
    ~irc_bot_call();

    void call_invite(LinphoneCore *lc, string uri);
    void call_terminate();
};

#endif