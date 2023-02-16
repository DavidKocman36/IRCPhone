#include "irc_bot_message.h"

irc_bot_message::irc_bot_message(/* args */)
{
}

irc_bot_message::~irc_bot_message()
{
}

void irc_bot_message::create_call_chat_room(LinphoneCall *call)
{
    this->chat_room=linphone_call_get_chat_room(call);
}

void irc_bot_message::create_chat_room(LinphoneCore *lc, string &uri)
{
    this->uri = uri.c_str();
    //chat_room = linphone_core_get_chat_room_from_uri(lc, this->uri);
    //this->chat_room = linphone_core_search_chat_room(lc, NULL, );
}

void irc_bot_message::send_message(string &msg)
{
    LinphoneChatMessage *chat_message = linphone_chat_room_create_message(this->chat_room, msg.c_str());
    linphone_chat_message_send(chat_message);
}