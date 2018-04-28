TESTER_INCLUDES = -I$(ROOT_DIR) -I$(HEADERS_DIR) -I$(TOOLS_DIR)
TESTER_FLAGS = --coverage $(TESTER_INCLUDES)
TESTER_LIBS = -L$(DEBUG) -l$(LIB_SHEENBIDI) -l$(LIB_PARSER)

TESTER      = $(DEBUG)/Tester
TESTER_UTIL = $(TESTER)/Utilities

TESTER_SRCS = $(TESTER_DIR)/AlgorithmTester.cpp \
              $(TESTER_DIR)/BidiTypeLookupTester.cpp \
              $(TESTER_DIR)/BracketLookupTester.cpp \
              $(TESTER_DIR)/CodepointSequenceTester.cpp \
              $(TESTER_DIR)/Configuration.cpp \
              $(TESTER_DIR)/GeneralCategoryLookupTester.cpp \
              $(TESTER_DIR)/main.cpp \
              $(TESTER_DIR)/MirrorLookupTester.cpp \
              $(TESTER_DIR)/ScriptLocatorTester.cpp \
              $(TESTER_DIR)/ScriptLookupTester.cpp \
              $(TESTER_DIR)/Utilities/Convert.cpp \
              $(TESTER_DIR)/Utilities/Unicode.cpp

TESTER_OBJS = $(TESTER_SRCS:$(TESTER_DIR)/%.cpp=$(TESTER)/%.o)

$(TESTER):
	mkdir $(TESTER)
	mkdir $(TESTER_UTIL)

$(TESTER)/%.o: $(TESTER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(TESTER_FLAGS) -c $< -o $@

$(TESTER_TARGET): $(TESTER_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(TESTER_FLAGS) $(TESTER_LIBS)

tester: debug parser $(TESTER) $(TESTER_TARGET)

tester_clean:
	$(RM) $(TESTER)/*.o
	$(RM) $(TESTER_UTIL)/*.o
	$(RM) $(TESTER_TARGET)
