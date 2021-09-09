/**
SCuM programmer.
*/

#include "uicr_config.h"
#include "nrf52840.h"

//=========================== defines =========================================

const uint8_t APP_VERSION[]         = {0x00,0x01};

#define UART_BUF_SIZE               1
#define NUM_LEDS                    4

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

void lfxtal_start(void);
void hfclock_start(void);
void led_enable(void);
void led_advance(void);
void uarts_init(void);

//=========================== variables =======================================

typedef struct {
    uint32_t       led_counter;
    uint8_t        uart_buf_DK_RX[UART_BUF_SIZE];
    uint8_t        uart_buf_DK_TX[UART_BUF_SIZE];
    uint8_t        uart_buf_SCuM_RX[UART_BUF_SIZE];
    uint8_t        uart_buf_SCuM_TX[UART_BUF_SIZE];
} app_vars_t;

app_vars_t app_vars;

typedef struct {
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
    
    // main loop
    while(1) {
        
        // bsp
        lfxtal_start();
        hfclock_start();
        led_enable();
        uarts_init();

        // wait for event
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event

        // debug
        app_dbg.num_task_loops++;
    }
}

//=========================== bsp =============================================

//=== lfxtal

void lfxtal_start(void) {
    
    // start 32kHz XTAL
    NRF_CLOCK->LFCLKSRC                = 0x00000001; // 1==XTAL
    NRF_CLOCK->EVENTS_LFCLKSTARTED     = 0;
    NRF_CLOCK->TASKS_LFCLKSTART        = 0x00000001;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);

}

//=== hfclock

void hfclock_start(void) {
    
    NRF_CLOCK->EVENTS_HFCLKSTARTED     = 0;
    NRF_CLOCK->TASKS_HFCLKSTART        = 0x00000001;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}

//=== led

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
    NRF_RTC0->CC[0]                    = (32768>>3);       // 32768>>3 = 125 ms
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

void uarts_init(void) {
    // do after HFCLOCK started

    //=== UART0 to DK
    
    // https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/vir_com_port.html
    // P0.05	RTS
    // P0.06	TXD <===
    // P0.07	CTS
    // P0.08	RXD <===
    
    // configure
    NRF_UARTE0->RXD.PTR                = (uint32_t)app_vars.uart_buf_DK_RX;
    NRF_UARTE0->RXD.MAXCNT             = UART_BUF_SIZE;
    NRF_UARTE0->TXD.PTR                = (uint32_t)app_vars.uart_buf_DK_TX;
    NRF_UARTE0->TXD.MAXCNT             = UART_BUF_SIZE;
    NRF_UARTE0->PSEL.TXD               = 0x00000006; // 0x00000006==P0.6
    NRF_UARTE0->PSEL.RXD               = 0x00000008; // 0x00000008==P0.8
    NRF_UARTE0->CONFIG                 = 0x00000000; // 0x00000000==no flow control, no parity bits, 1 stop bit
    NRF_UARTE0->BAUDRATE               = 0x004EA000; // 0x004EA000==19200 baud (actual rate: 19208)
    NRF_UARTE0->TASKS_STARTRX          = 0x00000001; // 0x00000001==start RX state machine; read received byte from RXD register
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
    NRF_UARTE0->INTENSET               = 0x00000010;
    NRF_UARTE0->ENABLE                 = 0x00000008; // 0x00000008==enable
    
    // enable interrupts
    NVIC_SetPriority(UARTE0_UART0_IRQn, 1);
    NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
    NVIC_EnableIRQ(UARTE0_UART0_IRQn);

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
     }
}

void UARTE0_UART0_IRQHandler(void) {

    // debug
    app_dbg.num_ISR_UARTE0_UART0_IRQHandler++;

    if (NRF_UARTE0->EVENTS_ENDRX == 0x00000001) {
        // byte received from DK

        // clear
        NRF_UARTE0->EVENTS_ENDRX = 0x00000000;

        // debug
        app_dbg.num_ISR_UARTE0_UART0_IRQHandler_ENDRX++;

        // send byte to SCuM
        app_vars.uart_buf_SCuM_TX[0] = app_vars.uart_buf_DK_RX[0];

        // start sending
        NRF_UARTE1->EVENTS_TXSTARTED = 0x00000000;
        NRF_UARTE1->TASKS_STARTTX = 0x00000001;
        while (NRF_UARTE1->EVENTS_TXSTARTED == 0x00000000);
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
