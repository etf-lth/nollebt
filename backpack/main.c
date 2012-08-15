/*
 * Ericsson Bluetooth Baseband Controller
 * Backpack Main
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"

#include "uart.h"
#include "transport.h"
#include "utils.h"
#include "signals.h"

#include <stdio.h>

#include "lwbt/phybusif.h"
#include "lwbt/lwbt_memp.h"
#include "lwbt/hci.h"
#include "lwbt/l2cap.h"
#include "lwbt/sdp.h"
#include "lwbt/rfcomm.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/stats.h"

/*
 * vanilla bss at 00000e74 and 54548 bytes long:
 * bss ends at 0000e388
 * 
 * our bss at 0000e400 until end of space and time (15 kB-ish)
 */

extern void _text, _etext, _data, _edata, _bss, _ebss;

void bpMain(void);

//extern void InitHW(void);
extern void bt_spp_start(void);
extern void bt_spp_tmr(void);
extern void spp_write(char *buf, int len);

// use a struct to make sure they stay in order.
// failing to do so will result in crashes or undefined behaviour,
// as OSE will initialize the memory between those areas and
// thus overwrite other application variables.
struct {
    char stack[1024]; // those sizes are hardcoded in head.S
    char stack2[1024];
    char bpPCB[92];
} procmem;

/*
 * This structure defines our process so
 * OSE can schedule it and pass signals to it.
 * Additional magic is done in head.S and the patching script.
 */
const PROCINIT backpackProc = {
    .type = 0,
    .entry = bpMain,
    .prio = 4,
    .ptr1 = (void *)0x73d0, // linked list of processes with given prio? wrong (=other) value will crash OSE.
    .pcb = procmem.bpPCB,
    .ptr2 = procmem.stack+sizeof(procmem.stack), // top of stack?
    .ptr3 = procmem.stack, // bottom of stack?
    .pid = PID_BACKPACK,
    .name = "backpack"
};

/*
 * This might be called by the runtime functions in math.S
 */
void __div0(void)
{
    UART2WriteString("OHMYGAAH! Division By Zero!\n");
    for (;;);
}

/*
 * Initialize the data segment from flash and clear the .bss seg.
 */
void InitDataZeroBSS(void)
{
    register char *src = &_etext;
    register char *dst = &_data;
    register char *end = &_edata;

    while (dst < end)
        *dst++ = *src++;

    for (dst = &_bss, end = &_ebss; dst < end; dst++)
        *dst = 0;
}

/*
 * This gets called before OSE is initialized.
 * It must return zero.
 */
int init_hook(void)
{
    InitDataZeroBSS();

    //InitHW();
    //UARTInit();
    //UARTSetBaudRate(UART_115200);

    // Wait for the UART to empty the TX buffer
    //while (UARTGetTxFIFOSize() > 0) ;

    // We must return 0, or we'll get unknown behaviour
    return 0;
}

/*
 * This gets called through the OSE interrupt handling mechanism.
 */
void uart2_rx_int(int bytes)
{
    SIGNAL *s = OSE_alloc(3+bytes, SIG_UART_RX);

    s->raw[2] = bytes;
    int idx = 0;
    while (bytes--) {
        s->raw[3+idx++] = UART2ReadByte();
    }

    OSE_send(&s, PID_BACKPACK);
}

int lwbt_init(void)
{
    //printf("Initializing lwBT...\n");
    mem_init();
    memp_init();
    pbuf_init();
    //printf("Memory management initialized\n");

    lwbt_memp_init();
    transport_init();
    if (hci_init() != ERR_OK) {
        printf("HCI initialization failed\n");
        return -1;
    }

    l2cap_init();
    sdp_init();
    rfcomm_init();
    //printf("lwBT initialized\n");
    return 0;
}

void lwbt_timer(void)
{
    //printf("lwbt_timer\n");
	l2cap_tmr();
	rfcomm_tmr();
	bt_spp_tmr();
}

/*
 * This is the entry point of our thread
 */
void bpMain(void)
{
    //unsigned int sp;
    //asm ("mov %0, sp":"=r" (sp));

    //UART2SetBaudRate(UART_115200);

    printf("IRMA Backpack (c) 2012 <fredrik@z80.se>\n");
    printf("Build: %s, %s\n\n", build_time, build_comment);

    //printf("My stack is at %08x\n", sp);
    
    lwbt_init();

    bt_spp_start();

    //printf("Application started\n");

    timer_add(1000, SIG_TIMER);

    //printf("We're going in...\n\n");

    static const SIGSELECT anysig[] = {0};
    for(;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_TIMER:
            lwbt_timer();
            timer_add(1000, SIG_TIMER);
            break;

        case SIG_TRANSPORT_EVENT:
        case SIG_TRANSPORT_DATA:
            transport_input(s);
            break;

        case SIG_UART_RX:
            spp_write(&s->raw[3], s->raw[2]);
            break;

        default:
            printf("unhandled signal %08x\n", s->sig_no);
        }

        OSE_free_buf(&s);
    }
}
