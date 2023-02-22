#include "irc_bot.h"

irc_bot::irc_bot()
{
    this->order = 0;
} 

void irc_bot::send_com(string command)
{
    string auxMsg = "PRIVMSG " + user_nick + " : \r\n";
    send(sockfd, command.c_str(), command.size(), 0);
    send(sockfd, auxMsg.c_str(), auxMsg.size(), 0);
}

void irc_bot::send_init_com(string command)
{
    send(sockfd, command.c_str(), command.size(), 0);
}

void irc_bot::bot_terminate(irc_bot_core *core, irc_bot_proxy *proxy)
{
    string msg;

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
            send_com(msg);
        }
        else
        {
            cout << "Unegistration failed!" << endl;
            msg = "PRIVMSG " + user_nick + " :Unegistration failed!\r\n";
            send_com(msg);
        }
    }
    
    core->core_destroy();
    msg = "PRIVMSG " + user_nick + " :Bot is ending, bye!\r\n";
    send_com(msg);
}

void irc_bot::print_status(irc_bot_core *core, irc_bot_proxy *proxy)
{
    string msg;
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
    LinphoneCall *currCall = linphone_core_get_current_call(core->_core);
    if(currCall != nullptr)
    {
        const char *callee = linphone_address_as_string(linphone_core_get_current_call_remote_address(core->_core));
        msg = "PRIVMSG " + user_nick + " :In call with " + string(callee) + "!\r\n";
        send_init_com(msg);
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :Not in a call!\r\n";
        send_init_com(msg);
    }
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
            send_init_com(msg);
        }
        else if(messages[4] == "c")
        {
            msg = "PRIVMSG " + user_nick + " :Call commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    call [-a <name>] <uri>: initiates a call to the given uri, with \"-a\" you can dial from contacts\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    accept <number>: accepts incoming call with the given order\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    decline: declines all incoming calls\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    hangup: hangs up current call\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    cancel: cancels outgoing call.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    hold: holds current call.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    resume: resumes current call.\r\n";
            send_init_com(msg);
        }
        else if(messages[4] == "r")
        {
            msg = "PRIVMSG " + user_nick + " :Register commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    register [-a <name>] <uri> <password>: registers your sip account, with \"-a\" you can register from database.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    unregister: unregisters your sip account\r\n";
            send_init_com(msg);
        }
        else if(messages[4] == "m")
        {
            msg = "PRIVMSG " + user_nick + " :Direct messages commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    mess [-a <name>] <uri> <text>: sends a direct message to the given uri, with \"-a\" you can send to a uri from contacts.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -m <text>: sends a direct message to the remote while in a call\r\n";
            send_init_com(msg);
        }
        else if(messages[4] == "a")
        {
            msg = "PRIVMSG " + user_nick + " :Address book commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -d, -c: drops the database (-d), creates the database (-c)\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ic <name> <uri>: inserts a new contact\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ir <name> <uri> <password>: inserts a new proxy identity\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -uc <name> <uri>: updated a contact\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -ur <name> <uri> <password>: updates a proxy identity. Type \"-\" for an attribute you dont want to change.\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -rc <name>, -rr <name>: remove a contact or proxy identity\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :NOTE: All <name> attributes must be unique!\r\n";
            send_init_com(msg);
        }
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :Usage: help <option>\r\n";
        send_init_com(msg);
        msg = "PRIVMSG " + user_nick + " :Options: o (options), c (call commands), r (register commands), m (instant message commands), a (address book commands)\r\n";
        send_init_com(msg);
    }
}

