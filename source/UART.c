//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
	\file		UART.c
	\author		Jorge Rubén Curiel Anaya
	\date		October 24th, 2019
	\brief		Implementation of the UART driver functions
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "MKL27Z644.h"
#include "UART.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
/*!
    \def	MCGIRCLK_CLK
    \brief	Select the MCGIRCLK option with a 0b11 (0d3)
*/
#define MCGIRCLK_CLK 		3

/*!
 	 \def	RDRF_MASK
 	 \brief	Mask to read the value in the Receive Data Register Full Flag
 */
#define RDRF_MASK			(1<<21)

/*!
 	 \def	TDRE_MASK
 	 \brief	Mask to read the value in the Transmit Data Register Empty Flag
 */
#define TDRE_MASK			(1<<23)

/*!
 	 \def	RAF_MASK
 	 \brief	Mask to read the value in the Receiver Active Flag
 */
#define RAF_MASK			(1<<24)

/*!
 	 \def	DATA_READ_MASK
 	 \brief	Mask to read the value in the DATA buffer
 */
#define DATA_READ_MASK		0xFF

/*!
	\def	SBR
	\brief	Set the value of SBR to 52
 */
#define SBR					0x34

/*!
 	 \def	SBR_MASK
 	 \brief	Mask for the [12:0] bits of SBR in LPUART0->BAUD
 */
#define SBR_MASK			0x1FFF

/*!
 	 \def	PORTA_MASK
 	 \brief	Mask to acces the PORTA clock enable bit
 */
#define PORTA_MASK			(1<<9)

/*!
 	 \def	PORT_ALT2
 	 \brief	Mask to acces the PORTA clock enable bit
 */
#define PORT_ALT2			(1<<1)

/*!
 	 \def	LPUART0_CTRL_TE
 	 \brief	Mask to set the transmit enable bit
 */
#define LPUART0_CTRL_TE		(1<<19)

/*!
 	 \def	LPUART0_CTRL_RE
 	 \brief	Mask to set the receiver enable bit
 */
#define LPUART0_CTRL_RE		(1<<18)

/*!
 	 \def	SBNS_MASK
 	 \brief	Mask to set the receiver enable bit
 */
#define SBNS_MASK			(1<<13)
//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------
/*!
    \var
    \brief
*/

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
/*!
    \fn				void UART_vfnDriverInit(void)
    \param			void No params
    \return			No returns
    \brief			Initialize the required elements for LPUART0 to work
*/
void UART_vfnDriverInit(void)
{
	// Select the MCGIRCLK clock signal for the LPUART0 peripheral
	SIM->SOPT2 |= SIM_SOPT2_LPUART0SRC(MCGIRCLK_CLK);

	// Activate LPUART0 in SIM->SCGC5
	SIM->SCGC5 |= SIM_SCGC5_LPUART0(1);

	// Enable the 8MHz clock and set the required LIRC configurations in order to function properly
	MCG->C1 |= MCG_C1_CLKS(1);
	MCG->C1 |= MCG_C1_IRCLKEN(1);
	MCG->C2 |= MCG_C2_IRCS(1);

	// Set the value of SBR[12:0] to 52
	LPUART0->BAUD = (LPUART0->BAUD & (~SBR_MASK)) | SBR;

	// Set the Receiver enable bit
	LPUART0->CTRL |= LPUART0_CTRL_RE;

	// Set the Transmit enable bit
	LPUART0->CTRL |= LPUART0_CTRL_TE;

	// Set the SBNS bit for two stop bits configuration
	LPUART0->BAUD |= SBNS_MASK;

	// Set the UART pins as:
	//		PTA1 -> LPUART0_RX
	//		PTA2 -> LPUART0_TX
	SIM->SCGC5 |= PORTA_MASK;
	PORTA->PCR[1] |= PORT_PCR_MUX(PORT_ALT2);
	PORTA->PCR[2] |= PORT_PCR_MUX(PORT_ALT2);
}

/*!
    \fn			uint8_t UART_bfnRead(uint8_t *readVal)
    \param		readVal Variable pointer where the read value by the UART buffer will be stored
    \return		If reading the value was successful, returns True (1); else, returns False (0)
    \brief		Read and store the value from the DATA buffer if a valid packet was read from the UART communication
*/
uint8_t UART_bfnRead(uint8_t *readVal)
{
	if((LPUART0->STAT & RDRF_MASK) && !(LPUART0->STAT & RAF_MASK))
	{
		*readVal = (LPUART0->DATA & DATA_READ_MASK);
		return 1;
	}
	else
	{
		return 0;
	}
}

/*!
    \fn			uint8_t UART_bfnSend(uint8_t *sendVal)
    \param		sendVal	Variable pointer where the value to be sent through UART is stored
    \return		If sending the value was successful, returns True (1); else, returns False (0)
    \brief		Send a byte through UART communication if the DATA buffer is available
*/
uint8_t UART_bfnSend(uint8_t *sendVal)
{
	if((LPUART0->STAT & TDRE_MASK) && !(LPUART0->STAT & RAF_MASK))
	{
		LPUART0->DATA |= *sendVal;
		return 1;
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------------------------------
