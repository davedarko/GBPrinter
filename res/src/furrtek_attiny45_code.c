/*
GBLink/PC interface v1.1
CC BY furrtek 2010
http://www.furrtek.org
Attiny45 @ 8Mhz
*/

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>

// Pin defs relative to gb (master) serial port:
#define		SCK	0
#define		SOUT	1
#define		SIN	2

/*
    __ __
  +|     |+
RX | t45 |SIN
TX |     |SOUT
GND|_____|SCK

*/

#define		RX	_BV(PB3)
#define		TX	_BV(PB4)

// Super UART software à 57600bps
void SerialPut(uint8_t byte) {
	PORTB &= ~TX;		// Start
	_delay_us(13);
	for (uint8_t bit=0x00;bit<8;bit++) {
		if ((byte >> bit) & 1)
			PORTB |= TX;
		else
			PORTB &= ~TX;
		_delay_us(14);
	}
	PORTB |= TX;		// Stop
}

uint8_t SerialGet(void) {
	uint8_t rreply=0;
	while(bit_is_set(PINB,3)) {}
	_delay_us(18);
	for (uint8_t bit=0x00;bit<8;bit++) {
		rreply = rreply>>1;
		if (bit_is_set(PINB,3)) rreply |= 0x80;
		_delay_us(18);
	}
	return rreply;
}

uint8_t GBSerialOut(uint8_t command) {
	uint8_t clock,reply=0;
	for (clock=0;clock<8;clock++) {
		reply = reply << 1;			// Shifte la réponse
		PORTB &= ~_BV(SCK);
		if ((command << clock) & 0x80) {
			PORTB |= _BV(SOUT);
		} else {
			PORTB &= ~_BV(SOUT);
		}
		_delay_us(20);				// Overclocked, 60
		PORTB |= _BV(SCK);
		if (bit_is_set(PINB,SIN)) reply |= 1;
		_delay_us(20);				// Overclocked, 60
	}
	_delay_us(20);
	return reply;
}

int main(void) {
	uint8_t c,b,cmd,repl,buffer[64];
	uint16_t CRC;

	WDTCR = (1<<WDCE) | (1<<WDE);	// Kick doggie
	WDTCR = 0x00;

	DDRB = 0b00010011;				// TX, SCK et SOUT en outputs
	PORTB = 0b00010000;				// Pull-Up sur TX

	while(1) {
		cmd = SerialGet();

		if (cmd == '?') {
			GBSerialOut(0x88);
			GBSerialOut(0x33);
			GBSerialOut(0x01);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x01);
			GBSerialOut(0x00);
			repl = GBSerialOut(0);
			GBSerialOut(0);
			if (repl == 129) {
				SerialPut('1');
			} else {
				SerialPut('0');
			}
		}

		if (cmd == '!') {
			GBSerialOut(0x88);
			GBSerialOut(0x33);
			GBSerialOut(0x01);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x01);
			GBSerialOut(0x00);
			repl = GBSerialOut(0);
			//SerialPut(repl);
			repl = GBSerialOut(0);
			//SerialPut(repl);

			GBSerialOut(0x88);
			GBSerialOut(0x33);
			GBSerialOut(0x04);
			GBSerialOut(0x00);
			GBSerialOut(0x80);
			GBSerialOut(0x02);
			CRC = 0x86;

			for (b=0;b<10;b++) {
				SerialPut('a');

				for (c=0;c<64;c++) {
					buffer[c] = SerialGet();
				}

				for (c=0;c<64;c++) {
					cmd = buffer[c];
					GBSerialOut(cmd);
					CRC += cmd;
				}
			}

			GBSerialOut((uint8_t)CRC & 0xFF);			// CRC Low
			GBSerialOut((CRC & 0xFF00)>>8);				// CRC High
			repl = GBSerialOut(0);
			//SerialPut(repl);
			repl = GBSerialOut(0);
			//SerialPut(repl);



			_delay_ms(10); 		// 131: répond 4 au lieu de 8, 265 timeout (0)

			GBSerialOut(0x88);	// Data vide
			GBSerialOut(0x33);
			GBSerialOut(0x04);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x00);
			GBSerialOut(0x04);
			GBSerialOut(0x00);
			repl = GBSerialOut(0);
			//SerialPut(repl);
			repl = GBSerialOut(0);
			//SerialPut(repl);

			GBSerialOut(0x88);	// Print
			GBSerialOut(0x33);
			GBSerialOut(0x02);
			GBSerialOut(0x00);
			GBSerialOut(0x04);
			GBSerialOut(0x00);

			CRC = 6;
			SerialPut('1');
			for (c=0;c<4;c++) {
				buffer[c] = SerialGet();
			}

			for (c=0;c<4;c++) {
				cmd = buffer[c];
				GBSerialOut(cmd);
				CRC += cmd;
			}

			GBSerialOut((uint8_t)CRC & 0xFF);			// CRC Low
			GBSerialOut((CRC & 0xFF00)>>8);				// CRC High

			repl = GBSerialOut(0);
			//SerialPut(repl);
			repl = GBSerialOut(0);
			//SerialPut(repl);

			while(1) {
				GBSerialOut(0x88);	// Status
				GBSerialOut(0x33);
				GBSerialOut(0x0F);
				GBSerialOut(0x00);
				GBSerialOut(0x00);
				GBSerialOut(0x00);
				GBSerialOut(0x0F);
				GBSerialOut(0x00);
				GBSerialOut(0);
				if (GBSerialOut(0) & 2) break;
				_delay_ms(100);
			}
			SerialPut('P');

			while(1) {
				GBSerialOut(0x88);	// Status
				GBSerialOut(0x33);
				GBSerialOut(0x0F);
				GBSerialOut(0x00);
				GBSerialOut(0x00);
				GBSerialOut(0x00);
				GBSerialOut(0x0F);
				GBSerialOut(0x00);
				GBSerialOut(0);
				if (!(GBSerialOut(0) & 2)) break;
				_delay_ms(100);
			}
			SerialPut('O');

		}

	}

    return 0;
}