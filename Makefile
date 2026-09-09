CXX = g++

STD = -std=c++23
WARNINGS = -Wall -Wextra -Wpedantic
MEMORY_CHECKS = -fsanitize=address,undefined -fno-omit-frame-pointer
INCLUDES = -Ivendor/mpc -Ivendor/arenas

# Find all C++ source files
SRC := $(shell find src vendor -name "*.cpp" -o -name "*.cc" -o -name "*.cxx")

# System libraries
LIBS = -ledit -lm
# For sanitizers (needed at link time)
LDFLAGS = $(MEMORY_CHECKS)

# Binary location
BINARY = bin/run
BIN_DIR = bin

debug: CXXFLAGS = -g -O0 $(STD) $(WARNINGS) $(MEMORY_CHECKS) $(INCLUDES)
release: CXXFLAGS = -O2 -DNDEBUG $(STD) $(WARNINGS) $(INCLUDES)

debug release: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(SRC) $(LIBS) $(LDFLAGS) -o $(BINARY)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Get everything after 'run' as arguments
ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

run: debug
	@printf "\n"
	@./$(BINARY) $(ARGS)
	@$(MAKE) --no-print-directory clean

# This line tells make to ignore the args
$(ARGS): run
	@:

clean: 
	@rm -rf $(BIN_DIR)
