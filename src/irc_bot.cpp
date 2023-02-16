#include "irc_bot.h"

irc_bot::irc_bot()
{
} 

void irc_bot::send_com(string command)
{
    string auxMsg = "PRIVMSG " + user_nick + " : \r\n";
    send(sockfd, auxMsg.c_str(), auxMsg.size(), 0);
    send(sockfd, command.c_str(), command.size(), 0);
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


//metoda pro volani adresaroveho API
void address_book()
{
    //if mess==add
    //  add_address()
    //else if mess==del
    //  del_address()
    //if mess==upd
    //  upd_address()

    return;
}

//metoda pro zachytavani zprav pri volani
/* mozne zpravy pri volani: PING, call, call (pro hold), hangup ,accept, decline, adresarove fce  */
/* bude volana v hlavnim loopu callu po Iterate funkci */
int irc_bot::check_messages_during_call(irc_bot_call *call, irc_bot_core *core, irc_bot_message *callChat)
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
            //jako prvni zkusit implementovat odvhozi hovory
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
            else if(command == ":-m")
            {
                string msg = "PRIVMSG " + user_nick + " :Sending a message\r\n";
                send_init_com(msg);

                string message;
                for(int i = 4; i < messages.size(); i++)
                {
                    message = message + " " + messages[i];
                }
                cout << message << endl;
                callChat->send_message(message);
            }
            else if(command == ":hangup")
            {
                if(call->_call != nullptr)
                {
                    string msg = "PRIVMSG " + user_nick + " :Hanging up!\r\n"; /*TODO: treba vypsat jaky hovor, nebo vsechny*/
                    send_com(msg);
                    /* TODO: Maybe terminate all? */
                    call->call_terminate();
                    return 1;
                }
            }
            else if(command == ":cancel")
            {
                /*init and ringing*/
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

            /* TODO: buggy->no audio after unholding */
            else if(command == ":hold")
            {
                string msg = "PRIVMSG " + user_nick + " :holding call " + outgoingCallee + "\r\n";
                outgoingCallee.clear();
                //call->call_terminate();
                linphone_call_pause(call->_call);
                /* TODO: Play music */
                send_com(msg);

            }
            else if(command == ":resume")
            {
                string msg = "PRIVMSG " + user_nick + " :resuming call " + outgoingCallee + "\r\n";
                outgoingCallee.clear();
                //call->call_terminate();
                linphone_call_resume(call->_call);
                linphone_call_set_speaker_muted(call->_call, false);
                send_com(msg);

            }
            else{
                string msg = "PRIVMSG " + user_nick + " :Unknown command from call! If you want to send message, type with prefix \"-m\"\r\n";
                send_com(msg);
            }
            //TODO: adresar
            /* adresar things */
        }
        messages.clear();
    }

    return 0;
}