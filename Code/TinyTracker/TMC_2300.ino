/*
 * TMC2300.c
 *
 *  Created on: 11.01.2019
 *      Author: LK
 */

#include "include/TMC2300.h"
#include "include/CRC.h"

#define TMCSerial Serial1

#define DEBUG
#define DBGSerial Serial

// => UART wrapper
void tmc2300_readWriteArray( uint8_t *data, size_t writeLength, size_t readLength)
{
  TMCSerial.write(data, writeLength);
  TMCSerial.flush(); // Wait for data to get sent. 

  // If no reply data is expected abort here
  if (readLength == 0)
    return;

  // Wait for the reply data to be received
  while (TMCSerial.available() < readLength){
    delay(200);
  }

  // Read the reply data
  TMCSerial.readBytes(data, readLength);

}
// <= UART wrapper

// => CRC wrapper
extern uint8_t tmc2300_CRC8(uint8_t *data, size_t length);
// <= CRC wrapper

void tmc2300_writeInt(uint8_t address, int32_t value)
{
	uint8_t data[8];

	data[0] = 0x05;
	//data[1] = 3;
  data[1] = 0;
	data[2] = address | TMC_WRITE_BIT;
	data[3] = (value >> 24) & 0xFF;
	data[4] = (value >> 16) & 0xFF;
	data[5] = (value >> 8 ) & 0xFF;
	data[6] = (value      ) & 0xFF;
	data[7] = tmc_CRC8(data, 7, 0);

	tmc2300_readWriteArray(&data[0], 8, 0);
}

int32_t tmc2300_readInt(uint8_t address)
{
	uint8_t data[8] = { 0 };

	address = TMC_ADDRESS(address);

	data[0] = 0x05;
	//data[1] = 3;
  data[1] = 0;
	data[2] = address;
	data[3] = tmc_CRC8(data, 3, 0);

	tmc2300_readWriteArray(data, 4, 8);

	// Byte 0: Sync nibble correct?
	if (data[0] != 0x05) {
    DBGSerial.println("Sync Nibble Fail");
		return 0;
	}
 
	// Byte 1: Master address correct?
	if (data[1] != 0xFF) {
    DBGSerial.println("Master Address Fail");
		return 0;
	}

	// Byte 2: Address correct?
	if (data[2] != address) {
    DBGSerial.println("Address Fail");
		return 0;
	}

	// Byte 7: CRC correct?
	if (data[7] != tmc_CRC8(data, 7, 0)) {
    DBGSerial.println("CRC Fail");
		return 0;
	}

	return (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
}
