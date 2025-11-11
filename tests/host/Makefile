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

# EpoxyDuino main.cpp - try root first, then src/
EP_MAIN := $(wildcard $(EP_DIR)/main.cpp $(EP_DIR)/src/main.cpp)
EP_MAIN := $(firstword $(EP_MAIN))
ifeq ($(EP_MAIN),)
  # Fallback: search for it
  EP_MAIN := $(shell find $(EP_DIR) -name "main.cpp" -type f 2>/dev/null | head -1)
endif

# Find AUnit source files
AUNIT_SRCS := $(shell find $(AUNIT_DIR)/src -name "*.cpp" -type f 2>/dev/null)
# Convert full paths to simple object names: AUnit/src/aunit/TestAgain.cpp -> aunit_TestAgain.o
# Extract just the filename and create object name
AUNIT_OBJS := $(foreach src,$(AUNIT_SRCS),$(patsubst %.cpp,%.o,$(notdir $(src))))
AUNIT_OBJS := $(addprefix aunit_,$(AUNIT_OBJS))

# Find EpoxyDuino implementation files (cores/epoxy/*.cpp, excluding main.cpp)
EP_IMPL_SRCS := $(shell find $(EP_DIR)/cores/epoxy -name "*.cpp" -type f 2>/dev/null | grep -v main.cpp)
EP_IMPL_OBJS := $(patsubst $(EP_DIR)/cores/epoxy/%.cpp,%_epoxy.o,$(EP_IMPL_SRCS))

# Test source files
TEST_SRCS := test_schedule.cpp test_system_state.cpp test_durability.cpp test_integration.cpp test_system_iot.cpp
TEST_OBJS := test_schedule.o test_system_state.o test_durability.o test_integration.o test_system_iot.o

# All object files
OBJS := $(TEST_OBJS) $(AUNIT_OBJS) $(EP_IMPL_OBJS)

# Add EpoxyDuino main.cpp
ifneq ($(EP_MAIN),)
  OBJS += epoxy_main.o
endif

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(OBJS) -o $(APP)

# Pattern rule for local test files
test_%.o: test_%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create explicit rules for each AUnit source file
define AUNIT_RULE
aunit_$(notdir $(1:.cpp=.o)): $(1)
	@echo "Compiling AUnit: $$< -> $$@"
	$$(CXX) $$(CXXFLAGS) -c $$< -o $$@
endef

$(foreach src,$(AUNIT_SRCS),$(eval $(call AUNIT_RULE,$(src))))

# Pattern rule for EpoxyDuino implementation files
%_epoxy.o: $(EP_DIR)/cores/epoxy/%.cpp
	@echo "Compiling EpoxyDuino: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Special rule for EpoxyDuino main.cpp (only if EP_MAIN is found)
ifneq ($(EP_MAIN),)
epoxy_main.o: $(EP_MAIN)
	@echo "Compiling EpoxyDuino main from $(EP_MAIN)"
	$(CXX) $(CXXFLAGS) -c $< -o $@
else
epoxy_main.o:
	@echo "Error: EpoxyDuino main.cpp not found. Please ensure EpoxyDuino is cloned."
	@exit 1
endif

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP) $(OBJS) epoxy_main.o $(AUNIT_OBJS) $(EP_IMPL_OBJS)

