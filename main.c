#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

void TWI_START(unsigned char bitrate);
void TWI_ADDRESS(unsigned char address);
void MASTER_RECEIVE(uint8_t datasize);
void TWI_STOP(void);

unsigned char data[] = {}; 							///< Holds the final data received from slave device.

int main(void){
	DDRA = 0xFF;
	
	TWI_START(0x80);
	TWI_ADDRESS(0xA0);
	MASTER_RECEIVE(12);
	for(uint8_t i=0;data[i]!=0;i++){
		PORTA = data[i];
		_delay_ms(500);
	}
}

/*!
 *	@brief Send START condition to confirm master status in current iteration.
 *	@param Bitrate of operation to determine SCL period (unsigned char).
 */

void TWI_START(unsigned char bitrate){
	TWBR = 0x00;
	TWBR = bitrate; 							///< Set the division factor of bitrate generator.
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTA); 				///< Enable TWI and set TWINT bit to 1. Send START condition.
	while(!(TWCR & (1<<TWINT)));						///< Wait while TWI completes current action.
	while((TWSR & 0xF8) != 0x08); 						///< Wait while ACK bit is received after START condition is transmitted.
}

/*!
 *	@brief Make a call to the slave device to read data from it.
 *	@param Address of the slave device (unsigned char).
 */

void TWI_ADDRESS(unsigned char address){
	while(1){
		TWDR = address + 0x01; 						///< Load slave device's address and READ bit to SDA.
		TWCR = (1<<TWEN)|(1<<TWINT);					///< Enable TWI and set TWINT bit to 1.
		while(!(TWCR & (1<<TWINT)));					///< Wait while TWI completes current action.
		if((TWSR & 0xF8) != 0x40){
			continue;						///< If ACK has not been received, repeat the loop.
		}
		break;								///< If ACK has been received break from loop.
	}
}

/*!
 *	@brief Receive required data over TWI from the slave device.
 *	@param Amount of data to be received in bytes (uint8_t).
 */

void MASTER_RECEIVE(uint8_t datasize){
	for(uint8_t i=0;i<(datasize-1);i++){
		TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA);				///< Enable TWI and set TWINT bit to 1. Generate an ACK bit on the TWI bus to call for data.
		while(!(TWCR & (1<<TWINT)));					///< Wait while TWI completes current action.
		data[i] = TWDR; 						///< Store the data received.
	}
	TWCR = (1<<TWEN)|(1<<TWINT);						///< When receiving last data byte generate a NACK pulse.
	while(!(TWCR & (1<<TWINT)));						///< Wait while TWI completes current action.
	data[datasize-1] = TWDR;						///< Store last data byte.
	TWI_STOP();
}

/*!
 *	@brief Send STOP condition once all data has been received.
 */

void TWI_STOP(void){
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO); 				///< Enable TWI and set TWINT bit to 1. Send STOP condition.
}
