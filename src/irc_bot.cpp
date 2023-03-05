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

#include "irc_bot.h"

irc_bot::irc_bot()
{
    this->order = 0;
    this->pausedByRemote = false;
} 

void irc_bot::send_com(string command)
{
    string auxMsg = "PRIVMSG " + user_nick + " : \r\n";
    send(sockfd, command.c_str(), command.size(), 0);
    /* The empty lina */
    send(sockfd, auxMsg.c_str(), auxMsg.size(), 0);
}

void irc_bot::send_init_com(string command)
{
    send(sockfd, command.c_str(), command.size(), 0);
}

void irc_bot::bot_terminate(irc_bot_core *core, irc_bot_proxy *proxy)
{
    string msg;

    /* Unregister if registered */
    if(proxy->proxy_cfg != nullptr && linphone_proxy_config_get_state(proxy->proxy_cfg) == LinphoneRegistrationOk)
    {
        proxy->bot_unregister(core->_core);
        while((linphone_proxy_config_get_state(proxy->proxy_cfg) !=  LinphoneRegistrationFailed) && (linphone_proxy_config_get_state(proxy->proxy_cfg) !=  LinphoneRegistrationCleared)){
            core->iterate();
            usleep(20000);
        }
        
        if(linphone_proxy_config_get_state(proxy->proxy_cfg) == LinphoneRegistrationCleared)
        {
            cout << "Succesfully unregistered!" << endl;
            msg = "PRIVMSG " + user_nick + " :Succesfully unregistered!\r\n";
            send_init_com(msg);
        }
        else
        {
            cout << "Unegistration failed!" << endl;
            msg = "PRIVMSG " + user_nick + " :Unegistration failed!\r\n";
            send_init_com(msg);
        }
    }
    
    /* Destroy the core and send farewell message */
    core->core_destroy();
    msg = "PRIVMSG " + user_nick + " :Bot is ending, bye!\r\n";
    send_com(msg);
    string quit_com = "QUIT\r\n";
    send_init_com(quit_com);
}

void irc_bot::print_status(irc_bot_core *core, irc_bot_proxy *proxy)
{
    string msg;
    /* If registered */
    if(proxy->proxy_cfg == nullptr)
    {
        msg = "PRIVMSG " + user_nick + " :Not registered!\r\n";
        send_init_com(msg);
        const char *uri = linphone_core_get_primary_contact(core->_core);
        msg = "PRIVMSG " + user_nick + " :Your uri: " + string(uri) + "!\r\n";
        send_init_com(msg);
    }
    else
    {
        if(linphone_proxy_config_get_state(proxy->proxy_cfg) != LinphoneRegistrationOk)
        {
            msg = "PRIVMSG " + user_nick + " :Not registered!\r\n";
            send_init_com(msg);
            const char *uri = linphone_core_get_primary_contact(core->_core);
            msg = "PRIVMSG " + user_nick + " :Your uri: " + string(uri) + "!\r\n";
            send_init_com(msg);
        }
        else{
            msg = "PRIVMSG " + user_nick + " :Registered as " + sipUsername + "!\r\n";
            send_init_com(msg);
        }
    }
    /* If in an active call */
    LinphoneCall *currCall = linphone_core_get_current_call(core->_core);
    if(currCall != nullptr)
    {
        const char *callee = linphone_address_as_string(linphone_core_get_current_call_remote_address(core->_core));
        msg = "PRIVMSG " + user_nick + " :In active call with " + string(callee) + "!\r\n";
        send_init_com(msg);
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :Not in an active call!\r\n";
        send_init_com(msg);
    }

    /* List of all calls (paused and active) */
    if(callsVector.size() > 0)
    {
        string calls;
        int size = 0;
        for(int i = 0; i < callsVector.size(); i++)
        {
            calls += string(linphone_address_get_username(linphone_call_get_remote_address(callsVector.at(i)._call))) + ", ";
            size++;
        }
        calls.erase(calls.size() - 2);
        msg = "PRIVMSG " + user_nick + " :List of all calls (" + to_string(size) + ") - " + calls + "\r\n";
        send_init_com(msg);
    }
    /* If STUN/TURN enabled */
    LinphoneNatPolicy *np = linphone_core_get_nat_policy(core->_core);
    if(linphone_nat_policy_stun_enabled(np))
    {
        const char *stun = linphone_core_get_stun_server(core->_core);
        msg = "PRIVMSG " + user_nick + " :STUN and ICE enabled: " + string(stun) + "!\r\n";
        send_init_com(msg);
        if(linphone_nat_policy_turn_enabled(np))
        {
            msg = "PRIVMSG " + user_nick + " :TURN enabled!\r\n";
            send_com(msg);
        }
        else
        {
            msg = "PRIVMSG " + user_nick + " :TURN not enabled!\r\n";
            send_com(msg);
        }
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :STUN/TURN not enabled!\r\n";
        send_com(msg);
    }
}

