AUTOMAKE_OPTIONS = foreign subdir-objects
bin_PROGRAMS = irc_bot
irc_bot_SOURCES = src/irc_bot_main.cpp src/my_string.cpp src/addr_book.cpp src/irc_bot.cpp src/irc_bot_core.cpp src/irc_bot_call.cpp src/irc_bot_proxy.cpp src/irc_bot_message.cpp 

# Link against our uninstalled copy of libhelloworld.
LDADD = $(top_builddir)/linphone-sdk/build/linphone-sdk/desktop/lib/liblinphone.so $(top_builddir)/linphone-sdk/build/linphone-sdk/desktop/lib/libbctoolbox.so -lsqlite3 -lresolv

# Make sure we can find the uninstalled header file.
AM_CPPFLAGS = -std=c++2a -g -w -I$(top_srcdir)/linphone-sdk/build/linphone-sdk/desktop/include
