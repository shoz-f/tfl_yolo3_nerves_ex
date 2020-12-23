ifeq ($(MIX_APP_PATH),)
calling_from_make:
	mix compile
endif

ifeq ($(CROSSCOMPILE),)
    ifeq ($(shell uname -s),Linux)
        DEPS_HOME ?= ./extra
        LIB_EXT    = -lpthread -ldl
        TFL_GEN    = linux_x86_64
        INC_EXT    = -I$(DEPS_HOME)/usr/include
    else
        DEPS_HOME ?= ./extra
        LIB_EXT    = -lmman
        TFL_GEN    = windows_x86_64
        INC_EXT    = -I$(DEPS_HOME)/usr/include
    endif
else
    ifeq (, $(findstring "$(MIX_TARGET)","rpi" "rpi0" "rpi2" "rpi3"))
        $(error "unknown target: $(MIX_TARGET)")
    endif

    ifeq ("$(MIX_TARGET)", $(findstring "$(MIX_TARGET)","rpi" "rpi0"))
        DEPS_HOME ?= ./extra
        TFL_GEN    = nerves_armv6
        INC_EXT    = -I$(DEPS_HOME)/usr/include -I$(DEPS_HOME)/usr/include/arm-linux-gnueabi
        LIB_EXT    = -L$(DEPS_HOME)/usr/lib/arm-linux-gnueabi -lpthread -ldl -latomic
    endif
    ifeq ("$(MIX_TARGET)", $(findstring "$(MIX_TARGET)","rpi2" "rpi3"))
        $(info "arm7")
        DEPS_HOME ?= ./extra
        TFL_GEN    = nerves_armv7
        INC_EXT    = -I$(DEPS_HOME)/usr/include -I$(DEPS_HOME)/usr/include/arm-linux-gnueabihf
        LIB_EXT    = -L$(DEPS_HOME)/usr/lib/arm-linux-gnueabihf -lpthread -ldl -latomic
    endif
endif

INCLUDE   = -I./src \
            -I$(DEPS_HOME)/tensorflow_src \
            -I$(DEPS_HOME)/tensorflow_src/tensorflow/lite/tools/make/downloads/flatbuffers/include \
            $(INC_EXT)
DEFINES   = #-D__LITTLE_ENDIAN__ -DTFLITE_WITHOUT_XNNPACK
PROFILE   = #-pg
CXXFLAGS += -O3 -DNDEBUG -fPIC --std=c++11 -fext-numeric-literals $(INCLUDE) $(DEFINES) $(PROFILE)
LDFLAGS  += $(LIB_EXT) -ljpeg $(PROFILE)

LIB_TFL = $(DEPS_HOME)/tensorflow_src/tensorflow/lite/tools/make/gen/$(TFL_GEN)/lib/libtensorflow-lite.a

PREFIX = $(MIX_APP_PATH)/priv
BUILD  = $(MIX_APP_PATH)/obj

#SRC=$(wildcard src/*.cc)
#SRC=src/tfl_interp.cc src/tfl_mnist.cc
SRC_CC = src/tfl_interp.cc src/tfl_yolo3.cc
OBJ=$(SRC_CC:src/%.cc=$(BUILD)/%.o)

all: $(BUILD) $(PREFIX) install

install: $(PREFIX)/tfl_interp

$(BUILD)/%.o: src/%.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(PREFIX)/tfl_interp: $(OBJ)
	$(CXX) $^ $(LIB_TFL) $(LDFLAGS) -o $@

clean:
	rm -f $(PREFIX)/tfl_interp $(OBJ)

$(PREFIX) $(BUILD):
	mkdir -p $@

print-vars:
	@$(foreach v,$(.VARIABLES),$(info $v=$($v)))

.PHONY: all clean calling_from_make install print-vars
