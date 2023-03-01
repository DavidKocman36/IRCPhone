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

#ifndef IRC_BOT_MESSAGE_H
#define IRC_BOT_MESSAGE_H

#include <linphone/linphonecore.h>

using namespace std; 

class irc_bot_message
{
public:
    /* The chat room object */
    LinphoneChatRoom* chat_room;
    /* The uri of the receiver */
    const char *uri;

    /* Constructor and destructor */
    irc_bot_message();
    ~irc_bot_message();

    /**
     * Creates a chat room with given uri to send instatnt messages.
     * 
     * @param lc Current Linphone core
     * @param uri The uri of the recipient in a format sip:username@domain
    */
    void create_chat_room(LinphoneCore *lc, string &uri);

    /**
     * Creates a chat room with current person in call with.
     * 
     * @param call Current call
    */
    void create_call_chat_room(LinphoneCall *call);

    /**
     * Sends direct message
     * 
     * @param msg The message
    */
    void send_message(string &msg);
    
};

#endif