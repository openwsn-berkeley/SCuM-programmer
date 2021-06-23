/**
SCuM programmer.
*/

#include "nrf52840.h"

//=========================== defines =========================================

const uint8_t APP_VERSION[]         = {0x00,0x01};

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
void led_enable(void);

//=========================== variables =======================================

typedef struct {
    uint32_t       led_counter;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t       num_task_loops;
    uint32_t       num_ISR_RTC0_IRQHandler;
    uint32_t       num_ISR_RTC0_IRQHandler_COMPARE0;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== main ============================================

int main(void) {
    
    // main loop
    while(1) {
        
        // bsp
        lfxtal_start();
        led_enable();

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

void led_enable(void) {
    
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