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

#include "irc_bot_core.h"

irc_bot_core::irc_bot_core()
{
    
}

irc_bot_core::~irc_bot_core()
{

}

/* Global variables explained in header file */
/* The reason they are global is because the callback is not a member function of irc_bot_core */
string incomingCallMessage;
string incomingChatMessage;
string remoteHungUp;
LinphoneCall *incomingCall;
const LinphoneAddress *from;
const char *user;
const char *domain;
vector<irc_bot_call>callsVector;
vector<IncCall>incomingCallsVector;
string remote;

IncCall incCall;
IncCall auxCall;

/* The linphone call state changed callback. Some of the states are off sometimes. */
static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	switch(cstate){
        case LinphoneCallStateIncomingReceived:
            /* Received an incoming call */
            from = linphone_call_get_remote_address(call);
            user = linphone_address_get_username(from);
            domain = linphone_address_get_domain(from);
            incomingCallMessage = string(user) + "@" + string(domain) + string(" is calling!");
            incomingCall = call;
            /* Push this call into the incoming calls vector */
            incCall.call = call;
            incCall.status = 1;
            incomingCallsVector.push_back(incCall);
            break;
        case LinphoneCallStateOutgoingInit:
            break;
		case LinphoneCallStateOutgoingRinging:
		    break;
		case LinphoneCallStateOutgoingEarlyMedia:
		    break;
		case LinphoneCallStateConnected:
		    break;
		case LinphoneCallStateStreamsRunning:
		    break;
        case LinphoneCallStatePausing:
            break;
        case LinphoneCallStatePaused:
            break;
		case LinphoneCallStateEnd:
		    break;
        case LinphoneCallStateReferred:
            callsVector.pop_back();
            break;
		case LinphoneCallStateError:
            /* When the call ended */
            /* When the incoming call rings no more change its status to 0 (inactive) */
            for(int i = 0; i < incomingCallsVector.size(); i++)
            {
                auxCall = incomingCallsVector.at(i);
                if(auxCall.status == 1 && auxCall.call == call)
                {
                    auxCall.status = 0;
                    incomingCallsVector.at(i) = auxCall;
                }
            }
            /* When remote or user hangs up a call then delete this call from the calls vector */
            for(int i = 0; i < callsVector.size(); i++)
            {
                if(callsVector.at(i)._call == call)
                {
                    callsVector.erase(callsVector.begin() + i);
                }
            }
            //remote hung up message
            from = linphone_call_get_remote_address(call);
            remoteHungUp =
             "A call from " + string(linphone_address_as_string(from)) + " is terminated!";
            break;
        case LinphoneCallStateReleased:
		    break;
		default:
            break;
	}
}

/* Registration state changed callback */
static void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message){
		printf("New registration state %s for user id [%s] at proxy [%s]\n"
				,linphone_registration_state_to_string(cstate)
				,linphone_proxy_config_get_identity(cfg)
				,linphone_proxy_config_get_addr(cfg));
}

/* The text received callback. Gets the content of the message. */
void text_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
    string aux = string(linphone_address_as_string(linphone_chat_room_get_peer_address(room))) + ": " + string(linphone_chat_message_get_text(message));
    incomingChatMessage = aux;
}

void irc_bot_core::create_nat_policy()
{
    this->_nat = linphone_core_create_nat_policy(this->_core);
    linphone_core_enable_ipv6(this->_core, true);
    linphone_core_enable_keep_alive(this->_core, true);
    linphone_core_set_nat_policy(this->_core, this->_nat);
}

int irc_bot_core::enable_stun(string &address)
{
    linphone_nat_policy_enable_ice(this->_nat, true);
    linphone_nat_policy_enable_stun(this->_nat, true);
    linphone_nat_policy_set_stun_server(this->_nat, address.c_str());
    const struct addrinfo *stun_s = linphone_nat_policy_get_stun_server_addrinfo(this->_nat);
    if(stun_s == nullptr)
    {
        linphone_nat_policy_enable_ice(this->_nat, false);
        linphone_nat_policy_enable_stun(this->_nat, false);
        return 1;
    }
    linphone_core_set_nat_policy(this->_core, this->_nat);  
    return 0; 
}

void irc_bot_core::enable_turn(string &user, string &passw)
{
    linphone_nat_policy_enable_turn(this->_nat, true);
    this->_turn_cred=linphone_auth_info_new(user.c_str() ,NULL, passw.c_str() ,NULL,NULL,NULL); /*create authentication structure from identity*/
    linphone_core_add_auth_info(this->_core, this->_turn_cred);
    linphone_nat_policy_set_stun_server_username(this->_nat, user.c_str());
    linphone_core_set_nat_policy(this->_core, this->_nat); 
}

int irc_bot_core::core_create()
{
    /* Create core. */
    this->_core = linphone_factory_create_core_3(this->_factory, nullptr, nullptr, nullptr);
    if(this->_core == nullptr)
    {
        cout << "An error creating core occured!" << endl;
        return 1;
    }
    int err;
    /* Start the core. A 'linphone' folder must be present in $HOME/.local/share/ folder. 
    This folder then contains the linphone core database.*/
    if((err = linphone_core_start(this->_core)) != 0){
        cout << "An error starting core occured! Error number: " << err << endl;
        return 1;
    }
    this->_cbs = linphone_factory_create_core_cbs(this->_factory);
    if(this->_cbs == nullptr)
    {
        cout << "An error creating core callbacks occured!" << endl;
        return 1;
    }
    cout << "Core successfully created!" << endl;

    /* Add callbacks */
    linphone_core_cbs_set_call_state_changed(this->_cbs, call_state_changed);
    linphone_core_cbs_set_registration_state_changed(this->_cbs, registration_state_changed);
    linphone_core_cbs_set_message_received(this->_cbs, text_received);
    linphone_core_add_callbacks(this->_core, this->_cbs);
    linphone_core_enable_echo_cancellation(this->_core, false);
    this->create_nat_policy();

    /* Set some linphone options */
    linphone_core_enable_adaptive_rate_control(this->_core, true);
    linphone_core_enable_mic(this->_core, true);
    linphone_core_set_audio_port_range(this->_core, 7077, 8000);
    linphone_core_set_play_file(this->_core, "./sounds/toy-mono.wav");
    linphone_core_set_ring(this->_core, "./sounds/ringback.wav");
    linphone_core_set_ringback(this->_core, "./sounds/ringback.wav");
    linphone_core_enable_video_capture(this->_core, false);
    linphone_core_enable_video_display(this->_core, false);

    return 0;
}

void irc_bot_core::core_destroy()
{
    if(this->_core != nullptr)
    {
        linphone_core_stop(this->_core);
        cout << "Core succesfully stopped!" << endl;
        linphone_core_unref(this->_core);
        cout << "Core succesfully destroyed!" << endl;
    }
}

void irc_bot_core::iterate()
{
    linphone_core_iterate(this->_core);
}

void irc_bot_core::disable_nat()
{   
    /* Clear STUN/TURN and all protocols */
    linphone_nat_policy_clear(this->_nat);
    /* Set these options again, just in case */
    linphone_core_enable_ipv6(this->_core, false);
    linphone_core_enable_keep_alive(this->_core, true);

    linphone_core_set_nat_policy(this->_core, this->_nat);
}