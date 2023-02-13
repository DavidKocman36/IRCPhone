#include "irc_bot_core.h"

irc_bot_core::irc_bot_core()
{
    
}

irc_bot_core::~irc_bot_core()
{

}

int aux = 0;

/* TODO: Dodelat callbacks na CONNECTED, OUTGOING a INCOMING */
static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg){
	switch(cstate){
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
            //linphone_call_pause(call);
		    break;

		case LinphoneCallStreamsRunning:
            //linphone_call_resume(call);
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
            break;
        case LinphoneCallReleased:
            printf("Call is released!\n");
		break;
		default:
			printf("Unhandled notification %i\n",cstate);
	}
}

static void registration_state_changed(struct _LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message){
		printf("New registration state %s for user id [%s] at proxy [%s]\n"
				,linphone_registration_state_to_string(cstate)
				,linphone_proxy_config_get_identity(cfg)
				,linphone_proxy_config_get_addr(cfg));
}


static void dtmfHandler(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
    //PhpLinphoneCall *theCall = PhpLinphoneCall::selectCall(call);
    //if (theCall != nullptr) {
        //#ifdef DEBUG
        //Php::out<<"[DEBUG - MAIN THREAD] Settings dtmf in the call object"<<std::endl;
        //#endif
    //theCall->setDtmf(dtmf);
    //} else {
      //  #ifdef DEBUG
        //Php::out<<"[DEBUG - MAIN THREAD] Call not found!"<<std::endl;
        //#endif
    //}
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
    linphone_core_cbs_set_dtmf_received(this->_cbs, dtmfHandler);
    linphone_core_cbs_set_registration_state_changed(this->_cbs, registration_state_changed);
    linphone_core_add_callbacks(this->_core, this->_cbs);
    linphone_core_enable_echo_cancellation(this->_core, false);
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
    //cout << "ITERUJU" << endl;
}
