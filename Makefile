CXX = g++

STD = -std=c++23
WARNINGS = -Wall -Wextra -Wpedantic
MEMORY_CHECKS = -fsanitize=address,undefined -fno-omit-frame-pointer
INCLUDES = -Iinclude -Ivendor/mpc -Ivendor/arenas

SRC := $(shell find src vendor -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \))

LIBS = -ledit -lm
BINARY = bin/run


DEBUG_OBJ := $(patsubst %.cpp,build/debug/%.cpp.o,$(filter %.cpp,$(SRC))) \
             $(patsubst %.cc,build/debug/%.cc.o,$(filter %.cc,$(SRC))) \
             $(patsubst %.cxx,build/debug/%.cxx.o,$(filter %.cxx,$(SRC)))

RELEASE_OBJ := $(patsubst %.cpp,build/release/%.cpp.o,$(filter %.cpp,$(SRC))) \
               $(patsubst %.cc,build/release/%.cc.o,$(filter %.cc,$(SRC))) \
               $(patsubst %.cxx,build/release/%.cxx.o,$(filter %.cxx,$(SRC)))


# debug: CXXFLAGS = -g -O0 $(STD) $(WARNINGS) $(MEMORY_CHECKS) $(INCLUDES) -MMD -MP
debug: CXXFLAGS = -g -O0 -gdwarf-4 $(STD) $(WARNINGS) $(MEMORY_CHECKS) $(INCLUDES) -MMD -MP
debug: LDFLAGS = $(MEMORY_CHECKS)
debug: $(DEBUG_OBJ) | bin
	@$(CXX) $(DEBUG_OBJ) $(LIBS) $(LDFLAGS) -o $(BINARY)


release: CXXFLAGS = -O2 -DNDEBUG $(STD) $(WARNINGS) $(INCLUDES) -MMD -MP
release: LDFLAGS =
release: $(RELEASE_OBJ) | bin
	@$(CXX) $(RELEASE_OBJ) $(LIBS) $(LDFLAGS) -o $(BINARY)


build/debug/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/debug/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/debug/%.cxx.o: %.cxx
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@


build/release/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/release/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

build/release/%.cxx.o: %.cxx
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@


-include $(DEBUG_OBJ:.o=.d)
-include $(RELEASE_OBJ:.o=.d)


bin:
	@mkdir -p $@

clean:
	@rm -rf build bin
