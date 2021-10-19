
#include "openhdlc.h"

//=========================== variables =======================================

openserial_vars_t openserial_vars;

//=========================== prototypes ======================================

// HDLC output
void     outputHdlcOpen(void);
void     outputHdlcWrite(uint8_t b);
void     outputHdlcClose(void);

// HDLC input
void     uart_rxByte(uint8_t rxbyte);
void     inputHdlcOpen(void);
void     inputHdlcWrite(uint8_t b);
void     inputHdlcClose(void);

// helpers
uint16_t crcIteration(uint16_t crc, uint8_t byte);
void     uart_txByte(uint8_t byte);

//=========================== public ==========================================

void openhdlc_init(openhdlc_rx_cbt openhdlc_rx_cb) {

    // initialize variables
    memset(&openserial_vars,0x00,sizeof(openserial_vars_t));
    openserial_vars.openhdlc_rx_cb = openhdlc_rx_cb;

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
    NRF_UART0->BAUDRATE                = 0x01D7E000; // 0x01D7E000==115200 baud (actual rate: 115942)
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
    
    outputHdlcOpen();
    for (i = 0; i < txBufLen; i++) {
        outputHdlcWrite(txBuf[i]);
    }
    outputHdlcClose();
}

//=========================== private =========================================

//=== hdlc output

void outputHdlcOpen(void) {

    // initialize the value of the CRC
    openserial_vars.hdlcOutputCrc = HDLC_CRCINIT;

    // write the opening HDLC flag
    uart_txByte(HDLC_FLAG);
}

void outputHdlcWrite(uint8_t b) {
    
    // iterate through CRC calculator
    openserial_vars.hdlcOutputCrc = crcIteration(openserial_vars.hdlcOutputCrc, b);

    // add byte to buffer
    if (b == HDLC_FLAG || b == HDLC_ESCAPE) {
        uart_txByte(HDLC_ESCAPE);
        b = b ^ HDLC_ESCAPE_MASK;
    }
    uart_txByte(b);
}

void outputHdlcClose(void) {
    uint16_t finalCrc;

    // finalize the calculation of the CRC
    finalCrc = ~openserial_vars.hdlcOutputCrc;

    // write the CRC value
    outputHdlcWrite((finalCrc >> 0) & 0xff);
    outputHdlcWrite((finalCrc >> 8) & 0xff);

    // write the closing HDLC flag
    uart_txByte(HDLC_FLAG);
}

//=== hdlc input

void uart_rxByte(uint8_t rxbyte) {
    if (
        openserial_vars.hdlcBusyReceiving == false &&
        openserial_vars.hdlcLastRxByte == HDLC_FLAG &&
        rxbyte != HDLC_FLAG
    ) {
        // start of frame

        // I'm now receiving
        openserial_vars.hdlcBusyReceiving = true;

        // create the HDLC frame
        inputHdlcOpen();

        // add the byte just received
        inputHdlcWrite(rxbyte);
    } else if (
      openserial_vars.hdlcBusyReceiving == true &&
      rxbyte != HDLC_FLAG
    ) {
        // middle of frame

        // add the byte just received
        inputHdlcWrite(rxbyte);
        if (openserial_vars.inputBufFillLevel + 1 > SERIAL_INPUT_BUFFER_SIZE) {
            // overflow
            
            openserial_vars.inputBufFillLevel = 0;
            openserial_vars.hdlcBusyReceiving = false;
        }
    } else if (
        openserial_vars.hdlcBusyReceiving == true &&
        rxbyte == HDLC_FLAG
    ) {
        // end of frame

        // finalize the HDLC frame
        inputHdlcClose();
        openserial_vars.hdlcBusyReceiving = false;

        if (openserial_vars.inputBufFillLevel == 0) {
            // wrong CRC
            
        } else {
            // good CRC

            openserial_vars.openhdlc_rx_cb(
                openserial_vars.inputBuf,
                openserial_vars.inputBufFillLevel
            );
            openserial_vars.inputBufFillLevel = 0;
        }
    }

    openserial_vars.hdlcLastRxByte = rxbyte;
}

void inputHdlcOpen(void) {
    
    // reset the input buffer index
    openserial_vars.inputBufFillLevel = 0;

    // initialize the value of the CRC
    openserial_vars.hdlcInputCrc = HDLC_CRCINIT;
}


void inputHdlcWrite(uint8_t b) {
   
    if (b == HDLC_ESCAPE) {
        openserial_vars.hdlcInputEscaping = true;
    } else {
        if (openserial_vars.hdlcInputEscaping == true) {
            b = b ^ HDLC_ESCAPE_MASK;
            openserial_vars.hdlcInputEscaping = false;
        }

        // add byte to input buffer
        openserial_vars.inputBuf[openserial_vars.inputBufFillLevel] = b;
        openserial_vars.inputBufFillLevel++;

        // iterate through CRC calculator
        openserial_vars.hdlcInputCrc = crcIteration(openserial_vars.hdlcInputCrc, b);
    }
}

void inputHdlcClose(void) {

    // verify the validity of the frame
    if (openserial_vars.hdlcInputCrc == HDLC_CRCGOOD) {
        // the CRC is correct

        // remove the CRC from the input buffer
        openserial_vars.inputBufFillLevel -= 2;
    } else {
        // the CRC is incorrect

        // drop the incoming frame
        openserial_vars.inputBufFillLevel = 0;
    }
}

//=== helpers

uint16_t crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

void uart_txByte(uint8_t byte) {
    NRF_UART0->EVENTS_TXDRDY = 0x00000000;
    NRF_UART0-> TXD          = byte;
    while(NRF_UART0->EVENTS_TXDRDY == 0x00000000);
}

//=========================== interrupt handlers ==============================

void UARTE0_UART0_IRQHandler(void) {

    if (NRF_UART0->EVENTS_RXDRDY == 0x00000001) {
        // byte received from computer

        // clear
        NRF_UART0->EVENTS_RXDRDY = 0x00000000;

        // read
        uart_rxByte(NRF_UART0->RXD);
    }
}
