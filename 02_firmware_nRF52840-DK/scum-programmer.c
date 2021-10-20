/**
SCuM programmer.
*/

#include "uicr_config.h"
#include "nrf52840.h"
#include "openhdlc.h"

//=========================== defines =========================================

const uint8_t APP_VERSION[]         = {0x00,0x01};

#define UART_BUF_SIZE               1
#define NUM_LEDS                    4

#define CHUNK_SIZE                  100

#define CMD_CLEAR                   0x01
#define CMD_CHUNK                   0x02
#define CMD_LOAD                    0x03
#define CMD_RESET                   0x04
#define CMD_GPIOCAL                 0x05

// https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrf52840_dk%2FUG%2Fdk%2Fhw_buttons_leds.html
// Button 1 P0.11
// Button 2 P0.12
// Button 3 P0.24
// Button 4 P0.25
// LED 1 P0.13
// LED 2 P0.14
// LED 3 P0.15
// LED 4 P0.16

//=========================== prototypes ======================================

// commands
static bool cmd_handler_CLEAR(uint8_t* cmd, uint8_t cmdLen);
static bool cmd_handler_CHUNK(uint8_t* cmd, uint8_t cmdLen);
static bool cmd_handler_LOAD(uint8_t* cmd, uint8_t cmdLen);
static bool cmd_handler_RESET(uint8_t* cmd, uint8_t cmdLen);
static bool cmd_handler_GPIOCAL(uint8_t* cmd, uint8_t cmdLen);
// hdlc
static void openhdlc_rx(uint8_t* rxFrame, uint8_t rxFrameLen);
// bsp
static void lfxtal_start(void);
static void hfclock_start(void);
static void led_enable(void);
static void led_advance(void);
static void uart1_init(void);

//=========================== variables =======================================

