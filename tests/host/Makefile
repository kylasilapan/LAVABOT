EP_DIR := $(CURDIR)/EpoxyDuino
AUNIT_DIR := $(CURDIR)/AUnit
APP := test_schedule

CXX := g++
# Include EpoxyDuino headers - covers all common locations for Print.h
# AUnit needs Arduino headers which EpoxyDuino provides
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra \
            -I$(EP_DIR) \
            -I$(EP_DIR)/core \
            -I$(EP_DIR)/core/arduino \
            -I$(EP_DIR)/cores/epoxy \
            -I$(EP_DIR)/libraries \
            -I$(EP_DIR)/src \
            -I$(AUNIT_DIR)/src \
            -I$(CURDIR)/../../src \
            -DARDUINO=10800 \
            -DEPOXY_DUINO \
            -D__AVR_ATmega328P__

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

