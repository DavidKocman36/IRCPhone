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
            send_init_com(msg);
        }
        else
        {
            cout << "Unegistration failed!" << endl;
            msg = "PRIVMSG " + user_nick + " :Unegistration failed!\r\n";
            send_init_com(msg);
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
    /* TODO: list of all calls */
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
            //msg = "PRIVMSG " + user_nick + " :    cancel: cancels outgoing call.\r\n";
            //send_init_com(msg);
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
            msg = "PRIVMSG " + user_nick + " :    -m <text>: sends a direct message to the remote you're in a call with (only during call)\r\n";
            send_init_com(msg);
        }
        else if(messages[4] == "a")
        {
            msg = "PRIVMSG " + user_nick + " :Address book commands:\r\n";
            send_init_com(msg);
            msg = "PRIVMSG " + user_nick + " :    -dropdb, -c: drops the database (-dropdb), creates the database (-c)\r\n";
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
            send_init_com(msg);
        }
    }
    else
    {
        msg = "PRIVMSG " + user_nick + " :Usage: help <option>\r\n";
        send_init_com(msg);
        msg = "PRIVMSG " + user_nick + " :Options: o (options), c (call commands), r (register commands), m (instant message commands), a (address book commands), dl (database list commands)\r\n";
        send_init_com(msg);
    }
}

int irc_bot::decline()
{
    int ret;
    incomingCall = nullptr;
    IncCall aux;
    for(int i = 0; i < incomingCallsVector.size(); i++)
    {
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
        incomingCallMessage.clear();
    }
    if(!remoteHungUp.empty())
    {
        string msg = "PRIVMSG " + user_nick + " :" + remoteHungUp + "\r\n";
        send_com(msg);
        remoteHungUp.clear();
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
                this->call(messages, call, core, callChat, addrBook, proxy, 1);
            }
            else if(command == ":accept")
            {
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
                string msg = "PRIVMSG " + user_nick + " :Sending a message\r\n";
                send_init_com(msg);

                string message;
                for(int i = 4; i < messages.size(); i++)
                {
                    message = message + " " + messages[i];
                }
                callChat.send_message(message);
            }
            else if(command == ":hangup")
            {
                call = callsVector.back(); 
                if(call._call != nullptr)
                {
                    string msg = "PRIVMSG " + user_nick + " :Hanging up!\r\n";
                    send_init_com(msg);
                    callsVector.pop_back();
                    call.call_terminate();
                    return 1;
                }
            }
            /* TODO: check whether I can hold/resume */
            else if(command == ":hold")
            {
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
                if(!addrBook.addr_book_iterate(command, messages))
                {
                    string msg = "PRIVMSG " + user_nick + " :" + addrBook.dbMessage + "\r\n";
                    send_com(msg);
                }
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
    cout << endl;
    cout << "VSTUPUJU DO LOOPU" << endl;
    cout << endl;
    call = callsVector.back(); 
    while (!(linphone_call_get_state(call._call) == LinphoneCallStateReleased) && !(linphone_call_get_state(call._call) == LinphoneCallStateError))
    {
        core.iterate();
        if(linphone_call_get_state(call._call) == LinphoneCallStateConnected && aux)
        {
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
        if(linphone_call_get_state(call._call) == LinphoneCallStateUpdating)
        {
            msg = "PRIVMSG " + user_nick + " :Could not call " + outgoingCallee + "!\r\n";
            send_com(msg);
            break;
        }
        ret = this->check_messages_during_call(call, core, callChat, addrBook, proxy);
        if(ret == 1)
        {
            break;
        }
        if(!callsVector.empty())
        {
            call = callsVector.back(); 
            outgoingCallee = string(linphone_address_as_string(linphone_call_get_remote_address(call._call)));
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
    else if(messages[4] == "-e")
    {
        if(messages.size() < 6)
        {
            msg = "PRIVMSG " + user_nick + " :No number provided! call -a <uri>\r\n";
            send_com(msg);
        }
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
    int err = call.call_invite(core._core, uri);
    if(err == 1)
    {
        msg = "PRIVMSG " + user_nick + " :An error occured when calling " + uri + "!\r\n";
        send_com(msg);
        return;
    }
    outgoingCallee = string(linphone_address_as_string(linphone_call_get_remote_address(call._call)));
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
    double pmax = std::ceil(addrBook.dbData.size() / 5.0);
    int size = addrBook.dbData.size();
    int lastPageIndex = addrBook.dbData.size() - (addrBook.dbData.size() % 5 );

    if(x > size)
        x = size;
    else
        x = 5;
    
    
    msg = "PRIVMSG " + user_nick + " :Now in database browsing mode!\r\n";
    send_init_com(msg);
    

    i = print(n, x, i, pact, pmax,addrBook);

    bool running = true;
    char buffer[4096];
    while(running)
    {
        core.iterate();
        int bytes_recieved = recv(sockfd, buffer, 4096, 0);
        if(bytes_recieved > 0){
            messages.clear();
            string ircMsg = string(buffer, 0, bytes_recieved);
            memset(buffer, 0, sizeof buffer);
            // trim of the "\r\n" for better command handling
            ircMsg.resize(ircMsg.length() - 2);
            split(ircMsg, " ", messages);

            if(messages[0] == "PING")
            {
                string pong = "PONG " + messages[1] + "\r\n";
                send_init_com(pong);
            }
            if(messages[1] == "PRIVMSG")
            {
                cout << ircMsg << endl; 
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
                    if(messages.size() > 5)
                    {
                        number = stoi(messages[5]);
                    }
                    else
                    {
                        number = 1;
                    }
                    pact += number;
                    if(pact > pmax)
                        pact = pmax;
                    n += number * 5;
                    if(n > size)
                        n = lastPageIndex;
                    x += number * 5;
                    if(x > size)
                        x = size;
                    
                    i = print(n, x, i, pact, pmax, addrBook);
                }
                else if(messages[3] == ":prev")
                {
                    if(messages.size() > 5)
                    {
                        number = stoi(messages[5]);
                    }
                    else
                    {
                        number = 1;
                    }
                    pact -= number;
                    if(pact < 0)
                        pact = 1;
                    n -= number * 5;
                    if(n < 0)
                        n = 0;
                    x -= number * 5;
                    if(x < 5)
                        x = 5;
                        if(x > size)
                            x = size;
                    
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
    addrBook.dbData.clear();
    return 0;
}