typedef struct {
    // image
    uint8_t        scumbin[64*1024];
    // uart
    uint8_t        uart_buf_DK_RX[UART_BUF_SIZE];
    uint8_t        uart_buf_DK_TX[UART_BUF_SIZE];
    uint8_t        uart_buf_SCuM_RX[UART_BUF_SIZE];
    uint8_t        uart_buf_SCuM_TX[UART_BUF_SIZE];
    // led
    uint32_t       led_counter;
    
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t       num_command_CLEAR;
    uint32_t       num_command_CHUNK;
    uint32_t       num_command_LOAD;
    uint32_t       num_command_RESET;
    uint32_t       num_command_GPIOCAL;
    uint32_t       num_command_invalid;
    uint32_t       num_task_loops;
    uint32_t       num_ISR_RTC0_IRQHandler;
    uint32_t       num_ISR_RTC0_IRQHandler_COMPARE0;
    uint32_t       num_ISR_UARTE0_UART0_IRQHandler;
    uint32_t       num_ISR_UARTE0_UART0_IRQHandler_ENDRX;
    uint32_t       num_ISR_UARTE1_IRQHandler;
    uint32_t       num_ISR_UARTE1_IRQHandler_ENDRX;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== main ============================================

int main(void) {
    
    // bsp
    lfxtal_start();
    hfclock_start();
    led_enable();
    uart1_init();
    openhdlc_init(&openhdlc_rx);
    
    // main loop
    while(1) {
        
        // wait for event
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event

        // debug
        app_dbg.num_task_loops++;
    }
}

//=========================== private =========================================

//=== commands

static bool cmd_handler_CLEAR(uint8_t* cmd, uint8_t cmdLen) {
    bool returnVal;

    returnVal = false;
    do {
        // check
        if (cmdLen!=0) {
            break;
        }
        returnVal = true;

        // do
        memset(app_vars.scumbin,0x00,sizeof(app_vars.scumbin));

    } while(0);

    return returnVal;
}

static bool cmd_handler_CHUNK(uint8_t* cmd, uint8_t cmdLen) {
    bool    returnVal;
    uint8_t offset;
    uint8_t numchunks;

    returnVal = false;
    do {
        // check
        if (cmdLen<2) {
            break;
        }
        returnVal = true;

        // do
        offset    = cmd[0]; // shorthand
        numchunks = cmd[1]; // shorthand
        memcpy(
            &app_vars.scumbin[CHUNK_SIZE*offset],
            &cmd[2],
            cmdLen-2
        );
    } while(0);

    return returnVal;
}

static bool cmd_handler_LOAD(uint8_t* cmd, uint8_t cmdLen) {
    bool returnVal;

    returnVal = false;
    do {
        // check
        if (cmdLen!=0) {
            break;
        }
        returnVal = true;

        // do
        // TODO

    } while(0);

    return returnVal;
}

static bool cmd_handler_RESET(uint8_t* cmd, uint8_t cmdLen) {
    return true; // TODO
}

static bool cmd_handler_GPIOCAL(uint8_t* cmd, uint8_t cmdLen){
    return true; // TODO
}

//=== hdlc

void openhdlc_rx(uint8_t* rxFrame, uint8_t rxFrameLen) {
    
    bool isValidCommand;

    do {
        
        // abort if no space for type field
        if (rxFrameLen<1) {
            break;
        }

        switch(rxFrame[0]) {
            case CMD_CLEAR:
                isValidCommand = cmd_handler_CLEAR(&rxFrame[1],rxFrameLen-1);
                if (isValidCommand==true) {app_dbg.num_command_CLEAR++;}
                break;
            case CMD_CHUNK:
                isValidCommand = cmd_handler_CHUNK(&rxFrame[1],rxFrameLen-1);
                if (isValidCommand==true) {app_dbg.num_command_CHUNK++;}
                break;
            case CMD_LOAD:
                isValidCommand = cmd_handler_LOAD(&rxFrame[1],rxFrameLen-1);
                if (isValidCommand==true) {app_dbg.num_command_LOAD++;}
                break;
            case CMD_RESET:
                isValidCommand = cmd_handler_RESET(&rxFrame[1],rxFrameLen-1);
                if (isValidCommand==true) {app_dbg.num_command_RESET++;}
                break;
            case CMD_GPIOCAL:
                isValidCommand = cmd_handler_GPIOCAL(&rxFrame[1],rxFrameLen-1);
                if (isValidCommand==true) {app_dbg.num_command_GPIOCAL++;}
                break;
        }
    } while(0);
    
    if (isValidCommand==false) {
        // debug
        app_dbg.num_command_invalid++;
    }
}

//=== bsp

void lfxtal_start(void) {
    
    // start 32 kHz XTAL
    NRF_CLOCK->LFCLKSRC                = 0x00000001; // 1==XTAL
    NRF_CLOCK->EVENTS_LFCLKSTARTED     = 0;
    NRF_CLOCK->TASKS_LFCLKSTART        = 0x00000001;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
}

void hfclock_start(void) {
    
    NRF_CLOCK->EVENTS_HFCLKSTARTED     = 0;
    NRF_CLOCK->TASKS_HFCLKSTART        = 0x00000001;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}

void led_enable(void) {
    // do after LF XTAL started

    // enable all LEDs
    NRF_P0->PIN_CNF[13]                = 0x00000003;            // LED 1
    NRF_P0->PIN_CNF[14]                = 0x00000003;            // LED 2
    NRF_P0->PIN_CNF[15]                = 0x00000003;            // LED 3
    NRF_P0->PIN_CNF[16]                = 0x00000003;            // LED 4
    
    // configure RTC0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // xxxx xxxx xxxx FEDC xxxx xxxx xxxx xxBA (C=compare 0)
    // 0000 0000 0000 0001 0000 0000 0000 0000 
    //    0    0    0    1    0    0    0    0 0x00010000
    NRF_RTC0->EVTENSET                 = 0x00010000;       // enable compare 0 event routing
    NRF_RTC0->INTENSET                 = 0x00010000;       // enable compare 0 interrupts

    // enable interrupts
    NVIC_SetPriority(RTC0_IRQn, 1);
    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NVIC_EnableIRQ(RTC0_IRQn);
    
    //
    NRF_RTC0->CC[0]                    = (32768>>3);       // (32768>>3) = 125 ms
    NRF_RTC0->TASKS_START              = 0x00000001;       // start RTC0
}

void led_advance(void) {
    
    // bump
    app_vars.led_counter               = (app_vars.led_counter+1)%NUM_LEDS;

    // apply
    NRF_P0->OUTSET                     = (0x00000001 << 13);
    NRF_P0->OUTSET                     = (0x00000001 << 14);
    NRF_P0->OUTSET                     = (0x00000001 << 15);
    NRF_P0->OUTSET                     = (0x00000001 << 16);
    switch (app_vars.led_counter) {
        case 0: NRF_P0->OUTCLR         = (0x00000001 << 13); break; // LED 1
        case 1: NRF_P0->OUTCLR         = (0x00000001 << 14); break; // LED 2
        case 2: NRF_P0->OUTCLR         = (0x00000001 << 16); break; // LED 4
        case 3: NRF_P0->OUTCLR         = (0x00000001 << 15); break; // LED 3
    }
}

void uart1_init(void) {
    // do after HFCLOCK started

    //=== UART1 to SCuM

    // TX: P0.26
    // RX: P0.02
    // FTDI cable:
    //     - black  GND
    //     - Orange TXD
    //     - yellow RXD
    NRF_UARTE1->RXD.PTR                = (uint32_t)app_vars.uart_buf_SCuM_RX;
    NRF_UARTE1->RXD.MAXCNT             = UART_BUF_SIZE;
    NRF_UARTE1->TXD.PTR                = (uint32_t)app_vars.uart_buf_SCuM_TX;
    NRF_UARTE1->TXD.MAXCNT             = UART_BUF_SIZE;
    NRF_UARTE1->PSEL.TXD               = 0x0000001a; // 0x0000001a==P0.26
    NRF_UARTE1->PSEL.RXD               = 0x00000002; // 0x00000002==P0.02
    NRF_UARTE1->CONFIG                 = 0x00000000; // 0x00000000==no flow control, no parity bits, 1 stop bit
    NRF_UARTE1->BAUDRATE               = 0x004EA000; // 0x004EA000==19200 baud (actual rate: 19208)
    NRF_UARTE1->TASKS_STARTRX          = 0x00000001; // 0x00000001==start RX state machine; read received byte from RXD register
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: CTS
    // .... .... .... .... .... .... .... ..B. B: NCTS
    // .... .... .... .... .... .... .... .C.. C: RXDRDY
    // .... .... .... .... .... .... ...D .... D: ENDRX
    // .... .... .... .... .... .... E... .... E: TXDRDY
    // .... .... .... .... .... ...F .... .... F: ENDTX
    // .... .... .... .... .... ..G. .... .... G: ERROR
    // .... .... .... ..H. .... .... .... .... H: RXTO
    // .... .... .... I... .... .... .... .... I: RXSTARTED
    // .... .... ...J .... .... .... .... .... J: TXSTARTED
    // .... .... .L.. .... .... .... .... .... L: TXSTOPPED
    // xxxx xxxx x0x0 0x0x xxxx xx00 0xx1 x000 
    //    0    0    0    0    0    0    1    0 0x00000010
    NRF_UARTE1->INTENSET               = 0x00000010;
    NRF_UARTE1->ENABLE                 = 0x00000008; // 0x00000008==enable
    
    // enable interrupts
    NVIC_SetPriority(UARTE1_IRQn, 1);
    NVIC_ClearPendingIRQ(UARTE1_IRQn);
    NVIC_EnableIRQ(UARTE1_IRQn);
}

//=========================== interrupt handlers ==============================

void RTC0_IRQHandler(void) {
    uint8_t dummy[3] = {1,2,3};

    // debug
    app_dbg.num_ISR_RTC0_IRQHandler++;

    // handle compare[0]
    if (NRF_RTC0->EVENTS_COMPARE[0] == 0x00000001 ) {
        
        // clear flag
        NRF_RTC0->EVENTS_COMPARE[0]    = 0x00000000;

        // clear COUNTER
        NRF_RTC0->TASKS_CLEAR          = 0x00000001;

        // debug
        app_dbg.num_ISR_RTC0_IRQHandler_COMPARE0++;

        // handle
        led_advance();
        //openhdlc_send(dummy,3);
     }
}

void UARTE1_IRQHandler(void) {

    // debug
    app_dbg.num_ISR_UARTE1_IRQHandler++;

    if (NRF_UARTE1->EVENTS_ENDRX == 0x00000001) {
        // byte received from SCuM

        // clear
        NRF_UARTE1->EVENTS_ENDRX = 0x00000000;

        // debug
        app_dbg.num_ISR_UARTE1_IRQHandler_ENDRX++;

        // send byte to DK
        app_vars.uart_buf_DK_TX[0] = app_vars.uart_buf_SCuM_RX[0];

        // start sending
        NRF_UARTE0->EVENTS_TXSTARTED = 0x00000000;
        NRF_UARTE0->TASKS_STARTTX = 0x00000001;
        while (NRF_UARTE0->EVENTS_TXSTARTED == 0x00000000);
    }
}
