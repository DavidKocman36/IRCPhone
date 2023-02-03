#include "irc_bot_core.h"

irc_bot_core::irc_bot_core()
{
    
}

irc_bot_core::~irc_bot_core()
{

}

void irc_bot_core::core_create()
{
    this->_core = linphone_factory_create_core_3(this->_factory, "./conf/linphonerc", nullptr, nullptr);
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