void irc_bot::print_help(vector<string> messages)
{
    string msg;
    if(messages.size() > 4)
    {
        if(messages[4] == "o")
        {
            msg = "PRIVMSG " + user_nick + " :Options:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    status: prints your status (registered, current call, your uri)\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -s <server> [-t <user> <passw>]: sets STUN/TURN server (-s) with its credentials (-t).\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    end: ends the bot. Also possible by CTRL+C in terminal.\r\n";
            send_com(msg);
        }
        else if(messages[4] == "c")
        {
            msg = "PRIVMSG " + user_nick + " :Call commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    call [-a <name>] | [-e <number>] | {<uri>}: initiates a call to the given uri, with \"-a\" you can dial from contacts, with \"-e\" you can call with ENUM lookup.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    accept {<number>}: accepts incoming call with the given order\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    decline: declines all incoming calls\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    hangup: hangs up current call\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    hold: holds current call.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    resume: resumes current call.\r\n";
            send_com(msg);
        }
        else if(messages[4] == "r")
        {
            msg = "PRIVMSG " + user_nick + " :Register commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    register [-a <name>] | {<uri>} {<password>}: registers your sip account, with \"-a\" you can register from database.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    unregister: unregisters your sip account\r\n";
            send_com(msg);
        }
        else if(messages[4] == "m")
        {
            msg = "PRIVMSG " + user_nick + " :Direct messages commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    mess {-a <name>} {<text>} | {<uri>} {<text>}: sends a direct message to the given uri, with \"-a\" you can send to a uri from contacts.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -m {<text>}: sends a direct message to the remote you're in a call with (only during call)\r\n";
            send_com(msg);
        }
        else if(messages[4] == "a")
        {
            msg = "PRIVMSG " + user_nick + " :Address book commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -dropdb, -c: drops the database (-dropdb), creates the database (-c)\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ic {<name>} {<uri>}: inserts a new contact\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ir {<name>} {<uri>} {<password>}: inserts a new proxy identity\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -uc {<name>} {<uri>}: updated a contact\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ur {<name>} {<uri>} {<password>}: updates a proxy identity. Type \"-\" for an attribute you dont want to change.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -rc {<name>}, -rr {<name>}: remove a contact or proxy identity\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :NOTE: All <name> attributes must be unique!\r\n";
            send_com(msg);
        }
        else if(messages[4] == "dl")
        {
            msg = "PRIVMSG " + user_nick + " :Database list commands (used for looking at your contacts/registrars):\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -con [<pattern>]: shows list of contacts with optional <pattern> regex. Regex is a standard SQLite regex with % and _ as wildcards.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -reg [<pattern>]: shows a list of your proxy profiles, passwords are not shown.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :These next are only available in browsing mode:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -next [<number>]: turns to the next page or skips <number> of pages.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -prev [<number>]: turns to the previous page or skips <number> of pages.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -exit: updates a proxy identity. Type \"-\" for an attribute you dont want to change.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :NOTE: <pattern> matches only the name attribute! e.g. -con test% prints only contacts starting with \"test\"\r\n";
            send_com(msg);
        }
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :Usage: help {<option>}\r\n";
        send_init_com(msg);
        msg = "PRIVMSG " + user_nick + " :Options: o (options), c (call commands), r (register commands), m (instant message commands), a (address book commands), dl (database list commands)\r\n";
        send_com(msg);
    }
}

