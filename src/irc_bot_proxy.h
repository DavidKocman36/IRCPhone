#ifndef IRC_BOT_PROXY_H
#define IRC_BOT_PROXY_H

#include <iostream>
#include <string>

#include <linphone/linphonecore.h>

using namespace std;

class irc_bot_proxy
{
public:
    const char *user;
    const char *passw;
    LinphoneProxyConfig* proxy_cfg;
    const char* server_addr;

    LinphoneAddress *from;
    LinphoneAuthInfo *info;

    irc_bot_proxy();
    ~irc_bot_proxy();

    void bot_register(LinphoneCore *lc);
    void bot_unregister(LinphoneCore *lc);
    void set_credentials(string &user, string &passw);
};

#endif