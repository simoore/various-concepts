######################################
# target
######################################
TARGET := echo_server

######################################
# building variables
######################################
DEBUG := 1
OPT   := -Og

#######################################
# paths & includes
#######################################
BUILD_DIR := build
LIB_DIR   := ../../lib

INCLUDES := -I.
INCLUDES += -I$(LIB_DIR)/STM32H7xx_HAL_Driver/Inc
INCLUDES += -I$(LIB_DIR)/STM32H7xx_HAL_Driver/Inc/Legacy
INCLUDES += -I$(LIB_DIR)/CMSIS/Device/ST/STM32H7xx/Include
INCLUDES += -I$(LIB_DIR)/CMSIS/Include

######################################
# sources
######################################
# App sources
SRCS := app.cpp
SRCS += base.cpp
SRCS += stm32h7xx_it.c
SRCS += system_stm32h7xx.c

# HAL Sources in ../lib/STM32H7xx_HAL_Driver/Src/
HAL_DIR := $(LIB_DIR)/STM32H7xx_HAL_Driver/Src
SRCS += $(HAL_DIR)/stm32h7xx_hal_cortex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_tim.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_tim_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_uart.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_uart_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_rcc.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_rcc_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_flash.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_flash_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_gpio.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_hsem.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_dma.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_dma_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_mdma.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_pwr.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_pwr_ex.c
SRCS += $(HAL_DIR)/stm32h7xx_hal.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_i2c.c
SRCS += $(HAL_DIR)/stm32h7xx_hal_i2c_ex.c

# ASM sources
SRCS += startup_stm32h743xx.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CPP := $(PREFIX)g++
CC  := $(GCC_PATH)/$(PREFIX)gcc
AS  := $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP  := $(GCC_PATH)/$(PREFIX)objcopy
SZ  := $(GCC_PATH)/$(PREFIX)size
else
CPP := $(PREFIX)g++
CC  := $(PREFIX)gcc
AS  := $(PREFIX)gcc -x assembler-with-cpp
CP  := $(PREFIX)objcopy
SZ  := $(PREFIX)size
endif
HEX := $(CP) -O ihex
BIN := $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
MCU     := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
C_DEFS  := -DUSE_HAL_DRIVER -DSTM32H743xx

ASFLAGS := $(MCU) $(OPT) -Wall -fdata-sections -ffunction-sections
CFLAGS  := $(MCU) $(C_DEFS) $(INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS_WITH_DEP = $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32H743ZITx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys -lstdc++

# -specs=nosys.specs
# -specs=nano.specs
LDFLAGS = $(MCU) -specs=nosys.specs -T$(LDSCRIPT) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# build the application
#######################################

C_SRCS   := $(filter %.c,$(SRCS))
CPP_SRCS := $(filter %.cpp,$(SRCS))
ASM_SRCS := $(filter %.s,$(SRCS))

OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SRCS:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SRCS)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRCS:.c=.o)))
vpath %.c $(sort $(dir $(C_SRCS)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SRCS:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SRCS)))

$(info    OBJECTS is $(OBJECTS))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS_WITH_DEP) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS_WITH_DEP) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CPP) -c $(CFLAGS_WITH_DEP) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
