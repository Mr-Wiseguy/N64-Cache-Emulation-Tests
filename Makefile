### User Configuration ###

# Name of ROM to build
TARGET := testing

# Do not change below this line!

# System config
SDK_INCLUDE := -I /etc/n64/usr/include
ULTRA_LINKER := -L /etc/n64/lib
GCC_LINKER := -L $(N64_LIBGCCDIR)
LIBULTRA := ultra_rom

### Tools ###

# System tools
MKDIR := mkdir
MKDIR_OPTS := -p

RMDIR := rm
RMDIR_OPTS := -rf

PRINT := printf '
  ENDCOLOR := \033[0m
  WHITE     := \033[0m
  ENDWHITE  := $(ENDCOLOR)
  GREEN     := \033[0;32m
  ENDGREEN  := $(ENDCOLOR)
  BLUE      := \033[0;34m
  ENDBLUE   := $(ENDCOLOR)
  YELLOW    := \033[0;33m
  ENDYELLOW := $(ENDCOLOR)
ENDLINE := \n'

PREFIX  := mips-n64-

# Build tools
CC      := $(N64CHAIN)$(PREFIX)g++ -x c
AS      := $(N64CHAIN)$(PREFIX)g++ -x assembler-with-cpp
CPP     := $(N64CHAIN)$(PREFIX)cpp
CXX     := $(N64CHAIN)$(PREFIX)g++
LD      := $(N64CHAIN)$(PREFIX)g++
OBJCOPY := $(N64CHAIN)$(PREFIX)objcopy

CKSUM   := tools/n64cksum.py

### Files and Directories ###

# Source files
SRC_ROOT  := src
SRC_DIRS  := $(wildcard $(SRC_ROOT)/*)
C_SRCS    := $(foreach src_dir,$(SRC_DIRS),$(wildcard $(src_dir)/*.c))
CXX_SRCS  := $(foreach src_dir,$(SRC_DIRS),$(wildcard $(src_dir)/*.cpp))
ASM_SRCS  := $(foreach src_dir,$(SRC_DIRS),$(wildcard $(src_dir)/*.s))
SEG_C_SRCS   := $(wildcard segments/*.c)
LD_SCRIPT := n64.ld
BOOT      := boot/boot.6102
ENTRY_AS  := boot/entry.s

# Build root
BUILD_ROOT     := build

# Build folders
BOOT_BUILD_DIR := $(BUILD_ROOT)/boot
BUILD_DIRS     := $(addprefix $(BUILD_ROOT)/,$(SRC_DIRS)) $(BOOT_BUILD_DIR)

# Build files
C_OBJS   := $(addprefix $(BUILD_ROOT)/,$(C_SRCS:.c=.o))
CXX_OBJS := $(addprefix $(BUILD_ROOT)/,$(CXX_SRCS:.cpp=.o))
ASM_OBJS := $(addprefix $(BUILD_ROOT)/,$(ASM_SRCS:.s=.o))
OBJS     := $(C_OBJS) $(CXX_OBJS) $(ASM_OBJS)
LD_CPP   := $(BUILD_ROOT)/$(LD_SCRIPT)
BOOT_OBJ := $(BUILD_ROOT)/$(BOOT).o
ENTRY_OBJ:= $(BUILD_ROOT)/$(ENTRY_AS:.s=.o)
D_FILES  := $(C_OBJS:.o=.d) $(CXX_OBJS:.o=.d) $(LD_CPP).d

CODESEG  := $(BUILD_ROOT)/codesegment.o
Z64      := $(addprefix $(BUILD_ROOT)/,$(TARGET).z64)
ELF      := $(Z64:.z64=.elf)

### Flags ###

# Build tool flags

CFLAGS     := -march=vr4300 -mno-abicalls -mabi=32 -ffreestanding -G 0 -D_LANGUAGE_C -ffunction-sections
CXXFLAGS   := -std=c++20 -fno-rtti -fno-exceptions -march=vr4300 -mno-abicalls -ffreestanding -mabi=32 -G 0 -D_LANGUAGE_C_PLUS_PLUS -ffunction-sections
CPPFLAGS   := -I include -I . -I src/ $(SDK_INCLUDE) -D_FINALROM -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -D_ULTRA64 -D__EXTENSIONS__ -DF3DEX_GBI_2 -DNDEBUG
WARNFLAGS  := -Wall -Wextra -Wpedantic -Wdouble-promotion -Wfloat-conversion
OPTFLAGS   := -Os -ffast-math -gdwarf-3
ASFLAGS    := -mtune=vr4300 -march=vr4300 -mabi=32 -mips3
LDFLAGS    := -T $(LD_CPP) -Wl,--accept-unknown-input-arch -Wl,--no-check-sections -Wl,-Map $(BUILD_ROOT)/$(TARGET).map \
			  $(ULTRA_LINKER) -L lib $(GCC_LINKER) -nostartfiles -Wl,-gc-sections
SEG_LDFLAGS:= $(ULTRA_LINKER) -L lib -lstdc++ -lnustd -ln_mus -ln_audio_sc -l$(LIBULTRA) -e init -Wl,-gc-sections
LDCPPFLAGS := -P -Wno-trigraphs -DBUILD_ROOT=$(BUILD_ROOT) -Umips
OCOPYFLAGS := --pad-to=0x101000 --gap-fill=0xFF

### Rules ###

# Default target, all
all: $(Z64)

# Make directories
$(BUILD_ROOT) $(BUILD_DIRS) :
	@$(PRINT)$(GREEN)Creating directory: $(ENDGREEN)$(BLUE)$@$(ENDBLUE)$(ENDLINE)
	@$(MKDIR) $@ $(MKDIR_OPTS)

# .cpp -> .o
$(BUILD_ROOT)/%.o : %.cpp | $(BUILD_DIRS)
	@$(PRINT)$(GREEN)Compiling C++ source file: $(ENDGREEN)$(BLUE)$<$(ENDBLUE)$(ENDLINE)
	@$(CXX) $< -o $@ -c -MMD -MF $(@:.o=.d) $(CXXFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(WARNFLAGS)

# .cpp -> .o (build folder)
$(BUILD_ROOT)/%.o : $(BUILD_ROOT)/%.cpp | $(BUILD_DIRS)
	@$(PRINT)$(GREEN)Compiling C++ source file: $(ENDGREEN)$(BLUE)$<$(ENDBLUE)$(ENDLINE)
	@$(CXX) $< -o $@ -c -MMD -MF $(@:.o=.d) $(CXXFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(WARNFLAGS)

# .c -> .o
$(BUILD_ROOT)/%.o : %.c | $(BUILD_DIRS)
	@$(PRINT)$(GREEN)Compiling C source file: $(ENDGREEN)$(BLUE)$<$(ENDBLUE)$(ENDLINE)
	@$(CC) $< -o $@ -c -MMD -MF $(@:.o=.d) $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(WARNFLAGS)

# .s -> .o
$(BUILD_ROOT)/%.o : %.s | $(BUILD_DIRS)
	@$(PRINT)$(GREEN)Assembling ASM source file: $(ENDGREEN)$(BLUE)$<$(ENDBLUE)$(ENDLINE)
	@$(AS) $< -o $@ -c -MMD -MF $(@:.o=.d) $(ASFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(WARNFLAGS)

# boot -> .o
$(BOOT_OBJ) : $(BOOT) | $(BOOT_BUILD_DIR)
	@$(PRINT)$(GREEN)Copying boot to object file$(ENDGREEN)$(ENDLINE)
	@$(OBJCOPY) -I binary -O elf32-big $< $@

# All .o -> codesegment.o
$(CODESEG) : $(OBJS)
	@$(PRINT)$(GREEN)Combining code objects into code segment$(ENDGREEN)$(ENDLINE)
	@$(LD) -o $@ -r $^ $(SEG_LDFLAGS)

# .o -> .elf
$(ELF) : $(CODESEG) $(LD_CPP) $(BOOT_OBJ) $(ENTRY_OBJ)
	@$(PRINT)$(GREEN)Linking elf file:$(ENDGREEN)$(BLUE)$@$(ENDBLUE)$(ENDLINE)
	@$(LD) $(LDFLAGS) -o $@

# .elf -> .z64
$(Z64) : $(ELF)
	@$(PRINT)$(GREEN)Creating z64: $(ENDGREEN)$(BLUE)$@$(ENDBLUE)$(ENDLINE)
	@$(OBJCOPY) $< $@ -O binary $(OCOPYFLAGS)
	@$(PRINT)$(GREEN)Calculating checksums$(ENDGREEN)$(ENDLINE)
	@$(CKSUM) $@
	@$(PRINT)$(WHITE)ROM Built!$(ENDWHITE)$(ENDLINE)

# Preprocess LD script
$(LD_CPP) : $(LD_SCRIPT) | $(BUILD_ROOT)
	@$(PRINT)$(GREEN)Preprocessing linker script$(ENDGREEN)$(ENDLINE)
	@$(CPP) $(LDCPPFLAGS) -MMD -MP -MT $@ -MF $@.d -o $@ $<

clean:
	@$(PRINT)$(YELLOW)Cleaning build$(ENDYELLOW)$(ENDLINE)
	@$(RMDIR) $(BUILD_ROOT) $(RMDIR_OPTS)

.PHONY: all clean load

-include $(D_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
