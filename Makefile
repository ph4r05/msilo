# $Id$
#
# example module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=msilo.so

CXX=/opt/rh/devtoolset-2/root/usr/bin/g++
LD=/opt/rh/devtoolset-2/root/usr/bin/g++

LIBS+= -lpthread -L/usr/local/lib
CXXFLAGS+= -std=c++11 -Wno-write-strings -fPIC $(DEFS)

include ../../Makefile.modules

LOGIC_DIR = cacheLogic
COMPILE.ccp = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) -c
LINK.o = $(CC) $(LDFLAGS) $(DEFS)

SRC_FILES = cacheLogic/logic.cpp \
            cacheLogic/ApiHelper.cpp \
            cacheLogic/MessageListElement.cpp \
            cacheLogic/MessageListElementWrapper.cpp \
            cacheLogic/MessageThreadElement.cpp \
            cacheLogic/MessageThreadManager.cpp \
            cacheLogic/MessageThreadMap.cpp \
            cacheLogic/MessageThreadSender.cpp \
            cacheLogic/MessageThreadWrapper.cpp \
            cacheLogic/SenderJobQueue.cpp

O_FILES = $(SRC_FILES:%.cpp=%.o)
LIBS+= $(O_FILES)

#%.o: %.c
#	$(COMPILE.c) $< $(OUTPUT_OPTION)

%.o: %.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)

cacheLogic:
	@$(MAKE) -C cacheLogic

msilo.so: msilo.o ms_msg_list.o msfuncs.o $(O_FILES)
#	$(CXX) $(CXXFLAGS) $(DEFS) -c $< -o $@