int irc_bot::decline()
{
    int ret;
    incomingCall = nullptr;
    IncCall aux;
    for(int i = 0; i < incomingCallsVector.size(); i++)
    {
        /* Decline all incoming calls that are still ringing */
        aux = incomingCallsVector.at(i);
        if(aux.status == 1)
        {
            ret = linphone_call_decline(aux.call, LinphoneReasonDeclined);
            if(ret != 0)
            {
                return ret;
            }
        }
    }  
    incomingCallsVector.clear();
    order = 0;
    return 0;
}

void irc_bot::decline_func()
{
    string msg;
    if(incomingCall != nullptr)
    {
        int ret;
        ret = decline();
        if(ret == 0)
        {
            msg = "PRIVMSG " + user_nick + " :Declined all calls!\r\n";
            send_com(msg);
        }
        else
        {
            msg = "PRIVMSG " + user_nick + " :An error declining call!\r\n";
            send_com(msg);
        }
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :There are no calls to be declined!\r\n";
        send_com(msg);
    }
}

int irc_bot::check_messages_during_call(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook, irc_bot_proxy &proxy)
{
    vector<string> messages;
    string msg;
    int bytes_recieved = 0;
    char buffer[4096];
    
    /* If an instant message comes */
    if(!incomingChatMessage.empty())
    {
        string msg = "PRIVMSG " + user_nick + " :" + incomingChatMessage + "\r\n";
        send_com(msg);
        incomingChatMessage.clear();
    }
    /* If someone calls */
    if(!incomingCallMessage.empty())
    {
        order += 1;
        string msg = "PRIVMSG " + user_nick + " :" + incomingCallMessage + ". " + "Type \"accept " + to_string(order) + "\" to accept this call!\r\n";
        send_com(msg);
        incomingCallMessage.clear();
    }
    /* If remote hangs up */
    if(!remoteHungUp.empty())
    {
        string msg = "PRIVMSG " + user_nick + " :" + remoteHungUp + "\r\n";
        send_com(msg);
        remoteHungUp.clear();
    }

    /* Receive IRC messages */
    bytes_recieved = recv(sockfd, buffer, 4096, 0);
    if(bytes_recieved > 0)
    {
        msg = string(buffer, 0, bytes_recieved);
        memset(buffer, 0, sizeof(buffer));
        cout << msg;

        // trim of the "\r\n" for better command handling
        msg.resize(msg.length() - 2);
        split(msg, " ", messages);

        std::vector<std::string>::iterator in;
        if((in = std::find(messages.begin(), messages.end(), "PING")) != messages.end())
        {
            int index = in - messages.begin() + 1;
            string pong = "PONG " + messages[index] + "\r\n";
            cout << pong << endl;
            send_init_com(pong);
            messages.clear();
            memset(buffer, 0, sizeof(buffer));
        }
        if(messages[1] == "PRIVMSG")
        {
            vector<string> aux;
            split(messages[0], "!", aux);
            string correct_nick = ":" + user_nick;
            /* Ignore messages from other users */
            if((aux[0] != correct_nick) || (messages[2] != nick))
            {
                return 0;
            }

            string command = messages[3];
            if(command == ":call")
            {
                /* initiate a call */
                this->call(messages, call, core, callChat, addrBook, proxy, 1);
            }
            else if(command == ":accept")
            {
                /* Accept a call if there is one */
                if(incomingCall != nullptr)
                {       
                    this->accept(messages, call, core, callChat, addrBook, proxy);
                }
                else
                {
                    msg = "PRIVMSG " + user_nick + " :There is no call to be accepted!\r\n";
                    send_com(msg);
                }
            }
            else if(command == ":decline")
            {
                /* Decline a call */
                this->decline_func();
            }
            else if(command == ":status")
            {
                print_status(&core, &proxy);
            }
            else if(command == ":help")
            {
                print_help(messages);
            }
            else if(command == ":-m")
            {
                /* Send an instant message to the user you are in call with */
                string msg = "PRIVMSG " + user_nick + " :Sending a message\r\n";
                send_init_com(msg);

                string message;
                for(int i = 4; i < messages.size(); i++)
                {
                    /* Concatenate the text */
                    message = message + " " + messages[i];
                }
                callChat.send_message(message);
            }
            else if(command == ":mess")
            {
                string msg;
                string uri;
                int start = 5;
                /* Send to a remote in contacts */
                if(messages[4] == "-a")
                {
                    if(messages.size() < 6)
                    {
                        msg = "PRIVMSG " + user_nick + " :Wrong usage! mess -a <name> <text>\r\n";
                        send_com(msg);
                        return 0;
                    }
                    if(!addrBook.addr_book_get_contact(messages[5]))
                    {
                        uri = addrBook.contactUri;
                        start = 6;
                    }   
                    else
                    {
                        msg = "PRIVMSG " + user_nick + " :" + addrBook.dbMessage + "\r\n";
                        send_com(msg);
                        return 0;
                    }
                }
                else
                {
                    if(messages.size() < 5)
                    {
                        msg = "PRIVMSG " + user_nick + " :Wrong usage! mess <uri> <text>\r\n";
                        send_com(msg);
                        return 0;
                    }
                    uri = messages[4];
                    start = 5;
                }
                irc_bot_message chatRoom;
                chatRoom.create_chat_room(core._core, uri);

                string chatMessage;
                /* Create and send the message */
                for(int i = start; i < messages.size(); i++)
                {
                    chatMessage = chatMessage + " " + messages[i];
                }
                chatRoom.send_message(chatMessage);
                msg = "PRIVMSG " + user_nick + " :Sending a message\r\n";
                send_init_com(msg);
            }
            else if(command == ":hangup")
            {
                /* Hangup the current call */
                call = callsVector.back(); 
                if(call._call != nullptr)
                {
                    string msg = "PRIVMSG " + user_nick + " :Hanging up!\r\n";
                    send_init_com(msg);
                    call.call_terminate();
                    return 1;
                }
            }
            else if(command == ":hold")
            {
                /* Put the current call on hold if possible */
                if(linphone_call_get_state(call._call) == LinphoneCallStateConnected)
                {
                    string msg = "PRIVMSG " + user_nick + " :Holding call " + outgoingCallee + "\r\n";
                    linphone_call_pause(call._call);
                    linphone_call_set_speaker_muted(call._call, true);
                    linphone_core_enable_mic(core._core, false);
                    send_com(msg);
                }
                else
                {
                    string msg = "PRIVMSG " + user_nick + " :Cannot get this call on hold!\r\n";
                    send_com(msg);
                }
                
            }
            else if(command == ":resume")
            {
                /* Resume the current call if possible */
                if(linphone_call_get_state(call._call) != LinphoneCallStateConnected)
                {
                    string msg = "PRIVMSG " + user_nick + " :Resuming call " + outgoingCallee + "\r\n";
                    linphone_call_resume(call._call);
                    linphone_call_set_speaker_muted(call._call, false);
                    linphone_core_enable_mic(core._core, true);
                    send_com(msg);
                }
                else
                {
                    string msg = "PRIVMSG " + user_nick + " :Cannot resume this call!\r\n";
                    send_com(msg);
                }

            }
            else{
                /* Address book commands */
                if(!addrBook.addr_book_iterate(command, messages))
                {
                    string msg = "PRIVMSG " + user_nick + " :" + addrBook.dbMessage + "\r\n";
                    send_com(msg);
                }
                /* Browse database */
                else if(command == ":-con")
                {
                    addrBook.addr_book_get_data(messages, Contact);
                    addr_book_print(messages, addrBook, core);
                }
                else if(command == ":-reg")
                {
                    addrBook.addr_book_get_data(messages, Registrar);
                    addr_book_print(messages, addrBook, core);
                }
                else
                {
                    string msg = "PRIVMSG " + user_nick + " :Unknown command! If you want to send message, type with prefix \"-m\"\r\n";
                    send_com(msg);
                }
            }
        }
        messages.clear();
    }

    return 0;
}

