#include "irc_bot.h"
#include "irc_bot_core.h"

irc_bot::irc_bot()
{
} 

void irc_bot::send_com(string command)
{
    send(sockfd, command.c_str(), command.size(), 0);
}

/* Udelat callback funkci call state changed !!!! */

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

int irc_bot::check_messages_during_call()
{
    vector<string> messages;
    string msg;
    int bytes_recieved = 0;
    char buffer[4096];

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
            send_com(pong);
        }
        if(messages[1] == "PRIVMSG")
        {
            //TODO: Integrace liblinphone knihovny
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
                /* zmena: pokud je jiz v hovoru a dalsimu zavola tak se musi spustit konference!! */
                /* jinak je hovor pozastaven a vola se s dalsim typkem*/
                /* taky (pokud mozno) pozastavi hovor */

                /* myslenka: metoda invite -> docasny loop a pri prijeti add to conference a pokracuje dale v teto fci */
                /* if (streams running) */
                /*     add to conference (asi aj samo unpausne)*/

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
                /* zmena: pokud je jiz v hovoru, tak se vytvori konference!! */
                /* jinak je hovor pozastaven a vola se s dalsim typkem*/
                /* taky (pokud mozno) pozastavi hovor */

                /* TODO: promyslet moznost zbytecneho zanorovani funkci?? */

            }
            else if(command == ":decline")
            {
                string msg = "PRIVMSG " + user_nick + " :decline\r\n";
                send_com(msg);
                /* decline */
            }
            else if(command == ":hangup")
            {
                string msg = "PRIVMSG " + user_nick + " :Hanging up!\r\n"; /*TODO: treba vypsat jaky hovor, nebo vsechny*/
                send_com(msg);
                return 1;
                /* hangup */
            }
            else{
                string msg = "PRIVMSG " + user_nick + " :Unknown command z callu!\r\n";
                send_com(msg);
            }
            //TODO: adresar
            /* adresar things */
        }
        messages.clear();
    }

    return 0;
}

int main(int argc, char *argv[]){

    // ./irc_bot {ip} {channel} {user} {password} | [port]
    // (mozna port)

    if(argc != 5){
        cout << "Not enogh aruments!" << endl;
        cout << "USAGE: ./irc_bot {server} {channel} {user_nick} {password}" << endl;
        return 1;
    }

    irc_bot bot;
    irc_bot_core core;
    core.core_create();
    cout << "vytvořeno" << endl;

    bot.server = argv[1];
    bot.channel = argv[2];

    if(bot.channel[0] != '#')
    {
        bot.channel = "#" + bot.channel;
    }

    bot.user_nick = argv[3];
    bot.password = argv[4];

    bot.nick = bot.user_nick + "_b";

    //resolve hostname
    struct hostent *he;    
    if ( (he = gethostbyname(bot.server.c_str()) ) == NULL ) {
        exit(1); 
    }

    bot.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bot.sockfd < 0)
    {
        perror("Socket:");
        return 1;
    }
    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 20000;
    
    if (setsockopt (bot.sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    {
        perror("Setsockopt: \n");
    }
        
    struct sockaddr_in irc_server;
    irc_server.sin_family = AF_INET;
    irc_server.sin_port = htons(6667);
    memcpy(&irc_server.sin_addr, he->h_addr_list[0], he->h_length);

    int conn = connect(bot.sockfd, (sockaddr*)&irc_server, sizeof(irc_server));
    if(conn < 0){
        perror("Socket:");
        return 1;
    }
    else{
        cout << "Connected: " << bot.server << endl;
    }

    char buffer[4096];

    string channel_com = "JOIN " + bot.channel + "\r\n";
    string password_com = "PASS " + bot.password + "\r\n";
    string user_com = "USER " + bot.nick + " 0 * :" + bot.user_nick + "\r\n";
    string nick_com = "NICK " + bot.nick + "\r\n";
    vector<string> messages;
    string msg;
    int bytes_recieved = 0;

    bot.send_com(password_com);
    bot.send_com(nick_com);
    bot.send_com(user_com);

    while(42)
    {
        bytes_recieved = recv(bot.sockfd, buffer, 4096, 0);
        if(bytes_recieved > 0)
        {
            msg = string(buffer, 0, bytes_recieved);
            cout << msg;

            // trim of the "\r\n" for better command handling
            msg.resize(msg.length() - 2);
            split(msg, " ", messages);

            if(std::find(messages.begin(), messages.end(), "MODE") != messages.end()) 
            {
                bot.send_com(channel_com);
            }

            if(std::find(messages.begin(), messages.end(), "ERROR") != messages.end()) 
            {
                //dealloc if needed
                return 0;
            }

            if(messages[0] == "PING")
            {
                string pong = "PONG " + messages[1] + "\r\n";
                bot.send_com(pong);
            }
            if(messages[1] == "PRIVMSG")
            {
                
                //jako prvni zkusit implementovat odvhozi hovory

                /* V HLAVNICH LOOPECH HOVORŮ MUSIM MIT SWITCH NA RUZNE STATES */
                /* zacatek - prubeh - konec */

                /* vytvorit si vektor hovoru, diky kterym budu moct acceptovat a rozlisovat hovory */
                /* bude jeste temporátní call objekt do kterého příjde právě odchozí hovor, aby mohl být lehce zrušen */

                /** 
                 * TODO
                 * AuthInfo
                 * Proxy
                 * Call
                 * Core !
                 * Config a FactoryConfig
                 * 
                */
                vector<string> aux;
                split(messages[0], "!", aux);
                string correct_nick = ":" + bot.user_nick;

                if((aux[0] != correct_nick) || (messages[2] != bot.nick))
                {
                    continue;
                }

                string command = messages[3];
                if(command == ":end") /*end*/
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :Bot is ending, bye!\r\n";
                    bot.send_com(msg);

                    core.core_destroy();
                    //dealloc if needed
                    return 0;
                }
                else if(command == ":call") /*call sip:aaaa@aah.cz*/
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :call\r\n";
                    bot.send_com(msg);
                    /* call */
                    /* zmena state na odchozi hovor -> metoda invite */
                    /* vstup do loopu s hovorem */
                    /* NAT routing v configs! */
                    int ret = -1;
                    while (42)
                    {
                        /* funkce na call -> iterate */
                        ret = bot.check_messages_during_call();
                        if(ret == 1)
                        {
                            break;
                        }
                    }
                    
                }
                else if(command == ":register")
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :register\r\n";
                    bot.send_com(msg);
                    /* register */
                }
                else if(command == ":unregister")
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :unregister\r\n";
                    bot.send_com(msg);
                    /* unregister */
                }
                else if(command == ":accept")
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :accept\r\n";
                    bot.send_com(msg);
                    /* accept */
                    /* tady pokud existuje prichozi hovor se hovor prijme */
                    /* zmeni se state -> metoda accept */
                    /* probehne vstup do loopu, ktery obsluhuje hovor (podle me stejny loop jak v hovoru) */

                }
                else if(command == ":decline")
                {
                    string msg = "PRIVMSG " + bot.user_nick + " :decline\r\n";
                    bot.send_com(msg);
                    /* decline */
                }
                //dodelat cancel
                else{
                    string msg = "PRIVMSG " + bot.user_nick + " :Unknown command!\r\n";
                    bot.send_com(msg);
                }
                //TODO: adresar
                /* adresar things */
            }
            
            messages.clear();
        }
        /* TODO: sledovani incoming hovoru */
        /* sledovat state jestli prisel call -> jen snadný check ze state IncomingReceived (?) */
        /* tady se vypise "user is calling" */
    }

    return 0;
}
