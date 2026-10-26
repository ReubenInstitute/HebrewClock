CXX      := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -O2 -I/usr/include/freetype2
LDLIBS   := -lfreetype
BUILD    := build
BIN      := Test

SOURCES  := $(filter-out %Test.cpp,$(wildcard *.cpp))
OBJECTS  := $(patsubst %.cpp,$(BUILD)/%.o,$(SOURCES))

ARDUINO_CLI       := arduino-cli
ARDUINO_LIBRARIES := /sdcard/ArduinoDroid/userlibraries
SKETCH_STAGE      := build/sketch/HebrewClock
FQBN_UNO          := arduino:avr:uno
FQBN_ESP32        := esp32:esp32:esp32
FQBN_STM32        := STMicroelectronics:stm32:GenF1

# AVR's loop()/setup() only use Date/Astro/HebrewDate/Custom/Geo (never Clock/
# Display, which pull in <memory>/<vector> that AVR-GCC doesn't ship), so it
# gets its own trimmed staging dir instead of the full $(SOURCES) set.
SKETCH_STAGE_AVR  := build/sketch-avr/HebrewClock
AVR_SOURCES       := Date.cpp Astro.cpp HebrewDate.cpp Custom.cpp Geo.cpp
AVR_HEADERS       := Date.h Astro.h HebrewDate.h Custom.h Geo.h

.PHONY: all test clean test-all uno esp32 stm32

all: test

test: $(BIN)
	./$(BIN)

$(BIN): $(OBJECTS) $(BUILD)/Test.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test-%: $(OBJECTS) $(BUILD)/%Test.o
	$(CXX) $(CXXFLAGS) -o $(BUILD)/$*Test $^ $(LDLIBS)
	$(BUILD)/$*Test

test-all:
	@for name in $(patsubst %Test.cpp,%,$(wildcard *Test.cpp)); do \
		$(MAKE) --no-print-directory test-$$name || exit 1; \
	done

$(SKETCH_STAGE)/HebrewClock.ino: HebrewClock.ino $(SOURCES) $(wildcard *.h)
	@mkdir -p $(SKETCH_STAGE)
	cp -f HebrewClock.ino $(SOURCES) $(wildcard *.h) $(SKETCH_STAGE)/

$(SKETCH_STAGE_AVR)/HebrewClock.ino: HebrewClock.ino $(AVR_SOURCES) $(AVR_HEADERS)
	@mkdir -p $(SKETCH_STAGE_AVR)
	cp -f HebrewClock.ino $(AVR_SOURCES) $(AVR_HEADERS) $(SKETCH_STAGE_AVR)/

uno: $(SKETCH_STAGE_AVR)/HebrewClock.ino
	$(ARDUINO_CLI) compile --fqbn $(FQBN_UNO) --output-dir build/uno $(SKETCH_STAGE_AVR)

esp32: $(SKETCH_STAGE)/HebrewClock.ino
	$(ARDUINO_CLI) compile --verbose --fqbn $(FQBN_ESP32) --libraries $(ARDUINO_LIBRARIES) --output-dir build/esp32 $(SKETCH_STAGE)

stm32: $(SKETCH_STAGE)/HebrewClock.ino
	$(ARDUINO_CLI) compile --fqbn $(FQBN_STM32) --libraries $(ARDUINO_LIBRARIES) --output-dir build/stm32 $(SKETCH_STAGE)

clean:
	rm -rf $(BUILD) $(BIN)
