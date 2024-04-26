/*
 * CRC.c
 *
 *  Created on: 04.12.2017
 *      Author: LH
 *
 *  This is a generic implementation for a CRC8 generator supporting
 *  both compile-time (1) and run-time initialized Lookup tables for efficient CRC8 calculation.
 *  You can store multiple tables for different polynomials and (non-)reflected CRCs.
 *  The different tables are referenced by an index, with an upper limit set at compile time (CRC_TABLE_COUNT).
 *
 *  To generate CRCs you must first generate the Lookup-table by calling fillCRCTable()
 *  with any index. CRCs can then be generated from any data buffer by calling CRC()
 *  with the same index previously given to fillCRCTable().
 *
 *  The table generation has been optimized for speed so that the runtime
 *  table generation can even be done during normal operation if required.
 *  However, as long as the required polynomials are known on initialization,
 *  the table generation should be done at that time.
 *  On the Landungsbruecke the initialization of a CRC table takes ~250µs. (2)
 *  Should your application still have problems with the table calculation time,
 *  this algorithm could probably be speed up by preparing a 2- or 4-bit lookup table
 *  to speed up the actual table generation.
 *
 *  (1): For compile-time CRC tables, just fill the table(s) by initializing CRCTables[] to the proper values.
 *  (2): Tested by toggling a GPIO pin, generating a table in-between and measuring the GPIO pulse width.
 */

#include "include/CRC.h"

typedef struct {
	uint8_t table[256];
	uint8_t polynomial;
	bool isReflected;
} CRCTypeDef;

CRCTypeDef CRCTables[CRC_TABLE_COUNT] = { 0 };

static uint8_t flipByte(uint8_t value);
static uint32_t flipBitsInBytes(uint32_t value);

uint8_t tmc_fillCRC8Table(uint8_t polynomial, bool isReflected, uint8_t index)
{
	uint32_t CRCdata;
	// Helper pointer for traversing the result table
	uint8_t *table;

	if(index >= CRC_TABLE_COUNT)
		return 0;

	CRCTables[index].polynomial   = polynomial;
	CRCTables[index].isReflected  = isReflected;
	table = &CRCTables[index].table[0];

	// Extend the polynomial to correct byte MSBs shifting into next bytes
	uint32_t poly = (uint32_t) polynomial | 0x0100;

	// Iterate over all 256 possible uint8_t values, compressed into a uint32_t (see detailed explanation above)
	uint32_t i;
	for(i = 0x03020100; i != 0x04030200; i+=0x04040404)
	{
		// For reflected table: Flip the bits of each input byte
		CRCdata = (isReflected)? flipBitsInBytes(i) : i;

		// Iterate over 8 Bits
		int j;
		for(j = 0; j < 8; j++)
		{
			// Store value of soon-to-be shifted out byte
			uint8_t isMSBSet = (CRCdata & 0x80000000)? 1:0;

			// CRC Shift
			CRCdata <<= 1;

			// XOR the bytes when required, lowest to highest
			CRCdata ^= (CRCdata & 0x00000100)? (poly      ) : 0;
			CRCdata ^= (CRCdata & 0x00010000)? (poly << 8 ) : 0;
			CRCdata ^= (CRCdata & 0x01000000)? (poly << 16) : 0;
			CRCdata ^= (isMSBSet)?             (poly << 24) : 0;
		}

		// For reflected table: Flip the bits of each output byte
		CRCdata = (isReflected)? flipBitsInBytes(CRCdata) : CRCdata;
		// Store the CRC result bytes in the table array
		*table++ = (uint8_t) CRCdata;
		CRCdata >>= 8;
		*table++ = (uint8_t) CRCdata;
		CRCdata >>= 8;
		*table++ = (uint8_t) CRCdata;
		CRCdata >>= 8;
		*table++ = (uint8_t) CRCdata;
	}

	return 1;
}

/* This function calculates the CRC from a data buffer
 *
 * Arguments:
 *     uint8_t *data: A pointer to the data that will be CRC'd.
 *     uint32_t bytes: The length of the data buffer.
 *     uint8_t index: The index of the CRC table to be used.
 */
uint8_t tmc_CRC8(uint8_t *data, uint32_t bytes, uint8_t index)
{
	uint8_t result = 0;
	uint8_t *table;

	if(index >= CRC_TABLE_COUNT)
		return 0;

	table = &CRCTables[index].table[0];

	while(bytes--)
		result = table[result ^ *data++];

	return (CRCTables[index].isReflected)? flipByte(result) : result;
}

uint8_t tmc_tableGetPolynomial(uint8_t index)
{
	if(index >= CRC_TABLE_COUNT)
		return 0;

	return CRCTables[index].polynomial;
}

bool tmc_tableIsReflected(uint8_t index)
{
	if(index >= CRC_TABLE_COUNT)
		return false;

	return CRCTables[index].isReflected;
}

// Helper functions
static uint8_t flipByte(uint8_t value)
{
	// swap odd and even bits
	value = ((value >> 1) & 0x55) | ((value & 0x55) << 1);
	// swap consecutive pairs
	value = ((value >> 2) & 0x33) | ((value & 0x33) << 2);
	// swap nibbles ...
	value = ((value >> 4) & 0x0F) | ((value & 0x0F) << 4);

	return value;
}

/* This helper function switches all bits within each byte.
 * The byte order remains the same:
 * [b31 b30 b29 b28 b27 b26 b25 b24 .. b7 b6 b5 b4 b3 b2 b1 b0]
 *                                  ||
 *                                 \||/
 *                                  \/
 * [b24 b25 b26 b27 b28 b29 b30 b31 .. b0 b1 b2 b3 b4 b5 b6 b7]
 */
static uint32_t flipBitsInBytes(uint32_t value)
{
	// swap odd and even bits
	value = ((value >> 1) & 0x55555555) | ((value & 0x55555555) << 1);
	// swap consecutive pairs
	value = ((value >> 2) & 0x33333333) | ((value & 0x33333333) << 2);
	// swap nibbles ...
	value = ((value >> 4) & 0x0F0F0F0F) | ((value & 0x0F0F0F0F) << 4);

	return value;
}
