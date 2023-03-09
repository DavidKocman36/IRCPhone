#
# This file is part of IRCPhone
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

CC = g++
CFLAGS  = -std=c++2a -g -w -I$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/include -L$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/lib -llinphone -lbctoolbox -lresolv

TARGET = src/irc_bot_main
TARGET_OUT = irc_bot
OBJECT_FILES = src/my_string.cpp src/addr_book.cpp src/irc_bot.cpp src/irc_bot_core.cpp src/irc_bot_call.cpp src/irc_bot_proxy.cpp src/irc_bot_message.cpp 

current_dir = $(shell pwd)

all: $(TARGET)

run: 
	./$(TARGET_OUT) irc.libera.chat Kocimistnost Koci password

$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp -lsqlite3 $(OBJECT_FILES) $(CFLAGS) -o $(TARGET_OUT)
