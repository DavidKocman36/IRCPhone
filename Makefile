CC = g++
CFLAGS  = -std=c++2a -g -w -I$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/include -L$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/lib -llinphone -lbctoolbox

TARGET = src/irc_bot_main
TARGET_OUT = irc_bot
OBJECT_FILES = src/my_string.cpp src/addr_book.cpp src/irc_bot.cpp src/irc_bot_core.cpp src/irc_bot_call.cpp src/irc_bot_proxy.cpp src/irc_bot_message.cpp 

current_dir = $(shell pwd)

all: $(TARGET)

run: 
	./$(TARGET_OUT) irc.libera.chat Kocimistnost Koci password

$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp -lsqlite3 $(OBJECT_FILES) $(CFLAGS) -o $(TARGET_OUT)

clean:
	$(RM) $(TARGET)