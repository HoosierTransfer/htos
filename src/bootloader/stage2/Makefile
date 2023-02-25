BUILD_DIR?=build/
ASM?=nasm
CC16?=./openwatcom/binl/wcc
LD16?=./openwatcom/binl/wlink

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.obj, $(SOURCES_ASM))


.PHONY: all stage2 clean

all: stage2

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin:
	$(ASM) main.asm -f bin -o $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2/c/%.obj

clean:
	rm -f $(BUILD_DIR)/stage2.bin
