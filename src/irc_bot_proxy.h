/*
 * This file is part of IRCPhone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IRC_BOT_PROXY_H
#define IRC_BOT_PROXY_H

#include <iostream>
#include <string>

#include <linphone/linphonecore.h>

using namespace std;

class irc_bot_proxy
{
public:
    /* Username and password used for proxy authentication */
    const char *user;
    const char *passw;
    /* Linphone proxy config */
    LinphoneProxyConfig* proxy_cfg;
    /* The full domain name of the proxy server */
    const char* server_addr;

    /* Identity */
    LinphoneAddress *from;
    /* Authentication info */
    LinphoneAuthInfo *info;

    /* Constructor and destructor */
    irc_bot_proxy();
    ~irc_bot_proxy();

    /**
     * Registers bot to the given proxy with the given credentials.
     * 
     * @param lc Current Linphone Core
     * @returns 1 on failure, 0 on success
    */
    int bot_register(LinphoneCore *lc);

    /**
     * Unregisters bot from current proxy.
     * 
     * @param lc Current Linphone Core
    */
    void bot_unregister(LinphoneCore *lc);

    /**
     * Sets the credential attributes.
     * 
     * @param user Username in the format of sip:username@domain
     * @param passw Password used for the account authentication.
    */
    void set_credentials(string &user, string &passw);
};

#endif