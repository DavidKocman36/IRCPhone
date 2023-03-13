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
 * 
 * Author: David Kocman
 * 
 */

#include "irc_bot_message.h"

irc_bot_message::irc_bot_message(/* args */)
{
}

irc_bot_message::~irc_bot_message()
{
}

void irc_bot_message::create_call_chat_room(LinphoneCall *call)
{
    //Create call chat room
    this->chat_room=linphone_call_get_chat_room(call);
}

void irc_bot_message::create_chat_room(LinphoneCore *lc, string &uri)
{
    //create the chat room with given uri
    this->uri = uri.c_str();
    LinphoneAddress *addr = linphone_core_create_address(lc, this->uri);
    this->chat_room = linphone_core_get_chat_room(lc, addr);
}

void irc_bot_message::send_message(string &msg)
{
    //Create a new message and send it
    LinphoneChatMessage *chat_message = linphone_chat_room_create_message(this->chat_room, msg.c_str());
    linphone_chat_message_send(chat_message);
}