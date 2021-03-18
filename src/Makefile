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

PROG = blink

all: $(PROG).elf

regs.h vectors.S: mkregs $(PROG).c
	./mkregs STM32F407.svd $(PROG).c

BLINK_OBJS = vectors.armo blink.armo
$(BLINK_OBJS): regs.h
blink.elf: $(BLINK_OBJS) Makefile ldscript
	$(ARM_CC) $(ARM_CFLAGS) -o blink.elf $(BLINK_OBJS) $(ARM_LIBS)
	$(ARM_PREFIX)-size blink.elf

UART_OBJS = vectors.armo uart.armo printf.armo
$(UART_OBJS): regs.h
uart.elf: $(UART_OBJS) Makefile ldscript
	$(ARM_CC) $(ARM_CFLAGS) -o uart.elf $(UART_OBJS) $(ARM_LIBS)
	$(ARM_PREFIX)-size uart.elf

TUSB_OBJS = tusb.armo
USB_OBJS = vectors.armo usb.armo printf.armo $(TUSB_OBJS)
$(USB_OBJS): regs.h
usb.elf: $(USB_OBJS) Makefile ldscript
	$(ARM_CC) $(ARM_CFLAGS) -o usb.elf $(USB_OBJS) $(ARM_LIBS)
	$(ARM_PREFIX)-size usb.elf

tusb.armo: ../tinyusb/src/tusb.c
	$(ARM_CC) $(ARM_CFLAGS) -c ../tinyusb/src/tusb.c



.PHONY: upload
upload: $(PROG).elf
	./upload $(PROG).elf || (cat openocd.log; false)

dis:
	$(ARM_OBJDUMP) --disassemble $(PROG).elf

clean:
	rm -f *.armo $(PROG).elf regs.h vectors.S

