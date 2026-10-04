// JUMA-TRX2 SPI 1 driver for 74HC595 ahift registers
// Two 74HC595 can be connected in cascade
// Juha Niinikoski, OH2NLT 03.12.2006
 
// MPLAB C30 version

// MCU dsPIC30F6014A
// Quiet bus test to reduce main board interference 23.12.2006
// Relocated local static variable old_data to exchg_data_spi1().
// Removed local variables spi_rx_data never used. A.Ryan - 5B4AIY - 3/JAN/2013
/*
 NOTE: The SPI bus is not used in the PA100D, it is there for future expansion. This module has been removed from the
 build list, and is only included in the source files in case you should decide to use the SPI bus with modified
 hardware. A.Ryan - 5B4AIY - 09/JAN/2014
 SPI Bus Operation (Juma TRX2)
 The Serial Peripheral Interchange bus is an intrinsic part of the dsPIC30F6014 microprocessor. The chip is equipped
 with two such busses, only SPI1 is used in the Juma TRX-2. For full details of the theory of operation of this
 interface bus, please refer to the Microchip dsPIC30F6014 data sheet. The following is an abreviated description.
 The SPI bus is controlled by means of configuration settings strobed into the SPI1CON status register. In this
 application the bus is configured as a Master, meaning that it generates the necessary clock and latch strobe signals
 for the peripheral device. The control register is set to deliver 16 bits of serial data from the SPI1SR to the
 SPI1DO pin. This occurs whenever data is sent to the SPI1BUF register. At the conclusion of this operation, the SPI
 interrupt is generated, and the interrupt code is required to deliver a latch pulse to the peripheral, as well as
 read the SPI1BUF input register. In this application, no actual data is input to the microprocessor, but to ensure
 that further data can be transmitted, this dummy read operation has to be performed in order to clear the SPI1POV
 flag. The SPI1DO pin is connected to the data input (Pin 14) of IC6 on the main board, a 74HC595 serial to parallel
 convertor chip. The serial data output (Pin 9 - Cascade) is connected to pin 14 of IC4 on the RF Filter board. The 16
 data bits are clocked by the inverted output of the SPI1_CLK which is itself configured to deliver negative going
 clock pulses via the SPI1CON register during the initialisation phase. (See comments of init_spi1()) At the conclusion
 the first 8 bits will have been clocked through to IC4 to set the band select filter, the second 8 bits will have been
 clocked into IC6, the control register to select filter width, AGC speed, MIC/LINE, USB/LSB/CW/Tune etc. Also at the
 conclusion the SPI1 interrupt will ocuur, and the SPI1 interrupt now generates the latch pulse to latch the clocked
 data into the output registers of IC6 and IC4, as well as clear the SPI1OV flag by reading SPI1BUF in preparation for
 the next data output, and finally resetting the IRQ flag. The data is only output if there has been a change, this
 minimises the audio interference that would result from a continuous clocking of the SPI bus. A.Ryan - 5B4AIY - 09/JAN/2014
*/

#include "juma-pa100.h"				// JUMA-TRX2 hardware specific definitions

#define SPI1_LATCH _LATB2			// Strobe signal in I/O pins

// Init SPI 1 system
// SPI1CON = 0x0439
// 04 = Set Bit 10, 16 bit operation
// 39 = Set Bit 5, Master Enable, Bit 4 SPRE2, Bit 3, SPRE1, Bit 0, PPRE0
// SPI1STAT = 0x8000 Set Bit 15, SPI1 Enable

void init_spi1(void)
	{
	SPI1_LATCH = 0;					// Set Slave Latch to idle state, low
	SPI1CON = 0x0439;				// Bit clock falling edge, clock/32
	SPI1STAT = 0x8000;				// SPI1 on
	_SPI1IF = 0;					// Clear IRQ flag
	_SPI1IE = 1;					// Enable SPI TX ready IRQ
	}

// Send data
void exchg_data_spi1(unsigned int data)
	{
	static unsigned int old_data;

	if(data != old_data)			// Transmit only if data changed
		{
    	while(SPI1STATbits.SPITBF);	// Wait until 'SPITBF' bit is cleared

		SPI1BUF = data;
		old_data = data;
		}
	}

// TX ready, generate latch pulse
void __attribute__((interrupt, auto_psv)) _SPI1Interrupt(void)
	{
	unsigned int spi_rx_data;

	SPI1_LATCH = 1;					// Generate latch strobe pulse __-__, rising edge sensitive

	if(SPI1STATbits.SPIRBF) spi_rx_data = SPI1BUF;		// Dummy read RX data, to clear internal SPI1OV flag

	SPI1_LATCH = 0;					// Back to idle state
	_SPI1IF = 0;					// Clear IRQ flag
	}

