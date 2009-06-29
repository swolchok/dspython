/*---------------------------------------------------------------------------------
	$Id: clock.c,v 1.5 2007/02/19 01:28:13 wntrmute Exp $


	Copyright (C) 2005
		Michael Noland (Joat)
		Jason Rogers (Dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
			must not claim that you wrote the original software. If you use
			this software in a product, an acknowledgment in the product
			documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
			must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
			distribution.

	$Log: clock.c,v $
	Revision 1.5  2007/02/19 01:28:13  wntrmute
	extend RTC functions
	
	Revision 1.4  2005/10/21 22:43:06  wntrmute
	Removed bogus ASSERT
	
	Revision 1.3  2005/09/20 05:05:52  wntrmute
	added header logging
	tidied formatting
	

---------------------------------------------------------------------------------*/

#include "nds/bios.h"
#include "nds/arm7/clock.h"


// Delay (in swiDelay units) for each bit transfer
#define RTC_DELAY 48

// Pin defines on RTC_CR
#define CS_0    (1<<6)
#define CS_1    ((1<<6) | (1<<2))
#define SCK_0   (1<<5)
#define SCK_1   ((1<<5) | (1<<1))
#define SIO_0   (1<<4)
#define SIO_1   ((1<<4) | (1<<0))
#define SIO_out (1<<4)
#define SIO_in  (1)

//---------------------------------------------------------------------------------
void BCDToInteger(uint8 * data, uint32 length) {
//---------------------------------------------------------------------------------
	u32 i;
	for (i = 0; i < length; i++) {
		data[i] = (data[i] & 0xF) + ((data[i] & 0xF0)>>4)*10;
	}
}


//---------------------------------------------------------------------------------
void integerToBCD(uint8 * data, uint32 length) {
//---------------------------------------------------------------------------------
	u32 i;
	for (i = 0; i < length; i++) {
		int high, low;
		swiDivMod(data[i], 10, &high, &low);
		data[i] = (high<<4) | low;
	}
}

//---------------------------------------------------------------------------------
void rtcTransaction(uint8 * command, uint32 commandLength, uint8 * result, uint32 resultLength) {
//---------------------------------------------------------------------------------
	uint32 bit;
	uint8 data;

	// Raise CS
	RTC_CR8 = CS_0 | SCK_1 | SIO_1;
	swiDelay(RTC_DELAY);
	RTC_CR8 = CS_1 | SCK_1 | SIO_1;
	swiDelay(RTC_DELAY);

	// Write command byte (high bit first)
		data = *command++;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data>>7);
			swiDelay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data>>7);
			swiDelay(RTC_DELAY);

			data = data << 1;
		}
	// Write parameter bytes (low bit first)
	for ( ; commandLength > 1; commandLength--) {
		data = *command++;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data & 1);
			swiDelay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data & 1);
			swiDelay(RTC_DELAY);

			data = data >> 1;
		}
	}

	// Read result bytes (low bit first)
	for ( ; resultLength > 0; resultLength--) {
		data = 0;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0;
			swiDelay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1;
			swiDelay(RTC_DELAY);

			if (RTC_CR8 & SIO_in) data |= (1 << bit);
		}
		*result++ = data;
	}

	// Finish up by dropping CS low
	RTC_CR8 = CS_0 | SCK_1;
	swiDelay(RTC_DELAY);
}


//---------------------------------------------------------------------------------
void rtcReset(void) {
//---------------------------------------------------------------------------------
	uint8 status;
	uint8 command[2];

	// Read the first status register
	command[0] = READ_STATUS_REG1;
	rtcTransaction(command, 1, &status, 1);

	// Reset the RTC if needed
	if (status & (STATUS_POC | STATUS_BLD)) {
		command[0] = WRITE_STATUS_REG1;
		command[1] = status | STATUS_RESET;
		rtcTransaction(command, 2, 0, 0);
	}
}


//---------------------------------------------------------------------------------
void rtcGetTimeAndDate(uint8 * time) {
//---------------------------------------------------------------------------------
	uint8 command, status;

	command = READ_TIME_AND_DATE;
	rtcTransaction(&command, 1, time, 7);

	command = READ_STATUS_REG1;
	rtcTransaction(&command, 1, &status, 1);

	if ( status & STATUS_24HRS ) {
		time[4] &= 0x3f;
	} else {

	}
	BCDToInteger(time,7);
}

//---------------------------------------------------------------------------------
void rtcSetTimeAndDate(uint8 * time) {
//---------------------------------------------------------------------------------
	uint8 command[8];
	
	int i;
	for ( i=0; i< 8; i++ ) {
		command[i+1] = time[i];
	}
	command[0] = WRITE_TIME_AND_DATE;
	// fixme: range checking on the data we tell it
	rtcTransaction(command, 8, 0, 0);
}

//---------------------------------------------------------------------------------
void rtcGetTime(uint8 * time) {
//---------------------------------------------------------------------------------
	uint8 command, status;

	command = READ_TIME;
	rtcTransaction(&command, 1, time, 3);

	command = READ_STATUS_REG1;
	rtcTransaction(&command, 1, &status, 1);
	if ( status & STATUS_24HRS ) {
		time[0] &= 0x3f;
	} else {

	}
	BCDToInteger(time,3);

}

//---------------------------------------------------------------------------------
void rtcSetTime(uint8 * time) {
//---------------------------------------------------------------------------------
	uint8 command[4];
	
	int i;
	for ( i=0; i< 3; i++ ) {
		command[i+1] = time[i];
	}
	command[0] = WRITE_TIME;
	// fixme: range checking on the data we tell it
	rtcTransaction(command, 4, 0, 0);
}




