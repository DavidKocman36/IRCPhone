# IBT

This is the bachelors thesis of SIP over IRC. Made by David Kocman at BUT FIT.

First you should get liblinphone-sdk from (link). Simply clone it and follow instructions on how to build it. My reccomended cmake commands are:

cmake -G "Ninja" -DLINPHONESDK_PLATFORM=Desktop -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_RELATIVE_PREFIX=YES -DENABLE_VIDEO=NO ..

cmake --build . --parallel 9

Later on it is HIGHLY reccomneded, even mandatory, to add an include path to g++ build. It is in the makefile but if you are not using mine, this tip might be helpful.


