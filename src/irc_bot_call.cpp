#include "irc_bot_call.h"

irc_bot_call::irc_bot_call()
{
    this->_call = nullptr;
}

irc_bot_call::~irc_bot_call()
{

}

void irc_bot_call::call_invite(LinphoneCore *lc ,string uri)
{
    this->_call = linphone_core_invite(lc, uri.c_str());
    cout << "STATE: " << linphone_call_get_state(this->_call) << endl;
    if(this->_call == nullptr)
    {
        cout << "Error calling " << uri << "!" << endl;
        return;
    }
    cout << "Calling " << uri << "!" << endl;
    linphone_call_ref(this->_call);
    //_calls.push_back(this->_call);
}

void irc_bot_call::call_terminate()
{
    if(this->_call != nullptr)
    {
        printf("Terminating the call...\n");
        linphone_call_terminate(this->_call);
	    /*at this stage we don't need the call object */
        linphone_call_unref(this->_call);
        //TODO: ne vždy poslední pop back
        //_calls.pop_back();
    }
}

/* ACCEPT and DENY*/