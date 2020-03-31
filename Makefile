
##
## List here your source files (both .s, .c and .cpp)
##
SRC := source/main.c

##
## Drivers' source files and include directories
##
DRIVERS_INC :=
DRIVERS_SRC :=

##
## List here additional static libraries with relative path
##
LIBS :=

##
## List here additional include directories (in the form -Iinclude_dir)
##
INCLUDE_DIRS :=

##
## List here additional defines
##
DEFINES :=

##
## Define used to select target processor
##
TARGET := -DSTM32F40_41xxx

##
## System clock frequency, in hertz. Must be defined and set to correct value
## in order to make drivers working correctly
##
CLK_FREQ :=

##
## Optimization level
##
OPTLEVEL := -O0
#OPTLEVEL:= -O2
#OPTLEVEL:= -O3
#OPTLEVEL:= -Os

##
## Device-specific source files and include directories, e.g. startup code
##
DEVICE_INC := -ICMSIS -Idevice
DEVICE_SRC :=                \
startup/startup.cpp          \
startup/libc_integration.cpp \
device/system_stm32f4xx.c

##
## Operating system's source files and include directories
##
OS_INC := -Ifreertos/portable/GCC/ARM_CM4F -Ifreertos/include
OS_SRC :=                             \
freertos/croutine.c                   \
freertos/event_groups.c               \
freertos/list.c                       \
freertos/queue.c                      \
freertos/stream_buffer.c              \
freertos/tasks.c                      \
freertos/timers.c                     \
freertos/portable/GCC/ARM_CM4F/port.c \
freertos/portable/MemMang/heap_3.c

##
## Exceptions support. Uncomment to disable them and save code size
##
#EXCEPT := -fno-exceptions -fno-rtti -D__NO_EXCEPTIONS

##############################################################################
## You should not need to modify anything below                             ##
##############################################################################

ALL_INC := -Iinclude $(OS_INC) $(DEVICE_INC) $(DRIVERS_INC) $(INCLUDE_DIRS)
ALL_SRC := $(SRC) $(OS_SRC) $(DEVICE_SRC) $(DRIVERS_SRC)
CONFIGS := $(TARGET) $(CLK_FREQ) $(OPTLEVEL) -DDONT_USE_CMSIS_INIT

ifeq ("$(VERBOSE)","1")
Q := 
ECHO := @true
else
Q := @
ECHO := @echo
endif

## Replaces both "foo.cpp"-->"foo.o" and "foo.c"-->"foo.o"
OBJ := $(addsuffix .o, $(basename $(ALL_SRC)))

CXXFLAGS := $(ALL_INC) -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp $(CONFIGS) \
            $(DEFINES) $(EXCEPT) -c -g -std=c++11
CFLAGS   := $(ALL_INC) -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp $(CONFIGS) \
            $(DEFINES) $(EXCEPT) -c -g
AFLAGS   := -mcpu=cortex-m4 -mthumb
LFLAGS   := -mcpu=cortex-m4 -mthumb -Wl,--gc-sections -Wl,-Map,main.map \
 	    $(OPTLEVEL) -nostdlib -Wl,-T./linkerscripts/linker_script.ld
DFLAGS   := -MMD -MP

LINK_LIBS := $(LIBS) -Wl,--start-group -lc -lgcc -Wl,--end-group

CC  := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AS  := arm-none-eabi-as
CP  := arm-none-eabi-objcopy
SZ  := arm-none-eabi-size

all: main.bin

main.bin: main.elf
	$(ECHO) "[CP  ] main.hex"
	$(Q)$(CP) -O ihex   main.elf main.hex
	$(ECHO) "[CP  ] main.bin"
	$(Q)$(CP) -O binary main.elf main.bin
	$(Q)$(SZ) main.elf

main.elf: $(OBJ) #all-recursive
	$(ECHO) "[LD  ] main.elf"
	$(Q)$(CXX) $(LFLAGS) -o main.elf $(OBJ) $(LINK_LIBS)

%.o: %.s
	$(ECHO) "[AS  ] $<"
	$(Q)$(AS)  $(AFLAGS) $< -o $@

%.o : %.c
	$(ECHO) "[CC  ] $<"
	$(Q)$(CC)  $(DFLAGS) $(CFLAGS) $< -o $@

%.o : %.cpp
	$(ECHO) "[CXX ] $<"
	$(Q)$(CXX) $(DFLAGS) $(CXXFLAGS) $< -o $@

flash: main_wrapped.bin
	$(ECHO) "[DFU ] $<"
	$(Q)./scripts/md380_dfu.py upgrade $<

main_wrapped.bin: main.bin
	$(ECHO) "[WRAP] $<"
	$(Q)./scripts/md380_fw.py --wrap $< $@

clean:
	-rm -f $(OBJ) main.elf main.hex main.bin main.map main_wrapped.bin $(OBJ:.o=.d)

#pull in dependecy info for existing .o files
-include $(OBJ:.o=.d)
