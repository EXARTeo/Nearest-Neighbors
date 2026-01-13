CXX      := g++
CXXFLAGS := -Wall -O3 -march=native -DNDEBUG

CPPFLAGS := -IIncludes
LDFLAGS  :=
LDLIBS   :=

SRCS := main.cpp $(wildcard Modules/*.cpp)

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

experiments: search
	bash scripts/run_experiments.sh

summarize:
	python3 scripts/summarize_ann_results.py

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)
