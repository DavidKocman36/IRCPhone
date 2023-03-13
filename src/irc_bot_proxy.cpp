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

#include "irc_bot_proxy.h"

irc_bot_proxy::irc_bot_proxy()
{
    /* Init all attributes as null just in case */
    this->proxy_cfg = nullptr;
    this->info = nullptr;
    this->from = nullptr;
    this->passw = nullptr;
    this->server_addr = nullptr;
    this->user = nullptr;
}

irc_bot_proxy::~irc_bot_proxy()
{

}

void irc_bot_proxy::set_credentials(string &user, string &passw)
{
    this->user = user.c_str();
    this->passw = passw.c_str();
}

int irc_bot_proxy::bot_register(LinphoneCore *lc)
{
    /*create proxy config*/
    this->proxy_cfg = linphone_core_create_proxy_config(lc);
    linphone_core_clear_proxy_config(lc);
    /*parse identity*/
    this->from = linphone_address_new(this->user);
    if (this->from == nullptr){
        return 1;
    }
    if (this->passw != nullptr){
        this->info=linphone_auth_info_new(linphone_address_get_username(this->from),NULL,this->passw,NULL,NULL,NULL); /*create authentication structure from identity*/
        linphone_core_add_auth_info(lc,this->info); /*add authentication info to LinphoneCore*/
    }
    // configure proxy entries
    linphone_proxy_config_set_identity_address(this->proxy_cfg, this->from); /*set identity with user name and domain*/
    this->server_addr = linphone_address_get_domain(this->from); /*extract domain address from identity*/
    linphone_proxy_config_set_server_addr(this->proxy_cfg, this->server_addr); /* we assume domain = proxy server address*/
    linphone_proxy_config_enable_register(this->proxy_cfg, true); /*activate registration for this proxy config*/
    linphone_address_unref(this->from); /*release resource*/
    linphone_core_add_proxy_config(lc, this->proxy_cfg); /*add proxy config to linphone core*/
    linphone_core_set_default_proxy_config(lc, this->proxy_cfg);
    return 0;
}

void irc_bot_proxy::bot_unregister(LinphoneCore *lc)
{   
    this->proxy_cfg = linphone_core_get_default_proxy_config(lc);
	linphone_proxy_config_edit(this->proxy_cfg); /*start editing proxy configuration*/
	linphone_proxy_config_enable_register(this->proxy_cfg, false); /*de-activate registration for this proxy config*/
	linphone_proxy_config_done(this->proxy_cfg); /*initiate REGISTER with expire = 0*/
    linphone_core_clear_proxy_config(lc);
}