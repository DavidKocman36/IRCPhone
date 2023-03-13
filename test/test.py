#!/usr/bin/env python3
#
# This file is part of IRCPhone tests
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Author: David Kocman
#

# PREREQUISITES:
#
# sipp
# python3.9+
#
# NOTE: Connect only to servers which don't require registering an account
# Best one is irc.libera.chat 
#
# Launch: 
# python3 test.py {server} {channel} [-r sip_identity sip_passw]
#
# Test cases:
#   Connecting bot
#   Regsiter and unregister -> optional
#   Register with addr book -> also optional
#   Call
#   Call using addr book and enum
#   Accept call
#   Decline call
#   Call and hold and resume 
#   Basic message 
#   Call from already engaged call 
#   Multiple users calling 
#
# Address book:
#   insert contact or identity
#   update contact or identity
#   remove contact or identity
#

import sys
import socket
import os
import subprocess
import re
import select
import signal
from time import sleep

# Class containing color definitions for colored output
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# Number of scenearios
maxNumber = 0
# Number of overall tests
maxNumberTests = 0
# Number of current sceneario
actualNumber = 1
# Subprocess variables
process = None
sippProc = None
# Counter of passed/faile tests
testsPassed = 0
testsFailed = 0

def handler(signum, frame):
    poll = process.poll()
    if poll is None: # process is still alive
        os.killpg(os.getpgid(process.pid), signal.SIGINT)

    if sippProc is not None:
        poll = sippProc.poll()
        if poll is None: # process is still alive
            os.killpg(os.getpgid(sippProc.pid), signal.SIGTERM)
    
    sys.exit(0)

"""
The function for PING responding
"""
def pong_response(s, result):
    if result[0:4] == "PING":
        pong = "PONG " + result[4:] + "\r\n"
        s.send(pong.encode())

"""
The function for receiving messages from the other bot. 5 seconds timeout
"""
def recv_timeout(sock, bytes_to_read):
    sock.setblocking(0)
    ready = select.select([sock], [], [], 5.0)
    if ready[0]:
        return sock.recv(bytes_to_read).decode("utf-8")

    raise socket.timeout()

"""
Loop for message checking and printing whether test passed or not
"""
def test_loop(nick ,control_mess ,pass_mess, fail_mess, s):
    aux = 0
    global testsPassed
    global testsFailed
    while True:
        # Receive with 5 second timeout. If bot doesnt send message for 15 secounds then the test has failed.
        try:
            result = recv_timeout(s, 1024)
        except TimeoutError:
            aux += 1
            if aux == 3:
                print(bcolors.FAIL + "TEST: " + fail_mess + " FAILED: Timed out!" + bcolors.ENDC)
                testsFailed += 1
                break
            continue

        print(result)
        pong_response(s, result)
        if re.search(r'(.*)PRIVMSG ' + nick + ' :' + control_mess + '(.*)', result, re.MULTILINE):
            print(bcolors.OKGREEN + "TEST " + pass_mess + " PASSED!" + bcolors.ENDC)
            testsPassed += 1
            sleep(1)
            break

"""
Loop for checking two messages.
"""
def test_loop_or(nick ,control_mess_1, control_mess_2 ,pass_mess, fail_mess, s):
    aux = 0
    global testsPassed
    global testsFailed
    while True:
        try:
            result = recv_timeout(s, 1024)
        except TimeoutError:
            aux += 1
            if aux == 3:
                print(bcolors.FAIL + "TEST: " + fail_mess + " FAILED: Timed out!" + bcolors.ENDC)
                testsFailed += 1
                break
            continue

        print(result)
        pong_response(s, result)
        if re.search(r'(.*)PRIVMSG ' + nick + ' :' + control_mess_1 + '(.*)', result, re.MULTILINE) or re.search(r'(.*)PRIVMSG ' + nick + ' :' + control_mess_2 + '(.*)', result, re.MULTILINE):
            print(bcolors.OKGREEN + "TEST " + pass_mess + " PASSED!" + bcolors.ENDC)
            testsPassed += 1
            sleep(1)
            break

