# IBT

This is the bachelors thesis of SIP over IRC. Made by David Kocman at BUT FIT.

First you should get liblinphone-sdk from (link). Simply clone it and follow instructions on how to build it. My reccomended cmake commands are:

cmake -G "Ninja" -DLINPHONESDK_PLATFORM=Desktop -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_RELATIVE_PREFIX=YES -DENABLE_VIDEO=NO ..

cmake --build . --parallel 9

Later on it is HIGHLY reccomneded, even mandatory, to add an include path to g++ build. It is in the makefile but if you are not using mine, this tip might be helpful.

install liblinphone10 if the file is not found!

mkdir if start core throws an exception!

Have to be started with root privileges!

sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/identity_grammar /usr/share/belr/grammars/
sudo cp linphone-sdk/build/linphone-sdk/desktop/share/belr/grammars/cpim_grammar /usr/share/belr/grammars/

find /usr/share/belr/grammars/identity_grammar

if you have no sqlite3 on your pc, just run "sudo apt-get install libsqlite3-dev"

Also have all dependencies used by liblinphone!!!


