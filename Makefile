CXX      := g++
CXXFLAGS := -Wall
CPPFLAGS := -IIncludes
LDFLAGS  :=
LDLIBS   :=

SRCS := tests/main.cpp $(wildcard Modules/*.cpp)

OBJS := $(patsubst %.cpp, build/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

TARGET := search

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)
