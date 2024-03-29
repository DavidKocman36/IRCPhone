# IRCPhone automated application tests

This script is used for automated testing of IRCPhone. Specific test cases are described in the source file itself.

# Installation 
For automated tests a tool called SIPp is used for SIP traffic generation. Python is used for the script.
Make sure to have python 3.9+ installed.

[Installation and prerequisites](https://sipp.readthedocs.io/en/latest/installation.html) of SIPp.

1. [Download](https://sourceforge.net/projects/sipp/files/) sipp.tar.gz
2. run `tar -xvzf sipp-xxx.tar.gz` to unpack
3. make pcapplay_ossl

The recommended version is sipp-3.3. If other version is used, the folder's name must be sipp-3.3.
Structure of a folder should look like this:
```
├── sipp-3.3
├── call_A.xml
├── call_B.xml
├── hold_res.xml
├── mess.xml
└── test.py

```
# Usage

It is MANDATORY to run the tests AFTER building the program. The binary shall be already built and present.

Launch the test script by:
`python3 test.py {server} {channel} [-r {identity} {passw}] [-s {stun_turn_server} {turn_username} {turn_password}]`
The -r argument is optional and is used for proxy registration tests. Real proxy id must be provided for successfull tests.
The -s argument is also optional and is used to test NAT traversal settings. Again, real STUN/TURN server and TURN credential must be provided.
