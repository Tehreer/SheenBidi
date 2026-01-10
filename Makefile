ROOT_DIR    = .
HEADERS_DIR = Headers
SOURCE_DIR  = Source
TESTS_DIR   = Tests
TOOLS_DIR   = Tools
PARSER_DIR  = $(TOOLS_DIR)/Parser

LIB_SHEENBIDI = SheenBidi
LIB_PARSER    = Parser
EXEC_TESTS    = Tests

ifndef CC
	CC = gcc
endif
ifndef CXX
	CXX = g++
endif

AR      = ar
ARFLAGS = -r
RM      = rm -rf

CFLAGS   = -ansi -pedantic -Wall -I$(HEADERS_DIR) -I$(SOURCE_DIR)
CXXFLAGS = -std=c++14 -g -Wall

DEBUG_FLAGS   = -DDEBUG -g -O0
RELEASE_FLAGS = -DNDEBUG -DSB_CONFIG_UNITY -Os

DEBUG   = Debug
RELEASE = Release

DEBUG_SOURCES = \
    $(SOURCE_DIR)/API/SBAlgorithm.c \
    $(SOURCE_DIR)/API/SBAllocator.c \
    $(SOURCE_DIR)/API/SBAttributeList.c \
    $(SOURCE_DIR)/API/SBAttributeRegistry.c \
    $(SOURCE_DIR)/API/SBBase.c \
    $(SOURCE_DIR)/API/SBCodepoint.c \
    $(SOURCE_DIR)/API/SBCodepointSequence.c \
    $(SOURCE_DIR)/API/SBLine.c \
    $(SOURCE_DIR)/API/SBLog.c \
    $(SOURCE_DIR)/API/SBMirrorLocator.c \
    $(SOURCE_DIR)/API/SBParagraph.c \
    $(SOURCE_DIR)/API/SBScriptLocator.c \
    $(SOURCE_DIR)/API/SBText.c \
    $(SOURCE_DIR)/API/SBTextConfig.c \
    $(SOURCE_DIR)/API/SBTextIterators.c \
    $(SOURCE_DIR)/Core/List.c \
    $(SOURCE_DIR)/Core/Memory.c \
    $(SOURCE_DIR)/Core/Object.c \
    $(SOURCE_DIR)/Core/Once.c \
    $(SOURCE_DIR)/Data/BidiTypeLookup.c \
    $(SOURCE_DIR)/Data/GeneralCategoryLookup.c \
    $(SOURCE_DIR)/Data/PairingLookup.c \
    $(SOURCE_DIR)/Data/ScriptLookup.c \
    $(SOURCE_DIR)/Script/ScriptStack.c \
    $(SOURCE_DIR)/Text/AttributeDictionary.c \
    $(SOURCE_DIR)/Text/AttributeManager.c \
    $(SOURCE_DIR)/UBA/BidiChain.c \
    $(SOURCE_DIR)/UBA/BracketQueue.c \
    $(SOURCE_DIR)/UBA/IsolatingRun.c \
    $(SOURCE_DIR)/UBA/LevelRun.c \
    $(SOURCE_DIR)/UBA/RunQueue.c \
    $(SOURCE_DIR)/UBA/StatusStack.c
RELEASE_SOURCES = $(SOURCE_DIR)/SheenBidi.c

DEBUG_OBJECTS   = $(DEBUG_SOURCES:$(SOURCE_DIR)/%.c=$(DEBUG)/%.o)
RELEASE_OBJECTS = $(RELEASE_SOURCES:$(SOURCE_DIR)/%.c=$(RELEASE)/%.o)

DEBUG_TARGET   = $(DEBUG)/lib$(LIB_SHEENBIDI).a
PARSER_TARGET  = $(DEBUG)/lib$(LIB_PARSER).a
TESTS_TARGET   = $(DEBUG)/$(EXEC_TESTS)
RELEASE_TARGET = $(RELEASE)/lib$(LIB_SHEENBIDI).a

all:     release
release: $(RELEASE_TARGET)
debug:   $(DEBUG_TARGET)

check: tests
	./Debug/Tests Tools/Unicode

clean:
	$(RM) $(DEBUG) $(RELEASE)

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(AR) $(ARFLAGS) $(DEBUG_TARGET) $(DEBUG_OBJECTS)

$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	$(AR) $(ARFLAGS) $(RELEASE_TARGET) $(RELEASE_OBJECTS)

$(DEBUG)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(RELEASE)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

.PHONY: all check clean compiler debug parser release tests

include $(PARSER_DIR)/Makefile
include $(TESTS_DIR)/Makefile
