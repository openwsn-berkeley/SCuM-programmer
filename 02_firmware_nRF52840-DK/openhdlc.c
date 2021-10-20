
#include "openhdlc.h"

//=========================== variables =======================================

openhdlc_vars_t openhdlc_vars;
openhdlc_dbg_t  openhdlc_dbg;

//=========================== prototypes ======================================

// HDLC output
static void     openhdlc_ouput_open(void);
static void     openhdlc_ouput_write(uint8_t b);
static void     openhdlc_ouput_close(void);
static void     openhdlc_uart_txByte(uint8_t byte);

// HDLC input
static void     openhdlc_uart_rxByte(uint8_t rxbyte);
static void     openhdlc_input_open(void);
static void     openhdlc_input_write(uint8_t b);
static void     openhdlc_input_close(void);

// helpers
static uint16_t openhdlc_crcIteration(uint16_t crc, uint8_t byte);

//=========================== public ==========================================

void openhdlc_init(openhdlc_rx_cbt openhdlc_rx_cb) {

    // initialize variables
    memset(&openhdlc_vars,0x00,sizeof(openhdlc_vars_t));
    memset(&openhdlc_dbg, 0x00,sizeof(openhdlc_dbg_t));
    openhdlc_vars.openhdlc_rx_cb = openhdlc_rx_cb;

    //=== UART0 to computer
    
    // https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/vir_com_port.html
    // P0.05	RTS
    // P0.06	TXD <===
    // P0.07	CTS
    // P0.08	RXD <===
    
    // configure
    NRF_UART0->ENABLE                  = 0x00000004; // 0x00000004==enable
    NRF_UART0->PSEL.TXD                = 0x00000006; // 0x00000006==P0.6
    NRF_UART0->PSEL.RXD                = 0x00000008; // 0x00000008==P0.8
    NRF_UART0->CONFIG                  = 0x00000000; // 0x00000000==no flow control, no parity bits, 1 stop bit
    NRF_UART0->BAUDRATE                = 0x10000000; // 0x10000000==1Mega baud
    NRF_UART0->TASKS_STARTTX           = 0x00000001; // 0x00000001==start TX state machine; write to TXD to send
    NRF_UART0->TASKS_STARTRX           = 0x00000001; // 0x00000001==start RX state machine; read received byte from RXD register
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...0 A: CTS
    // .... .... .... .... .... .... .... ..0. B: NCTS
    // .... .... .... .... .... .... .... .1.. C: RXDRDY
    // .... .... .... .... .... .... 0... .... D: TXDRDY
    // .... .... .... .... .... ..0. .... .... E: ERROR
    // .... .... .... ..0. .... .... .... .... F: RXTO
    // xxxx xxxx xxxx xx0x xxxx xx0x 0xxx x100 
    //    0    0    0    0    0    0    0    4 0x00000004
    NRF_UART0->INTENSET                = 0x00000004;
    
    // enable interrupts
    NVIC_SetPriority(UARTE0_UART0_IRQn, 1);
    NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
    NVIC_EnableIRQ(UARTE0_UART0_IRQn);
}

void openhdlc_send(uint8_t* txBuf, uint8_t txBufLen) {
    uint8_t i;
    
    // debug
    openhdlc_dbg.num_frames_sent++;

    openhdlc_ouput_open();
    for (i = 0; i < txBufLen; i++) {
        openhdlc_ouput_write(txBuf[i]);
    }
    openhdlc_ouput_close();
}

//=========================== private =========================================

//=== hdlc output

static void openhdlc_ouput_open(void) {

    // initialize the value of the CRC
    openhdlc_vars.hdlcOutputCrc = HDLC_CRCINIT;

    // write the opening HDLC flag
    openhdlc_uart_txByte(HDLC_FLAG);
}

static void openhdlc_ouput_write(uint8_t b) {
    
    // iterate through CRC calculator
    openhdlc_vars.hdlcOutputCrc = openhdlc_crcIteration(openhdlc_vars.hdlcOutputCrc, b);

    // add byte to buffer
    if (b == HDLC_FLAG || b == HDLC_ESCAPE) {
        openhdlc_uart_txByte(HDLC_ESCAPE);
        b = b ^ HDLC_ESCAPE_MASK;
    }
    openhdlc_uart_txByte(b);
}

