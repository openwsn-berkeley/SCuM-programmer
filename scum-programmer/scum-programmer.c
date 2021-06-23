/**
SCuM programmer.
*/

#include "nrf52840.h"

//=========================== defines =========================================

const uint8_t APP_VERSION[]         = {0x00,0x01};

//=========================== variables =======================================


typedef struct {
    uint32_t       dummy;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t       num_task_loops;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== main ============================================

int main(void) {
    
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

//=========================== bsp =============================================


//=========================== interrupt handlers ==============================

