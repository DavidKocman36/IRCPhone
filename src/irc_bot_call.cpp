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
 */

#include "irc_bot_call.h"

irc_bot_call::irc_bot_call()
{
    this->_call = nullptr;
}

irc_bot_call::~irc_bot_call()
{
}

int irc_bot_call::call_invite(LinphoneCore *lc ,string uri)
{
    //Initiate the call
    this->_call = linphone_core_invite(lc, uri.c_str());
    if(this->_call == nullptr)
    {
        return 1;
    }
    //Get a reference of the call if we want to work with if later
    linphone_call_ref(this->_call);
    return 0;
}

void irc_bot_call::call_terminate()
{
    if(this->_call != nullptr)
    {
        linphone_call_terminate(this->_call);
    }
}