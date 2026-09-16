CXX = g++

STD = -std=c++23
WARNINGS = -Wall -Wextra -Wpedantic
MEMORY_CHECKS = -fsanitize=address,undefined -fno-omit-frame-pointer
INCLUDES = -Iinclude -Ivendor/mpc -Ivendor/arenas

SRC := $(shell find src vendor -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \))
OBJ := $(patsubst %.cpp,build/%.cpp.o,$(filter %.cpp,$(SRC))) \
       $(patsubst %.cc,build/%.cc.o,$(filter %.cc,$(SRC))) \
       $(patsubst %.cxx,build/%.cxx.o,$(filter %.cxx,$(SRC)))

LIBS = -ledit -lm
LDFLAGS = $(MEMORY_CHECKS)

BINARY = bin/run

debug: CXXFLAGS = -g -O0 $(STD) $(WARNINGS) $(MEMORY_CHECKS) $(INCLUDES) -MMD -MP
debug: $(BINARY)

release: CXXFLAGS = -O2 -DNDEBUG $(STD) $(WARNINGS) $(INCLUDES) -MMD -MP
release: $(BINARY)

$(BINARY): $(OBJ) | bin
	@$(CXX) $(OBJ) $(LIBS) $(LDFLAGS) -o $@

build/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.cxx.o: %.cxx
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJ:.o=.d)

bin:
	@mkdir -p $@

ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

run: debug
	@printf "\n"
	@./$(BINARY) $(ARGS)

$(ARGS): run
	@:

clean:
	@rm -rf build bin