void irc_bot::call_loop(irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy)
{
    int ret = -1;
    bool aux = true;
    bool isOnHold = false;
    string msg;
    irc_bot_call auxCall = call;

    cout << endl;
    cout << "VSTUPUJU DO LOOPU" << endl;
    cout << endl;
    while (!(linphone_call_get_state(call._call) == LinphoneCallStateReleased) && !(linphone_call_get_state(call._call) == LinphoneCallStateError))
    {
        core.iterate();
        if(linphone_call_get_state(call._call) == LinphoneCallStateConnected && aux)
        {
            /* If call is successfully connected then let the user know and create chat room  */
            msg = "PRIVMSG " + user_nick + " :A call with " + outgoingCallee + " is established!\r\n";
            send_init_com(msg);
            callChat.create_call_chat_room(call._call);
            if(callChat.chat_room == nullptr)
            {
                msg = "PRIVMSG " + user_nick + " :Error creating chat room!\r\n";
                send_com(msg);
            }
            msg = "PRIVMSG " + user_nick + " :Chat room established with " + outgoingCallee + "!\r\n";
            send_com(msg);
            linphone_call_set_speaker_muted(call._call, false);
            aux = false;
        }
        /* Let the user know if the call was put on hold/resumed by remote */
        if(!isOnHold && linphone_call_get_state(call._call) == LinphoneCallStateEnd)
        {
            msg = "PRIVMSG " + user_nick + " :" + outgoingCallee + " put the call on hold!\r\n";
            send_init_com(msg);
            isOnHold = true;
        }
        if(isOnHold && linphone_call_get_state(call._call) == LinphoneCallStateConnected)
        {
            msg = "PRIVMSG " + user_nick + " :" + outgoingCallee + " resumed the call!\r\n";
            send_init_com(msg);
            isOnHold = false;
        }
        /* If the call encounters an error - especially when calling wrong uri */
        if(linphone_call_get_state(call._call) == LinphoneCallStateUpdating)
        {
            msg = "PRIVMSG " + user_nick + " :Could not call " + outgoingCallee + "! Wrong uri!\r\n";
            send_com(msg);
            break;
        }
        ret = this->check_messages_during_call(call, core, callChat, addrBook, proxy);
        if(ret == 1)
        {
            break;
        }
        /* Get the current call. Useful when having more than one call. */
        /* If the call shall be still active, then it is present in calls vector */
        /* If not, the loop is immediately terminated */
        if(!callsVector.empty())
        {
            bool found = false;
            for(int i = 0; i < callsVector.size(); i++)
            {
                if(auxCall._call == callsVector.at(i)._call)
                {
                    call = callsVector.at(i);
                    outgoingCallee = linphone_address_as_string(linphone_call_get_remote_address(call._call));
                    found = true;
                }
            }
            if(!found)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    cout << endl;
    cout << "VYSTUPUJU ret=" << ret << endl;
    cout << "SIZE: " << callsVector.size() << endl;
    cout << endl;
}

void irc_bot::call(vector <string>messages, irc_bot_call &currentCall, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy, int opt)
{
    string msg;
    string uri;
    irc_bot_call call;
    if(messages.size() < 5)
    {
        msg = "PRIVMSG " + user_nick + " :Wrong usage! call [-a <name>] <uri>\r\n";
        send_com(msg);
        return;
    }
    /* Calling from contacts */
    if(messages[4] == "-a")
    {
        if(messages.size() < 6)
        {
            msg = "PRIVMSG " + user_nick + " :No name provided! call -a <uri>\r\n";
            send_com(msg);
            return;
        }
        if(!addrBook.addr_book_get_contact(messages[5]))
        {
            uri = addrBook.contactUri;
        }   
        else
        {
            msg = "PRIVMSG " + user_nick + " :" + addrBook.dbMessage + "\r\n";
            send_com(msg);
            return;
        }
    }
    /* Calling with a phone number */
    else if(messages[4] == "-e")
    {
        if(messages.size() < 6)
        {
            msg = "PRIVMSG " + user_nick + " :No number provided! call -e <number>\r\n";
            send_com(msg);
        }
        /* NAPTR lookup */
        uri = addrBook.get_enum_uri(messages);
        if(uri.empty())
        {
            msg = "PRIVMSG " + user_nick + " :Provided phone number does not map to a sip URI!\r\n";
            send_com(msg);
            return;
        }
    }
    else
    {
        if(messages.size() < 5)
        {
            msg = "PRIVMSG " + user_nick + " :No sip uri provided! call -a <name>\r\n";
            send_com(msg);
            return;
        }
        uri = messages[4];
    }

    msg = "PRIVMSG " + user_nick + " :Calling " + uri + "\r\n";
    send_com(msg);
    /* Initiate the call */
    int err = call.call_invite(core._core, uri);
    if(err == 1)
    {
        msg = "PRIVMSG " + user_nick + " :An error occured when calling " + uri + "!\r\n";
        send_com(msg);
        return;
    }
    outgoingCallee = string(linphone_address_as_string(linphone_call_get_remote_address(call._call)));
    /* Push the call into the active calls vector */
    callsVector.push_back(call);
    call_loop(call, core, callChat, addrBook, proxy);
}

void irc_bot::accept(vector <string>messages, irc_bot_call &call, irc_bot_core &core, irc_bot_message &callChat, addr_book &addrBook,  irc_bot_proxy &proxy)
{
    string msg;
    if(messages.size() != 5)
    {
        msg = "PRIVMSG " + user_nick + " :Wrong usage! accept <number>\r\n";
        send_com(msg);
        return;
    }
    if(stoi(messages[4]) > incomingCallsVector.size())
    {
        msg = "PRIVMSG " + user_nick + " :No incoming call on this position!\r\n";
        send_com(msg);
        return;
    }
    /* Get the index of the call in incoming calls vector */
    int index = stoi(messages[4]) - 1;
    IncCall incCall = incomingCallsVector[index];
    if(incCall.status == 0)
    {
        msg = "PRIVMSG " + user_nick + " :No incoming call on this position!\r\n";
        send_com(msg);
        return;
    }

    incomingCall = incCall.call;
    if(incomingCall == nullptr)
    {
        msg = "PRIVMSG " + user_nick + " :An error accepting call!\r\n";
        send_com(msg);
        return;
    }

    /* Accept the call */
    int ret = linphone_call_accept(incomingCall);
    const LinphoneAddress *from = linphone_call_get_remote_address(incomingCall);
    outgoingCallee =  string(linphone_address_as_string(from));
    if(ret == 0)
    {
        msg = "PRIVMSG " + user_nick + " :Accepted call from " + outgoingCallee + "!\r\n";
        send_com(msg);
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :An error accepting call!\r\n";
        send_com(msg);
        return;
    }
    
    call._call = incomingCall;
    incomingCall = nullptr;
    IncCall aux;
    /* Decline all other calls */
    for(int i = 0; i < incomingCallsVector.size(); i++)
    {
        aux = incomingCallsVector.at(i);
        if(aux.status == 1)
        {
            ret = linphone_call_decline(aux.call, LinphoneReasonDeclined);
        }
    }
    incomingCallsVector.clear();
    order = 0;

    linphone_call_ref(call._call);
    /* Push the call into the active calls vector */
    callsVector.push_back(call);
    call_loop(call, core, callChat, addrBook, proxy);
}

int irc_bot::print(int n, int x, int &i, int pact, int pmax, addr_book addrBook)
{
    string msg;
    Data data;
    for(i = n; i < x; i++)
    {
        data = addrBook.dbData.at(i);
        msg = "PRIVMSG " + user_nick + " :" + data.name + " - " + data.uri + "\r\n";
        send_init_com(msg);
    }
    msg = "PRIVMSG " + user_nick + " :---------- Page " + to_string(pact) +"/" + to_string(pmax) + " ----------\r\n";
    send_init_com(msg);
    return i;
}

int irc_bot::addr_book_print(vector<string> messages, addr_book &addrBook, irc_bot_core core)
{
    string msg;
    if(addrBook.dbData.empty())
    {
        msg = "PRIVMSG " + user_nick + " :No result!\r\n";
        send_com(msg);
        return 0;
    }
    int i;
    int number = 1;
    int n = 0;
    int x = 5;
    int pact = 1;
    /* Compute the number of pages */
    double pmax = std::ceil(addrBook.dbData.size() / 5.0);
    int size = addrBook.dbData.size();
    /* This is the first index of data on the last page */
    int lastPageIndex = addrBook.dbData.size() - (addrBook.dbData.size() % 5 );

    /* The max number of data on each page is 5 so if it is less than 5 the x is callibrated */
    if(x > size)
        x = size;
    else
        x = 5;
    
    
    msg = "PRIVMSG " + user_nick + " :Now in database browsing mode!\r\n";
    send_init_com(msg);
    
    /* Print the first page */
    i = print(n, x, i, pact, pmax,addrBook);

    bool running = true;
    char buffer[4096];
    while(running)
    {
        /* If this function is called from a call then the core should still do the background work */
        core.iterate();
        /* Receive commands */
        int bytes_recieved = recv(sockfd, buffer, 4096, 0);
        if(bytes_recieved > 0){
            messages.clear();
            string ircMsg = string(buffer, 0, bytes_recieved);
            memset(buffer, 0, sizeof buffer);
            // trim of the "\r\n" for better command handling
            ircMsg.resize(ircMsg.length() - 2);
            split(ircMsg, " ", messages);

            std::vector<std::string>::iterator in;
            if((in = std::find(messages.begin(), messages.end(), "PING")) != messages.end())
            {
                int index = in - messages.begin() + 1;
                string pong = "PONG " + messages[index] + "\r\n";
                send_init_com(pong);
                messages.clear();
                memset(buffer, 0, sizeof(buffer));
            }
            if(messages[1] == "PRIVMSG")
            {
                vector<string> aux;
                split(messages[0], "!", aux);
                string correct_nick = ":" + user_nick;

                if((aux[0] != correct_nick) || (messages[2] != nick))
                {
                    continue;
                }

                if(messages[3] == ":exit")
                {
                    running = false;
                }
                else if(messages[3] == ":next")
                {
                    /* If user provides a number of pages to skip. Else turn to next page (1) */
                    if(messages.size() > 4)
                    {
                        if(!strIsDigit(messages[4]))
                        {
                            msg = "PRIVMSG " + user_nick + " :Command expects a positive number!\r\n";
                            send_init_com(msg);
                            continue;
                        }
                        number = stoi(messages[4]);
                    }
                    else
                    {
                        number = 1;
                    }
                    /* Increase actual page number */
                    pact += number;
                    /* If it is overflowed then correct it */
                    if(pact > pmax)
                        pact = pmax;
                    /* Compute the next starting index */
                    n += number * 5;
                    /* If it overflowed then correct it */
                    if(n > size)
                        n = lastPageIndex;
                    /* Compute the last data index */
                    x += number * 5;
                    /* If it overflowed then correct it */
                    if(x > size)
                        x = size;
                    
                    i = print(n, x, i, pact, pmax, addrBook);
                }
                else if(messages[3] == ":prev")
                {
                    if(messages.size() > 4)
                    {
                        if(!strIsDigit(messages[4]))
                        {
                            msg = "PRIVMSG " + user_nick + " :Command expects a positive number!\r\n";
                            send_init_com(msg);
                            continue;
                        }
                        number = stoi(messages[4]);
                    }
                    else
                    {
                        number = 1;
                    }
                    /* Same as in "next" but with - */
                    pact -= number;
                    if(pact < 0)
                        pact = 1;
                    n -= number * 5;
                    if(n < 0)
                        n = 0;
                    x = n + 5;
                        /* Here if user types "prev" on first page containing < 5 data then x is corrected */
                    if(x > size)
                        x = size;
                    cout << n << " " << x << endl;
                    i = print(n, x, i, pact, pmax, addrBook);
                }
                else
                {
                    msg = "PRIVMSG " + user_nick + " :Unknown command in browsing mode! Use \"prev <number>\", \"exit\" or \"next <number>\".\r\n";
                    send_com(msg);
                }
            }
        }
    }
    msg = "PRIVMSG " + user_nick + " :Exiting database browsing mode!\r\n";
    send_com(msg);
    /* Clear the data vector */
    addrBook.dbData.clear();
    return 0;
}
