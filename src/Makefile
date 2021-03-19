ARM_PREFIX = arm-none-eabi

ARM_CC = $(ARM_PREFIX)-gcc
ARM_OBJDUMP = $(ARM_PREFIX)-objdump
# was -Os
ARM_CFLAGS = -g -Wall -I. -I../tinyusb/src -mthumb -nostdlib -mcpu=cortex-m4 \
	-ffreestanding -nodefaultlibs \
	-T ldscript -Wl,--gc-sections,--relax \
	-nostartfiles -nostdlib
ARM_LIBS = -lgcc

.SUFFIXES: .armo

.c.armo:
	$(ARM_CC) $(ARM_CFLAGS) -c -o $*.armo $*.c

.S.armo:
	$(ARM_CC) $(ARM_CFLAGS) -c -x assembler-with-cpp -o $*.armo $*.S

#PROG = blink
PROG = start_f407

all: $(PROG).elf

regs.h vectors.S: mkregs $(PROG).c
	./mkregs STM32F407.svd $(PROG).c

BLINK_OBJS = vectors.armo blink.armo
$(BLINK_OBJS): regs.h
blink.elf: $(BLINK_OBJS) Makefile ldscript
	$(ARM_CC) $(ARM_CFLAGS) -o blink.elf $(BLINK_OBJS) $(ARM_LIBS)
	$(ARM_PREFIX)-size blink.elf

START_F407_OBJS = vectors.armo start_f407.armo
$(START_F407_OBJS): regs.h
start_f407.elf: $(START_F407_OBJS) Makefile ldscript
	$(ARM_CC) $(ARM_CFLAGS) -o start_f407.elf $(START_F407_OBJS) $(ARM_LIBS)
	$(ARM_PREFIX)-size start_f407.elf


clean:
	rm -f *.armo $(PROG).elf regs.h vectors.S

