ROOT_DIR    = .
HEADERS_DIR = Headers
SOURCE_DIR  = Source
TOOLS_DIR   = Tools
PARSER_DIR  = $(TOOLS_DIR)/Parser
TESTER_DIR  = $(TOOLS_DIR)/Tester

LIB_SHEENBIDI = sheenbidi
LIB_PARSER    = sheenbidiparser
EXEC_TESTER   = sheenbiditester

AR = ar
CC = gcc
CXX = g++

ARFLAGS = -r
CFLAGS = -ansi -pedantic -Wall -I$(HEADERS_DIR)
CXXFLAGS = -std=c++11 -g -Wall
DEBUG_FLAGS = -DDEBUG -g -O0
RELEASE_FLAGS = -DNDEBUG -DSB_CONFIG_UNITY -Os

DEBUG = Debug
RELEASE = Release

DEBUG_SOURCES = $(SOURCE_DIR)/SBAlgorithm.c \
                $(SOURCE_DIR)/SBBase.c \
                $(SOURCE_DIR)/SBBidiChain.c \
                $(SOURCE_DIR)/SBBidiLink.c \
                $(SOURCE_DIR)/SBBracketQueue.c \
                $(SOURCE_DIR)/SBCharTypeLookup.c \
                $(SOURCE_DIR)/SBCodepointSequence.c \
                $(SOURCE_DIR)/SBIsolatingRun.c \
                $(SOURCE_DIR)/SBLevelRun.c \
                $(SOURCE_DIR)/SBLine.c \
                $(SOURCE_DIR)/SBLog.c \
                $(SOURCE_DIR)/SBMirrorLocator.c \
                $(SOURCE_DIR)/SBPairingLookup.c \
                $(SOURCE_DIR)/SBParagraph.c \
                $(SOURCE_DIR)/SBRunQueue.c \
                $(SOURCE_DIR)/SBStatusStack.c
RELEASE_SOURCES = $(SOURCE_DIR)/SheenBidi.c

DEBUG_OBJECTS   = $(DEBUG_SOURCES:$(SOURCE_DIR)/%.c=$(DEBUG)/%.o)
RELEASE_OBJECTS = $(RELEASE_SOURCES:$(SOURCE_DIR)/%.c=$(RELEASE)/%.o)

DEBUG_TARGET   = $(DEBUG)/lib$(LIB_SHEENBIDI).a
PARSER_TARGET  = $(DEBUG)/lib$(LIB_PARSER).a
TESTER_TARGET  = $(DEBUG)/$(EXEC_TESTER)
RELEASE_TARGET = $(RELEASE)/lib$(LIB_SHEENBIDI).a

$(DEBUG):
	mkdir $(DEBUG)

$(RELEASE):
	mkdir $(RELEASE)

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(AR) $(ARFLAGS) $(DEBUG_TARGET) $(DEBUG_OBJECTS)

$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	$(AR) $(ARFLAGS) $(RELEASE_TARGET) $(RELEASE_OBJECTS)

$(DEBUG)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(RELEASE)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

.PHONY: all debug parser tester release clean 

debug:   $(DEBUG) $(DEBUG_TARGET)
release: $(RELEASE) $(RELEASE_TARGET)
all:     release

clean: parser_clean tester_clean
	$(RM) $(DEBUG)/*.o $(DEBUG_TARGET)
	$(RM) $(RELEASE)/*.o $(RELEASE_TARGET)

include $(PARSER_DIR)/Makefile
include $(TESTER_DIR)/Makefile
