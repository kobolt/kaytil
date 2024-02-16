CFLAGS=-Wall -Wextra -DDISABLE_Z80_TRACE -D_POSIX_C_SOURCE -std=c99

all: kaytil cbios.bin cpm22.bin

kaytil: main.o z80.o mem.o io.o disk.o console.o
	gcc -o kaytil $^ ${CFLAGS}

cbios.bin: cbios.hex
	srec_cat cbios.hex -intel -o cbios.tmp -binary
	dd bs=1 skip=64000 if=cbios.tmp of=cbios.bin
	rm cbios.tmp

cpm22.bin: cpm22.hex
	srec_cat cpm22.hex -intel -o cpm22.tmp -binary
	dd bs=1 skip=58368 if=cpm22.tmp of=cpm22.bin
	rm cpm22.tmp

main.o: main.c
	gcc -c $^ ${CFLAGS}

z80.o: z80.c
	gcc -c $^ ${CFLAGS}

mem.o: mem.c
	gcc -c $^ ${CFLAGS}

io.o: io.c
	gcc -c $^ ${CFLAGS}

disk.o: disk.c
	gcc -c $^ ${CFLAGS}

console.o: console.c
	gcc -c $^ ${CFLAGS}

.PHONY: clean
clean:
	rm -f *.o kaytil

