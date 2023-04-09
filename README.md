# IRCPhone

This is the bachelors thesis of SIP integration into IRC client. Made by David Kocman at BUT FIT.

The IRCPhone is a SIP User agent based on open source SIP library liblinphone using IRC client as its GUI, launched from a command prompt. It is an IRC bot that lingers in a channel and listens to user's commands, which control the bot.

Features:
- Voice calls
- Register at proxy
- Instant messages
- Calling through ENUM lookup
- Address book for storing contacts and proxy identities
- Easy to use commands
- All basic SIP features



# Installation and prerequisites

TL:DR prerequisites to download for this program:
1. build-essentials
2. liblinphone10
3. libsqlite3-dev
4. all liblinphone [dependencies](https://gitlab.linphone.org/BC/public/linphone-sdk)

You may use the provided bash script to retrieve dependencies 2 and 3. Just run `chmod u+x ./packages.sh` if the script is not executable, then launch it with `sudo ./packages.sh`, but the library must be built first.

## Liblinphone build

The liblinphone is included as a git submodule within this project. If the submodule for some reaseon was not downloaded, follow the instructions below. To build the library, follow the steps 2 and 3.

You should get liblinphone-sdk from [here](https://www.linphone.org/technical-corner/liblinphone). Simply clone it and follow [instructions](https://gitlab.linphone.org/BC/public/linphone-sdk) on how to build it. Basically it is:

1. Clone the repository `git clone https://gitlab.linphone.org/BC/public/linphone-sdk.git --recursive`
2. Go to linphone-sdk folder and make a 'build' folder (`mkdir build`)
3. Go to the 'build' folder and run cmake commands mentioned below

My used cmake commands are:

`cmake -G "Ninja" -DLINPHONESDK_PLATFORM=Desktop -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_RELATIVE_PREFIX=YES -DENABLE_VIDEO=NO ..`

After this command above is completed:

`cmake --build . --parallel 9`

## Building the program

After building the library, run `sudo ./packages.sh` to retrieve all dependencies this project requires.
To make it manually, you may use:

`sudo apt-get install liblinphone10`

`sudo apt-get install libsqlite3-dev`.

It may be needed to make the script executable first.

Next, it is very important to make sure identity, cpim and vcard grammars are in `/usr/share/belr/grammars` folder (used for instant messages).

Run these commands: `find /usr/share/belr/grammars/identity_grammar`, `find /usr/share/belr/grammars/cpim_grammar` and `find /usr/share/belr/grammars/vcard_grammar`
If the grammars are not found then just copy these grammars from linphone-sdk and create the folders if they are not yet created.

`sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/identity_grammar /usr/share/belr/grammars/`

`sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/cpim_grammar /usr/share/belr/grammars/`

`sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/vcard_grammar /usr/share/belr/grammars/`

It is also crucial to check, whether the `share` folder in `$HOME/.local/share/linphone` is present. If not, the program exits at the startup because linphone database could not be stored anywhere.

Last step is to build the executable.
For this, the `build.sh` script is used. This script runs autotools and is launched as follows:

`./build.sh`

Make sure to have files `configure.ac`, `makefile.am` and `src/makefile.am` present.
Just like the `packages.sh`, this script might also be needed to be made executable.

`chmod u+x ./build.sh`

After all of the steps above are completed, you may launch the program with:
`./irc_bot {server} {channel} {user} {password}` 
For the meaning of each argument, please refer to `manual.txt`.

Structure of the root folder should look like this:
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
... and also other important scripts and files.

# Usage

See `manual.txt`.

# Tests

For automated tests a tool called SIPp is used for SIP traffic generation.

More in test/README.md
