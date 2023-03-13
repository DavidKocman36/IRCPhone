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

#ifndef IRC_BOT_CORE_H
#define IRC_BOT_CORE_H

#include <linphone/linphonecore.h>
#include <algorithm>
#include <vector>
#include "irc_bot_call.h"

using namespace std;

struct IncCall
{
    /*Reference to the incoming call*/
    LinphoneCall *call;
    /*Whether still ringing or not*/
    int status;
};

/* Message used to notify user when someone is calling */
extern string incomingCallMessage;
/* Message used to notify user when a message is received */
extern string incomingChatMessage;
/* Message used to notify user when remote hung up the call or call is terminated */
extern string remoteHungUp;
/* The incoming call */
extern LinphoneCall *incomingCall;

/* Vector containing all the active calls */
extern vector<irc_bot_call>callsVector;
/* Vector containing all the incoming calls */
extern vector<IncCall>incomingCallsVector;

class irc_bot_core
{
public:
    /* The factory object */
    LinphoneFactory *_factory;
    /* The core object itself */
    LinphoneCore *_core;
    /* Core callbacks object */
    LinphoneCoreCbs *_cbs;
    /* Nat policy object */
    LinphoneNatPolicy *_nat;
    /* TURN server credentials */
    LinphoneAuthInfo *_turn_cred;

    /* Constructor and destructor */
    irc_bot_core();
    ~irc_bot_core();

    /**
     * Function that creates the core, adds callbacks and creates empty NAT policy
     * 
     * @returns 0 on success, 1 on failure
    */
    int core_create();

    /**
     * Function that destroys the core.
    */
    void core_destroy();

    /**
     * Function for doing the linphones core background work.
    */
    void iterate();

    /**
     * Function for creating the NAT policy object
    */
    void create_nat_policy();

    /**
     * Function that enables ICE and STUN.
     * 
     * @param address The address of the STUN/TURN server
    */
    void enable_stun(string &address);

    /**
     * Function that enables TURN. Also authenticates the TURN user.
     * 
     * @param user The TURN username
     * @param passw The TURN password
    */
    void enable_turn(string &user, string &passw);

    /**
     * Function that disables STUN/TURN and ICE
    */
    void disable_nat();
};

#endif