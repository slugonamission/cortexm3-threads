#include "usbcon.h"

#include <string.h>
#include <lpc17xx_uart.h>
#include <lpc17xx_pinsel.h>
#include <lpc_types.h>

// Read options
int usbcon_read(char *buf,int length)
{
    return(UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)buf, length, NONE_BLOCKING));
}

// Write options
int usbcon_write(char *buf)
{
    int length = strlen(buf);
    return(UART_Send((LPC_UART_TypeDef *)LPC_UART0,(uint8_t *)buf,length, BLOCKING));
}
// init code for the USB serial line
void usbcon_init(void)
{
    UART_CFG_Type UARTConfigStruct;// UART Configuration structure variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;// UART FIFO configuration Struct variable
    PINSEL_CFG_Type PinCfg;// Pin configuration for UART
    /*
     * Initialize UART pin connect
     */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    // USB serial first
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 3;
    PINSEL_ConfigPin(&PinCfg);
    
    /* Initialize UART Configuration parameter structure to default state:
     * - Baudrate = 9600bps
     * - 8 data bit
     * - 1 Stop bit
     * - None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);
    /* Initialize FIFOConfigStruct to default state:
     * - FIFO_DMAMode = DISABLE
     * - FIFO_Level = UART_FIFO_TRGLEV0
     * - FIFO_ResetRxBuf = ENABLE
     * - FIFO_ResetTxBuf = ENABLE
     * - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
    // Built the basic structures, lets start the devices/
    // USB serial
    UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);// Initialize UART0 peripheral with given to corresponding parameter
    UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);// Initialize FIFO for UART0 peripheral
    UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);// Enable UART Transmit
    
}
