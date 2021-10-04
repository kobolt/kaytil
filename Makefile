
TOOL_PATH:=/opt/gcc-rx-elf/bin

CFLAGS = -c -O2 -ffunction-sections -fdata-sections -mcpu=rx600 -I. -Isakura -Isakura/common -Wall -Wextra
LDFLAGS = -Wl,--gc-sections -nostartfiles

################################################################################

all: kaytil.bin

kaytil.bin: kaytil.elf
	$(TOOL_PATH)/rx-elf-objcopy -O binary $^ $@

kaytil.elf: main.o z80.o mem.o io.o disk.o console.o fat16.o crt0.o stubs.o led.o timer.o uart.o sdcard.o cpm22.o cbios.o
	$(TOOL_PATH)/rx-elf-gcc $(LDFLAGS) -T sakura/common/sakura_rx.ld $^ -o $@

################################################################################

main.o: main.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

z80.o: z80.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

mem.o: mem.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

io.o: io.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

disk.o: disk.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

console.o: console.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

fat16.o: fat16.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

################################################################################

crt0.o: sakura/common/sakura_crt0.S
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

stubs.o: sakura/common/sakura_stubs.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

led.o: sakura/led.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

timer.o: sakura/timer.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

uart.o: sakura/uart.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

sdcard.o: sakura/sdcard.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

################################################################################

cbios.o: cbios.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

cpm22.o: cpm22.c
	$(TOOL_PATH)/rx-elf-gcc $(CFLAGS) $^ -o $@

cbios.c: cbios.bin
	python2 bin2c.py cbios.bin > cbios.c

cpm22.c: cpm22.bin
	python2 bin2c.py cpm22.bin > cpm22.c

cbios.bin: cbios.hex
	srec_cat cbios.hex -intel -o cbios.tmp -binary
	dd bs=1 skip=64000 if=cbios.tmp of=cbios.bin
	rm cbios.tmp

cpm22.bin: cpm22.hex
	srec_cat cpm22.hex -intel -o cpm22.tmp -binary
	dd bs=1 skip=58368 if=cpm22.tmp of=cpm22.bin
	rm cpm22.tmp

################################################################################

.PHONY: clean
clean:
	rm -f *.o *.elf *.bin cbios.c cpm22.c


