// JUMA-PA100 protocol
// Juha Niinikoski, OH2NLT 06.07.2006

// Active only if TRX2 protocol is selected 15.11.2008
/*
 The PA-100D Protocol uses the following commands:
 Query Band (Not used)
 The query command from the Juma TRX-2 is: =?B\n\r
 Where n can take the value from 0 - 10
 0 = Out-Of-Band
 1 = 160m
 2 = 80m
 3 = 40m
 4 = 30m
 5 = 20m
 6 = 17m
 7 = 15m
 8 = 12m
 9 = 10m
 10 = Unknown

 Set Band
 The set band command from the Juma TRX-2 is: =Bn\n\r
 Where n has the same meanings as for the query response, except that the only values will be 1 - 9.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "juma-pa100.h"				// Board definitions
#include "pa100_eeprom.h"			// Get EEPROM structure definitions

// External References
extern int not_used;
// External Functions
extern void putch(char);			// UART I/O
extern unsigned char kbhit(void);
extern unsigned char getch(void);
extern void xmit_cmd(const char *);
extern void poll_chk(int);
extern const char *poll_cmd[];

// Global variables
int poll_resp_rec;					// Poll response (freq set) received from TRX-2/KX3, 1 = Received
// Module variables
unsigned char cmd_buf[MAX_BUFFER];	// Command buffer
int cmd_buf_idx = 0;				// Buffer fill pointer

// EEPROM Structures
// User Configuration Data
extern struct
	{
	struct defval defval;
	} eeprom;
/*
 TRX2 protocol parser
 Clear buffer
 This method uses less code and is more efficient than a FOR-NEXT loop by 8 instructions! It clears the buffer, and leaves
 cmd_buf_idx reset to zero when the procedure exits. A.Ryan - 5B4AIY
*/
void clear_buffer(void)
	{
	cmd_buf_idx = MAX_BUFFER;

	do {cmd_buf[--cmd_buf_idx] = 0x00;} while (cmd_buf_idx);
	}

// Parse TRX2 message
void serial_pa100(void)
	{
	char c;

	poll_chk(JUMA_TRX2);					// Check if we need to get the frequency

	if(kbhit())
		{
		c = getch();						// Get next character

//		printf("\n\rCharacter: %c $%2.2X, Index: %i", c, c, cmd_buf_idx);		// test
		if(c == 0x0A) return;				// Ignore Newline

		switch(cmd_buf_idx)
			{
			case 0:							// Possible start of message
				if(c != MSG_START) return;	// If it is not start of message character, ignore.
			break;

			case 1:							// If second character is not a command,
				if(c != 'B')
					{
					cmd_buf_idx = 0;		// then reset index, and return.
					return;
					}
			break;

			case 2:							// If the third character is invalid,
				if(!isdigit(c))
					{
					cmd_buf_idx = 0;		// then reset index, and return.
					return;
					}
			break;

			case 3:
			case 4:
			default:						// If none of the above, then there is something wrong.
				if(c == 0x0D)				// If fourth/fifth character is end-of-message byte,
					{
					cmd_buf[cmd_buf_idx] = c;			// terminate the buffer,
					Current_Band = atoi(cmd_buf + 2);	// and get the new band,

					if(Current_Band < MIN_BAND || Current_Band > MAX_BAND) Current_Band = NOT_KNOWN;	// check the new band for validity, an invalid band inhibits TX,

					poll_resp_rec = TRUE;	// and set the polling response flag.
					}						// If it is not the end-of-message byte,
				clear_buffer();				// and in any event, clear the buffer and restart.
				return;
			break;
			}
		cmd_buf[cmd_buf_idx++] = c;			// Add the character to the buffer.
		}
	}

