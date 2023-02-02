CC = g++
CFLAGS  = -std=c++2a -g -I$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/include -L$(current_dir)/linphone-sdk/build/linphone-sdk/desktop/lib -llinphone

TARGET = src/irc_bot
TARGET_OUT = irc_bot
OBJECT_FILES = src/irc_bot_core.cpp

current_dir = $(shell pwd)

all: $(TARGET)

run: 
	./$(TARGET_OUT) irc.libera.chat Kocimistnost Koci password

$(TARGET): $(TARGET).cpp
	$(CC) $(TARGET).cpp $(OBJECT_FILES) $(CFLAGS) -o $(TARGET_OUT)

clean:
	$(RM) $(TARGET)