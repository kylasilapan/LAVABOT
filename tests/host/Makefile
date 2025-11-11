EP_DIR := $(CURDIR)/EpoxyDuino
AUNIT_DIR := $(CURDIR)/AUnit
APP := test_schedule

CXX := g++
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -I$(EP_DIR) -I$(AUNIT_DIR)/src -I$(CURDIR)/../../src

SRCS := test_schedule.cpp test_system_state.cpp test_durability.cpp $(EP_DIR)/main.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(OBJS) -o $(APP)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP) $(OBJS)

