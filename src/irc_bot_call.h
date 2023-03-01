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

#ifndef IRC_BOT_CALL_H
#define IRC_BOT_CALL_H

#include <vector>

#include <linphone/linphonecore.h>


using namespace std;

class irc_bot_call
{
public:
    /* Actual call */
    LinphoneCall *_call;

    /* Constructor and destructor */
    irc_bot_call();
    ~irc_bot_call();

    /**
     * Initiates an outgoing call
     * 
     * @param lc Current Linphone core
     * @param uri The uri of the remote user in format sip:username@domain
     * @returns 1 on failure, 0 on success
    */
    int call_invite(LinphoneCore *lc, string uri);

    /**
     * Hangs up the current call
    */
    void call_terminate();
};

#endif