//metoda pro zachytavani zprav pri volani
int irc_bot::check_messages_during_call(irc_bot_call *call, irc_bot_core *core, irc_bot_message *callChat, addr_book *addrBook, irc_bot_proxy *proxy)
{
    vector<string> messages;
    string msg;
    int bytes_recieved = 0;
    char buffer[4096];

    if(!incomingChatMessage.empty())
    {
        string msg = "PRIVMSG " + user_nick + " :" + incomingChatMessage + "\r\n";
        send_com(msg);
        incomingChatMessage.clear();
    }
    if(!incomingCallMessage.empty())
    {
        order += 1;
        string msg = "PRIVMSG " + user_nick + " :" + incomingCallMessage + ". " + "Type \"accept " + to_string(order) + "\" to accept this call!\r\n";
        send_com(msg);
        incomingCallsVector.push_back(incomingCall);
        incomingCallMessage.clear();
    }

    bytes_recieved = recv(sockfd, buffer, 4096, 0);
    if(bytes_recieved > 0)
    {
        msg = string(buffer, 0, bytes_recieved);
        cout << msg;

        // trim of the "\r\n" for better command handling
        msg.resize(msg.length() - 2);
        split(msg, " ", messages);

        if(messages[0] == "PING")
        {
            string pong = "PONG " + messages[1] + "\r\n";
            send_init_com(pong);
        }
        if(messages[1] == "PRIVMSG")
        {
            vector<string> aux;
            split(messages[0], "!", aux);
            string correct_nick = ":" + user_nick;

            if((aux[0] != correct_nick) || (messages[2] != nick))
            {
                return 0;
            }

            string command = messages[3];
            if(command == ":call") /*call sip:aaaa@aah.cz*/
            {
                string msg = "PRIVMSG " + user_nick + " :call\r\n";
                send_com(msg);
                /* call */
                /* zmena state na odchozi hovor -> metoda invite */
                /* vstup do loopu s hovorem */
                /* jinak je hovor pozastaven a vola se s dalsim typkem*/
                /* taky (pokud mozno) pozastavi hovor */

                /* TODO: promyslet moznost zbytecneho zanorovani funkci?? */
            }
            else if(command == ":accept")
            {
                string msg = "PRIVMSG " + user_nick + " :accept\r\n";
                send_com(msg);
                /* accept */
                /* tady pokud existuje prichozi hovor se hovor prijme */
                /* zmeni se state -> metoda accept */
                /* probehne vstup do loopu, ktery obsluhuje hovor (podle me stejny loop jak v hovoru) */

                /* TODO: promyslet moznost zbytecneho zanorovani funkci?? */

            }
            else if(command == ":decline")
            {
                string msg = "PRIVMSG " + user_nick + " :decline\r\n";
                send_com(msg);
                /* decline */
            }
            else if(command == ":status")
            {
                print_status(core, proxy);
            }
            else if(command == ":help")
            {
                print_help(messages);
            }
            else if(command == ":-m")
            {
                string msg = "PRIVMSG " + user_nick + " :Sending a message\r\n";
                send_init_com(msg);

                string message;
                for(int i = 4; i < messages.size(); i++)
                {
                    message = message + " " + messages[i];
                }
                callChat->send_message(message);
            }
            else if(command == ":hangup")
            {
                if(call->_call != nullptr)
                {
                    string msg = "PRIVMSG " + user_nick + " :Hanging up!\r\n";
                    send_com(msg);
                    outgoingCallee.clear();
                    /* TODO: Maybe terminate all? */
                    call->call_terminate();
                    return 1;
                }
            }
            else if(command == ":cancel")
            {
                /*init and ringing*/
                /* TODO: fix this a bit */
                if(call->_call != nullptr && (linphone_call_get_state(call->_call) == LinphoneCallOutgoingInit || linphone_call_get_state(call->_call) == LinphoneCallOutgoingRinging))
                {
                    string msg = "PRIVMSG " + user_nick + " :Cancelling call to " + outgoingCallee + "\r\n";
                    outgoingCallee.clear();
                    call->call_terminate();
                    send_com(msg);
                    return 1;
                }
                else
                {
                    string msg = "PRIVMSG " + user_nick + " :There is no call to be cancelled!\r\n";
                    send_com(msg);
                }
                
            }

            /* TODO: buggy->no audio after unholding sometimes */
            else if(command == ":hold")
            {
                string msg = "PRIVMSG " + user_nick + " :holding call " + outgoingCallee + "\r\n";
                linphone_call_pause(call->_call);
                linphone_call_set_speaker_muted(call->_call, true);
                linphone_core_enable_mic(core->_core, false);
                send_com(msg);
            }
            else if(command == ":resume")
            {
                string msg = "PRIVMSG " + user_nick + " :resuming call " + outgoingCallee + "\r\n";
                linphone_call_resume(call->_call);
                linphone_call_set_speaker_muted(call->_call, false);
                linphone_core_enable_mic(core->_core, true);
                send_com(msg);

            }
            else{
                if(!addrBook->addr_book_iterate(command, messages))
                {
                    string msg = "PRIVMSG " + user_nick + " :" + addrBook->dbMessage + "\r\n";
                    send_com(msg);
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

void irc_bot::call_loop(irc_bot_call *call, irc_bot_core *core, irc_bot_message *callChat, addr_book *addrBook,  irc_bot_proxy *proxy)
{
    int ret = -1;
    bool aux = true;
    string msg;

    while (!(linphone_call_get_state(call->_call) == LinphoneCallReleased) && !(linphone_call_get_state(call->_call) == LinphoneCallStateError))
    {
        core->iterate();
        if(linphone_call_get_state(call->_call) == LinphoneCallConnected && aux)
        {
            msg = "PRIVMSG " + user_nick + " :A call with " + outgoingCallee + " is established!\r\n";
            send_com(msg);
            callChat->create_call_chat_room(call->_call);
            if(callChat->chat_room == nullptr)
            {
                msg = "PRIVMSG " + user_nick + " :Error creating chat room!\r\n";
                send_com(msg);
            }
            msg = "PRIVMSG " + user_nick + " :Chat room established with " + outgoingCallee + "!\r\n";
            send_com(msg);
            linphone_call_set_speaker_muted(call->_call, false);
            aux = false;
        }
        /**
         * TODO: If call is paused by remote (send a message)
        */
        ret = check_messages_during_call(call, core, callChat, addrBook, proxy);
        if(ret == 1)
        {
            break;
        }
    }
    
    msg = "PRIVMSG " + user_nick + " :A call with " + outgoingCallee + " ended!\r\n";
    send_com(msg);
}