static void openhdlc_ouput_close(void) {
    uint16_t finalCrc;

    // finalize the calculation of the CRC
    finalCrc = ~openhdlc_vars.hdlcOutputCrc;

    // write the CRC value
    openhdlc_ouput_write((finalCrc >> 0) & 0xff);
    openhdlc_ouput_write((finalCrc >> 8) & 0xff);

    // write the closing HDLC flag
    openhdlc_uart_txByte(HDLC_FLAG);
}

static void openhdlc_uart_txByte(uint8_t byte) {
    
    // debug
    openhdlc_dbg.num_uart_bytes_sent++;
    
    // send
    NRF_UART0->EVENTS_TXDRDY = 0x00000000;
    NRF_UART0->TXD           = byte;
    while(NRF_UART0->EVENTS_TXDRDY == 0x00000000);
}

//=== hdlc input

static void openhdlc_uart_rxByte(uint8_t rxbyte) {
    if (
        openhdlc_vars.hdlcBusyReceiving == false &&
        openhdlc_vars.hdlcLastRxByte == HDLC_FLAG &&
        rxbyte != HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        openhdlc_vars.hdlcBusyReceiving = true;

        // create the HDLC frame
        openhdlc_input_open();

        // add the byte just received
        openhdlc_input_write(rxbyte);
    } else if (
      openhdlc_vars.hdlcBusyReceiving == true &&
      rxbyte != HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        openhdlc_input_write(rxbyte);
        if (openhdlc_vars.inputBufFillLevel + 1 > SERIAL_INPUT_BUFFER_SIZE) {
            // overflow
            
            openhdlc_vars.inputBufFillLevel = 0;
            openhdlc_vars.hdlcBusyReceiving = false;
        }
    } else if (
        openhdlc_vars.hdlcBusyReceiving == true &&
        rxbyte == HDLC_FLAG
    ) {
        // end of frame

        // finalize the HDLC frame
        openhdlc_input_close();
        openhdlc_vars.hdlcBusyReceiving = false;

        if (openhdlc_vars.inputBufFillLevel == 0) {
            // wrong CRC
            
            // debug
            openhdlc_dbg.num_frames_received_error++;

        } else {
            // good CRC

            // debug
            openhdlc_dbg.num_frames_received_ok++;

            // handle
            openhdlc_vars.openhdlc_rx_cb(
                openhdlc_vars.inputBuf,
                openhdlc_vars.inputBufFillLevel
            );

            // reset
            openhdlc_vars.inputBufFillLevel = 0;
        }
    }

    openhdlc_vars.hdlcLastRxByte = rxbyte;
}

static void openhdlc_input_open(void) {
    
    // reset the input buffer index
    openhdlc_vars.inputBufFillLevel = 0;

    // initialize the value of the CRC
    openhdlc_vars.hdlcInputCrc = HDLC_CRCINIT;
}


static void openhdlc_input_write(uint8_t b) {
   
    if (b == HDLC_ESCAPE) {
        openhdlc_vars.hdlcInputEscaping = true;
    } else {
        if (openhdlc_vars.hdlcInputEscaping == true) {
            b = b ^ HDLC_ESCAPE_MASK;
            openhdlc_vars.hdlcInputEscaping = false;
        }

        // add byte to input buffer
        openhdlc_vars.inputBuf[openhdlc_vars.inputBufFillLevel] = b;
        openhdlc_vars.inputBufFillLevel++;

        // iterate through CRC calculator
        openhdlc_vars.hdlcInputCrc = openhdlc_crcIteration(openhdlc_vars.hdlcInputCrc, b);
    }
}

static void openhdlc_input_close(void) {

    // verify the validity of the frame
    if (openhdlc_vars.hdlcInputCrc == HDLC_CRCGOOD) {
        // the CRC is correct

        // remove the CRC from the input buffer
        openhdlc_vars.inputBufFillLevel -= 2;
    } else {
        // the CRC is incorrect

        // drop the incoming frame
        openhdlc_vars.inputBufFillLevel = 0;
    }
}

//=== helpers

static uint16_t openhdlc_crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

//=========================== interrupt handlers ==============================

void UARTE0_UART0_IRQHandler(void) {

    // debug
    openhdlc_dbg.num_ISR_UARTE0_UART0_IRQHandler++;

    if (NRF_UART0->EVENTS_RXDRDY == 0x00000001) {
        // byte received from computer

        // debug
        openhdlc_dbg.num_ISR_UARTE0_UART0_IRQHandler_RXDRDY++;
       
        // clear
        NRF_UART0->EVENTS_RXDRDY = 0x00000000;

        // read
        openhdlc_uart_rxByte(NRF_UART0->RXD);
    }
}
