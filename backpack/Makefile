CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
LDNAME = backpack.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -I. -I./lwbt/ -I ./fulibc/inc/ -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g

LWBT = lwbt/fcs.o lwbt/hci.o lwbt/l2cap.o lwbt/lwbt_memp.o lwbt/rfcomm.o lwbt/sdp.o
LWIP = lwip/mem.o lwip/memp.o lwip/pbuf.o lwip/inet.o lwip/stats.o
FULIBC = $(subst .c,.o,$(shell find fulibc/ -name '*.c'))

all: backpack.elf

clean:
	rm -rf *.o *.elf *.bin ${LWBT} ${LWIP} ${FULIBC}

backpack.elf: head.o uart.o utils.o main.o transport.o bt_spp.o math.o host/api.o ${LWBT} ${LWIP} ${FULIBC}
	${LD} -T ${LDNAME} -g -o $@ $^
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
