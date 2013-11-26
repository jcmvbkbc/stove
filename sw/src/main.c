#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#define TERM_DDR  DDRB
#define TERM_PORT PORTB
#define TERM_BIT  PB0

#define HEAT_DDR  DDRC
#define HEAT_PORT PORTC
#define HEAT_BIT  PB5

int main() {
	TERM_DDR |= _BV(TERM_BIT);
	HEAT_DDR |= _BV(HEAT_BIT);

	while(1) {
		;
	}
}