#include "irc_bot_core.h"

irc_bot_core::irc_bot_core()
{
    
}

irc_bot_core::~irc_bot_core()
{

}

string incomingCallMessage;
string incomingChatMessage;
LinphoneCall *incomingCall;
const LinphoneAddress *from;
const char *user;
const char *domain;


/* TODO: Dodelat callbacks na CONNECTED, OUTGOING a INCOMING */
static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	switch(cstate){
        case LinphoneCallIncomingReceived:
			printf("A call is here!!!!!\n");
            //linphone_call_accept(call);
            from = linphone_call_get_remote_address(call);
            user = linphone_address_get_username(from);
            domain = linphone_address_get_domain(from);
            incomingCallMessage = string(user) + "@" + string(domain) + string(" is calling!");
            incomingCall = call;
            cout << endl << incomingCallMessage << endl;
            break;
        case LinphoneCallStateOutgoingInit:
            printf("Started the call!\n");
            break;
		case LinphoneCallOutgoingRinging:
			printf("It is now ringing remotely !\n");
		    break;
		case LinphoneCallOutgoingEarlyMedia:
			printf("Receiving some early media\n");
		    break;
		case LinphoneCallConnected:
			printf("We are connected !\n");
		    break;
		case LinphoneCallStreamsRunning:
			printf("Media streams established !\n");
		    break;
        case LinphoneCallStatePausing:
            break;
        case LinphoneCallStatePaused:
            //linphone_call_resume(call);
            break;
		case LinphoneCallEnd:
			printf("Call is terminated.\n");
		    break;
		case LinphoneCallError:
			printf("Call failure !\n");
            if(incomingCall != nullptr)
            {
                incomingCall = nullptr;
                incomingCallMessage = string(user) + "@" + string(domain) + string(" is not calling anymore!");
            }
            break;
        case LinphoneCallReleased:
            printf("Call is released!\n");
		    break;
		default:
			printf("Unhandled notification %i\n",cstate);
	}
}

static void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message){
        /* TODO: deprecated */
		printf("New registration state %s for user id [%s] at proxy [%s]\n"
				,linphone_registration_state_to_string(cstate)
				,linphone_proxy_config_get_identity(cfg)
				,linphone_proxy_config_get_addr(cfg));
}


void text_received(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *message) {
    //printf(" Message [%s] received from [%s] \n", linphone_chat_message_get_text(message) , linphone_address_as_string(linphone_chat_room_get_peer_address(room)));
    string aux = string(linphone_address_as_string(linphone_chat_room_get_peer_address(room))) + ": " + string(linphone_chat_message_get_text(message));
    incomingChatMessage = aux;
}

void irc_bot_core::create_nat_policy()
{
    this->_nat = linphone_core_create_nat_policy(this->_core);
    linphone_core_enable_ipv6(this->_core, false);
    linphone_core_enable_keep_alive(this->_core, true);
    linphone_core_set_nat_policy(this->_core, this->_nat);
}

void irc_bot_core::enable_stun(string &address)
{
    linphone_nat_policy_enable_ice(this->_nat, true);
    linphone_nat_policy_enable_stun(this->_nat, true);
    linphone_nat_policy_set_stun_server(this->_nat, address.c_str());
    linphone_core_set_nat_policy(this->_core, this->_nat);   
}

void irc_bot_core::enable_turn(string &user, string &passw)
{
    linphone_nat_policy_enable_turn(this->_nat, true);
    this->_turn_cred=linphone_auth_info_new(user.c_str() ,NULL, passw.c_str() ,NULL,NULL,NULL); /*create authentication structure from identity*/
    linphone_core_add_auth_info(this->_core, this->_turn_cred);
    linphone_nat_policy_set_stun_server_username(this->_nat, user.c_str());
    linphone_core_set_nat_policy(this->_core, this->_nat); 
}

void irc_bot_core::core_create()
{
    this->_core = linphone_factory_create_core_3(this->_factory, "./conf/linphonerc", nullptr, nullptr);
    //this->_core = linphone_factory_create_core_3(this->_factory, nullptr, nullptr, nullptr);
    if(this->_core == nullptr)
    {
        cout << "An error creating core occured!" << endl;
    }
    int err;
    if((err = linphone_core_start(this->_core)) != 0){
        cout << err << endl;
        cout << "An error starting core occured!" << endl;
    }
    this->_cbs = linphone_factory_create_core_cbs(this->_factory);
    if(this->_cbs == nullptr)
    {
        cout << "An creating core callbacks occured!" << endl;
    }
    cout << "Core successfully created!" << endl;

    linphone_core_cbs_set_call_state_changed(this->_cbs, call_state_changed);
    linphone_core_cbs_set_registration_state_changed(this->_cbs, registration_state_changed);
    linphone_core_cbs_set_message_received(this->_cbs, text_received);
    linphone_core_add_callbacks(this->_core, this->_cbs);
    linphone_core_enable_echo_cancellation(this->_core, false);
    this->create_nat_policy();
    //create core cbs
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
