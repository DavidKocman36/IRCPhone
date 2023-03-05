# IRCPhone

This is the bachelors thesis of SIP integration into IRC client. Made by David Kocman at BUT FIT.

The IRCPhone is a SIP User agent based on open source SIP library liblinphone using IRC as its GUI, launched from command prompt. It is an IRC bot that lingers in a channel and listens to users commands.

Features:
- Voice calls
- Register at proxy
- Instant messages
- Calling through ENUM lookup
- Address book for storing contacts and proxy identities
- Easy to use commands
- All basic SIP features

# Installation and prerequisites

Make sure to have ALL dependencies required by liblinphone.
First you should get liblinphone-sdk from [here](https://www.linphone.org/technical-corner/liblinphone). Simply clone it and follow [instructions](https://gitlab.linphone.org/BC/public/linphone-sdk) on how to build it. My recommended cmake commands are:

`cmake -G "Ninja" -DLINPHONESDK_PLATFORM=Desktop -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_RELATIVE_PREFIX=YES -DENABLE_VIDEO=NO ..`

`cmake --build . --parallel 9`

NOTE: It is HIGHLY recommended, even mandatory, to add an include and lib paths to g++ build. It is in Makefile but if you are not using mine, this tip might be helpful.

Next, it is very important to make sure identity and cpim grammars are in /usr/share/belr/grammars folder.

Run these commands: `find /usr/share/belr/grammars/identity_grammar` and `find /usr/share/belr/grammars/cpim_grammar`
If the grammars are not found then just copy these grammars from linphone-sdk.

`sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/identity_grammar /usr/share/belr/grammars/`
`sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/cpim_grammar /usr/share/belr/grammars/`

Last but not least, make sure you have sqlite3 on your system.
If not just run `sudo apt-get install libsqlite3-dev`.

After building of linphone-sdk is successfull and you have all dependencies, run "make" command to compile IRCPhone. 
Launch with
`./irc_bot {server} {channel} {user} {password}` 

If after compilation and launch you encounter a message that liblinphone.so.10 is not found then `sudo apt-get install liblinphone10` shall do the trick.

If core on startup fails to open the Linphone.db database just create the folder mentioned by the error message. The database will be stored in this folder then. 

Structure of a folder should look like this:
```
├── db
├── linphone-sdk
├── sounds
├── src
├── test
│ 
├── irc_bot
└── Makefile
```

# Usage

See manual.txt.

# Tests

For automated tests a tool called SIPp is used for SIP traffic generation.

[Installation and prerequisites](https://sipp.readthedocs.io/en/latest/installation.html)

1. [Download](https://sourceforge.net/projects/sipp/files/) sipp.tar.gz
2. run `tar -xvzf sipp-xxx.tar.gz` to unpack
3. make

# Known issues