# Tests the bot's connection to the server
def test_bot_connect(server, channel, nick, s):
    global actualNumber
    global process
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Testing bot's connection---------" + bcolors.ENDC)
    # Start a subprocess -> in this case, the sip irc bot
    process = subprocess.Popen("cd .. && make && ./irc_bot " + server + " " + channel + " " + nick + " none", shell=True, preexec_fn=os.setsid)
    aux = 0
    while True:
        # Wait for one minute before failing. All of this because of makefile.
        try:
            result = recv_timeout(s, 1024)
        except TimeoutError:
            aux += 1
            if aux == 12:
                print(bcolors.FAIL + "TEST: Connecting bot FAILED: Timed out!" + bcolors.ENDC)
                print("Aborting tests - bot is not present!")
                os.killpg(os.getpgid(process.pid), signal.SIGINT)
                sys.exit(1)
            continue

        print(result)
        pong_response(s, result)
        if re.search(r'(.*)PRIVMSG ' + nick + ' :Hello(.*)', result, re.MULTILINE):
            print(bcolors.OKGREEN + "TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Connecting bot PASSED!" + bcolors.ENDC)
            actualNumber += 1
            sleep(3)
            break

"""
Function for testing address book CRUD operations - contacts
"""
def test_crud_contact(nick, s):
    global actualNumber
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Testing bot's CRUD contact operations---------" + bcolors.ENDC)
    data = "PRIVMSG SIPTest_b :-ic testName testUri\r\n"
    s.send(data.encode())
    
    test_loop_or(nick, "Record created successfully!", "Contact already exists!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Inserting contact", "Inserting contact", s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :-uc testName newTestUri\r\n"
    s.send(data.encode())

    test_loop(nick, "Record updated successfully!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Updating contact", "Updating contact", s)
    
    sleep(2)
    data = "PRIVMSG SIPTest_b :-rc testName\r\n"
    s.send(data.encode())

    test_loop(nick, "Record deleted successfully!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Deleting contact", "Deleting contact", s)
    actualNumber += 1
    sleep(2)

"""
Function for testing address book CRUD operations - registrar
"""
def test_crud_register(nick, s):
    global actualNumber
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Testing bot's CRUD registrar operations---------" + bcolors.ENDC)
    data = "PRIVMSG SIPTest_b :-ir testName testUri testPassw\r\n"
    s.send(data.encode())
    
    test_loop_or(nick, "Record created successfully!", "Identity already exists!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Inserting identity", "Inserting identity", s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :-ur testName newTestUri -\r\n"
    s.send(data.encode())

    test_loop(nick, "Record updated successfully!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Updating identity", "Updating identity", s)
    
    sleep(2)
    data = "PRIVMSG SIPTest_b :-rr testName\r\n"
    s.send(data.encode())

    test_loop(nick, "Record deleted successfully!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Deleting identity", "Deleting identity", s)
    actualNumber += 1
    sleep(2)
    
"""
Testing registration to proxy
"""
def test_register(identity, passw ,nick, s):
    global actualNumber
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Testing registration to proxy---------" + bcolors.ENDC)
    data = "PRIVMSG SIPTest_b :register " + identity + " " + passw + " \r\n"
    s.send(data.encode())
    
    test_loop(nick, "Succesfully registered as " + identity + "!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Registration", "Registration", s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :unregister \r\n"
    s.send(data.encode())
    
    test_loop(nick, "Succesfully unregistered!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Unregistration", "Unregistration", s)
    actualNumber += 1
    sleep(4)

"""
Testing registration to proxy with address book lookup
"""
def test_register_w_lookup(identity, passw ,nick, s):
    global actualNumber
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Testing registration to proxy with address book lookup---------" + bcolors.ENDC)
    data = "PRIVMSG SIPTest_b :-ir testName " + identity + " "+ passw +"\r\n"
    s.send(data.encode())
    sleep(3)
    data = "PRIVMSG SIPTest_b :register -a testName \r\n"
    s.send(data.encode())
    
    test_loop(nick, "Succesfully registered as " + identity + "!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Registration with lookup", "Registration with lookup", s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :unregister \r\n"
    s.send(data.encode())
    
    test_loop(nick, "Succesfully unregistered!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Unregistration with lookup", "Unregistration with lookup", s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :-rr testName\r\n"
    s.send(data.encode())
    actualNumber += 1
    sleep(2)

"""
Test basic call
"""
def test_call(nick, s):
    global actualNumber
    global sippProc

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call---------" + bcolors.ENDC)
    sippProc = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uas -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(3)

    data = "PRIVMSG SIPTest_b :call sip:127.0.0.1:5061\r\n"
    s.send(data.encode())

    test_loop(nick, "Calling sip:127.0.0.1:5061", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call", "Basic call" ,  s)
    test_loop(nick, "A call with sip:127.0.0.1:5061 is established!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call - Established", "Basic call - Established" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :hangup\r\n"
    s.send(data.encode())
    test_loop(nick, "A call from sip:127.0.0.1:5061 is terminated!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call - End", "Basic call - End" ,  s)

    actualNumber += 1

    sleep(2)

"""
Test basic call with address book and ENUM lookup
"""
def test_call_w_lookup(nick, s):
    global actualNumber
    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call with Contact and ENUM lookup---------" + bcolors.ENDC)
    
    data = "PRIVMSG SIPTest_b :-ic testName sip:test@none\r\n"
    s.send(data.encode())
    sleep(2)
    data = "PRIVMSG SIPTest_b :call -a testName\r\n"
    s.send(data.encode())

    test_loop(nick, "Calling sip:test@none", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call with contact lookup", "Basic call with contact lookup" ,  s)
    test_loop(nick, "Could not call sip:test@none! Wrong uri!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call - Not went through", "Basic call - Not went through" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :call -e 00437200101011\r\n"
    s.send(data.encode())

    test_loop(nick, "Calling sip:enum-test@sip.nemox.net", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call with ENUM lookup", "Basic call with ENUM lookup" ,  s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :hangup\r\n"
    s.send(data.encode())

    test_loop(nick, "A call from sip:enum-test@sip.nemox.net is terminated!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Basic call with ENUM lookup - End", "Basic call with ENUM lookup" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :-rc testName\r\n"
    s.send(data.encode())
    actualNumber += 1

"""
Accept an incoming call.
"""   
def test_incoming_call(nick, s):
    global actualNumber
    global sippProc

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call---------" + bcolors.ENDC)
    sippProc = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uac 127.0.0.1 -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(3)

    test_loop(nick, "(.*) is calling!. Type \"accept 1\" to accept this call!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call", "Incoming call" ,  s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :accept 1\r\n"
    s.send(data.encode())

    test_loop(nick, "A call with (.*) is established!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call - Established", "Incoming call - Established" ,  s)
    test_loop(nick, "A call from (.*) is terminated!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call - End", "Incoming call - End" ,  s)

    actualNumber += 1

    sleep(2)

"""
Decline an incoming call
"""
def test_decline_call(nick, s):
    global actualNumber
    global sippProc

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Decline call---------" + bcolors.ENDC)
    sippProc = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uac 127.0.1.1:5060 -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(2)
    data = "PRIVMSG SIPTest_b :decline\r\n"
    s.send(data.encode())

    test_loop(nick, "Declined all calls!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Decline call - Declined", "Decline call - Declined" ,  s)

    actualNumber += 1

    sleep(2)

"""
Make a call and put it on hold
"""
def test_hold_resume(nick, s):
    global actualNumber
    global sippProc

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Holding and resuming call---------" + bcolors.ENDC)
    sippProc = subprocess.Popen("cd sipp-3.3 && ./sipp -sf ../hold_res.xml -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(3)
    data = "PRIVMSG SIPTest_b :call sip:127.0.0.1:5061\r\n"
    s.send(data.encode())

    sleep(2)

    data = "PRIVMSG SIPTest_b :hold\r\n"
    s.send(data.encode())

    test_loop(nick, "Holding call sip:127.0.0.1:506", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Holding call", "Holding call" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :resume\r\n"
    s.send(data.encode())

    test_loop(nick, "Resuming call sip:127.0.0.1:5061", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Resuming call", "Resuming call" ,  s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :hangup\r\n"
    s.send(data.encode())

    actualNumber += 1

    sleep(2)

"""
Send and receive instant messages. Also with address book lookup
"""
def test_message(nick, s):
    global actualNumber
    global sippProc

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Receiving and sending instant message---------" + bcolors.ENDC)
    sippProc = subprocess.Popen("cd sipp-3.3 && ./sipp -sf ../mess.xml 127.0.0.1 -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(2)

    test_loop(nick, "sip:sipp@(.*): Hello!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Receiving message", "Receiving message" ,  s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :mess sip:127.0.0.1 Hello!\r\n"
    s.send(data.encode())

    test_loop(nick, "Sending a message", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Sending message", "Sending message" ,  s)

    sleep(1)
    data = "PRIVMSG SIPTest_b :-ic testName sip:127.0.0.1\r\n"
    s.send(data.encode())
    sleep(1)
    data = "PRIVMSG SIPTest_b :mess -a testName Hello!\r\n"
    s.send(data.encode())
    test_loop(nick, "Sending a message", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Sending message with lookup", "Sending message with lookup" ,  s)

    data = "PRIVMSG SIPTest_b :-rc testName\r\n"
    s.send(data.encode())
    actualNumber += 1

    sleep(2)

"""
Make a call from already established call.
"""
def test_call_in_a_call(nick, s):
    global actualNumber

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Outgoing call from another call---------" + bcolors.ENDC)
    sippProc1 = subprocess.Popen("cd sipp-3.3 && ./sipp -sf ../call_A.xml -i 127.0.0.1 -p 5061 -m 1 -bg", shell=True, preexec_fn=os.setsid)
    sippProc2 = subprocess.Popen("cd sipp-3.3 && ./sipp -sf ../call_B.xml -i 127.0.0.1 -p 5062 -m 1 -bg", shell=True, preexec_fn=os.setsid)

    sleep(3)
    data = "PRIVMSG SIPTest_b :call sip:127.0.0.1:5061\r\n"
    s.send(data.encode())

    sleep(2)

    data = "PRIVMSG SIPTest_b :call sip:127.0.0.1:5062\r\n"
    s.send(data.encode())

    test_loop(nick, "Calling sip:127.0.0.1:5062", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Second call", "Second call" ,  s)
    test_loop(nick, "A call with sip:127.0.0.1:5062 is established!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Second call - Established", "Second call - Established" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :hangup\r\n"
    s.send(data.encode())

    test_loop(nick, "A call from sip:127.0.0.1:5062 is terminated!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Second call - End", "Second call - End" ,  s)

    sleep(2)
    data = "PRIVMSG SIPTest_b :resume\r\n"
    s.send(data.encode())

    test_loop(nick, "Resuming call sip:127.0.0.1:5061", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: First call - Resuming", "First call - Resuming" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :hangup\r\n"
    s.send(data.encode())

    test_loop(nick, "A call from sip:127.0.0.1:5061 is terminated!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: First call - End", "First call - End" ,  s)

    actualNumber += 1

    sleep(2)

    poll = sippProc1.poll()
    if poll is None: # sippProc1 is still alive
        os.killpg(os.getpgid(sippProc1.pid), signal.SIGTERM)

    poll = sippProc2.poll()
    if poll is None: # sippProc2 is still alive
        os.killpg(os.getpgid(sippProc2.pid), signal.SIGTERM)

"""
Get multiple incoming calls and accept the second one.
"""
def test_multiple_inc_calls(nick, s):
    global actualNumber

    print(bcolors.HEADER + "---------TEST [" + str(actualNumber) +"/" + str(maxNumber) + "]: Multiple incoming calls---------" + bcolors.ENDC)
    sippProc1 = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uac 127.0.0.1 -i 127.0.0.1 -p 5061 -m 1 -bg", shell=True, preexec_fn=os.setsid)
    test_loop(nick, "(.*) is calling!. Type \"accept 1\" to accept this call!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call 1", "Incoming call 1" ,  s)
    sippProc2 = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uac 127.0.0.1 -i 127.0.0.1 -p 5062 -m 1 -bg", shell=True, preexec_fn=os.setsid)
    test_loop(nick, "(.*) is calling!. Type \"accept 2\" to accept this call!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call 2", "Incoming call 2" ,  s)
    sippProc3 = subprocess.Popen("cd sipp-3.3 && ./sipp -sn uac 127.0.0.1 -i 127.0.0.1 -p 5063 -m 1 -bg", shell=True, preexec_fn=os.setsid)
    test_loop(nick, "(.*) is calling!. Type \"accept 3\" to accept this call!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Incoming call 3", "Incoming call 3" ,  s)

    sleep(2)

    data = "PRIVMSG SIPTest_b :accept 2\r\n"
    s.send(data.encode())

    test_loop(nick, "A call with (.*)127.0.0.1:5062(.*) is established!", "[" + str(actualNumber) +"/" + str(maxNumber) + "]: Second incoming call - Established", "Second incoming call - Established" ,  s)
    sleep(2)

    actualNumber += 1

    sleep(2)

    poll = sippProc1.poll()
    if poll is None: # sippProc1 is still alive
        os.killpg(os.getpgid(sippProc1.pid), signal.SIGTERM)

    poll = sippProc2.poll()
    if poll is None: # sippProc2 is still alive
        os.killpg(os.getpgid(sippProc2.pid), signal.SIGTERM)

    poll = sippProc3.poll()
    if poll is None: # sippProc3 is still alive
        os.killpg(os.getpgid(sippProc3.pid), signal.SIGTERM)

"""
Main
"""
def main_func():

    n = len(sys.argv)
    if n < 3:
        print("ERROR: Wrong arguments! Usage: python3 test.py {server} {channel} [-r sip_identity sip_passw]")
        return

    server = sys.argv[1]
    channel = sys.argv[2]
    register = False
    identity, passw = None, None
    port = 6666
    nick = "SIPTest"
    global maxNumber
    global maxNumberTests
    maxNumber = 11
    maxNumberTests = 31
    # The registration tests are optional. A real proxy identity must be provided
    if n > 3 and sys.argv[3] == "-r":
        if n < 6:
            print("ERROR: Wrong arguments! Usage: python3 test.py {local_ip} {server} {channel} [-r sip_identity sip_passw]")
            return

        identity = sys.argv[4]
        passw = sys.argv[5]
        register = True
        maxNumber += 2 
        maxNumberTests += 4
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #Now connect to server
    print("Connecting test bot...")
    s.connect((server, port))

    passw_data = "PASSW none\r\n"
    s.send(passw_data.encode())

    nick_data = "NICK " + nick + "\r\n"
    s.send(nick_data.encode())

    user_data = "USER " + nick + " 0 * :SIP testing bot\r\n" 
    s.send(user_data.encode())

    channel_data = "JOIN #" + channel + "\r\n"
    s.send(channel_data.encode())

    while True:
        result = s.recv(1024).decode("utf-8")

        pong_response(s, result)
        # https://stackoverflow.com/questions/54271174/irc-messages-randomly-not-sending-from-bot
        if re.search(r'(.*)End of /NAMES list.(.*)', result, re.MULTILINE):
            print("Connected!")
            break

    # Test cases
    signal.signal(signal.SIGINT, handler)
    test_bot_connect(server, channel, nick, s)
    test_crud_contact(nick, s)
    test_crud_register(nick, s)
    test_call(nick, s)
    test_call_w_lookup(nick, s)
    test_incoming_call(nick, s)
    test_decline_call(nick, s)
    test_hold_resume(nick, s)
    test_message(nick, s)
    test_call_in_a_call(nick, s)
    test_multiple_inc_calls(nick, s)

    if(register):
        test_register(identity, passw, nick, s)
        test_register_w_lookup(identity, passw, nick, s)

    print(bcolors.HEADER + "\n-------------RESULTS-------------" + bcolors.ENDC)
    print("\nPassed: " + str(testsPassed))
    print("Failed: " + str(testsFailed))
    print("Out of possible " + str(maxNumberTests) + " tests from " + str(maxNumber) + " test scenearios \n")
    print(bcolors.HEADER + "---------------------------------" + bcolors.ENDC)
    
    # End the bot
    data = "PRIVMSG SIPTest_b :end\r\n"
    s.send(data.encode())

    sleep(1)

    poll = process.poll()
    if poll is None: # process is still alive
        os.killpg(os.getpgid(process.pid), signal.SIGINT)
    
    sleep(2)
    print("Test bot is disconnecting!")
    quit_data = "QUIT\r\n"
    s.send(quit_data.encode())
    
    

if __name__ == "__main__":
    main_func()
