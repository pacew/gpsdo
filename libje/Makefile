ARM_PREFIX = arm-none-eabi

ARM_CC = $(ARM_PREFIX)-gcc
ARM_AR = $(ARM_PREFIX)-ar

ARM_CFLAGS = -mcpu=cortex-m4 \
	-g -Wall \
	-nostdinc -I. \
	-mthumb \
	-ffreestanding \
	-fstrict-volatile-bitfields

FILES = strncmp.armo

all: libje.a

libje.a: $(FILES)
	rm -f libje.a
	$(ARM_AR) rs libje.a $(FILES)

.SUFFIXES: .armo

.c.armo:
	$(ARM_CC) $(ARM_CFLAGS) -c -o $*.armo $*.c

clean:
	rm -f libje.a *.armo
