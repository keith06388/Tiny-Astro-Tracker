/*
 * TMC2300.h
 *
 *  Created on: 11.01.2019
 *      Author: LK
 */

#ifndef TMC_IC_TMC2300_H_
#define TMC_IC_TMC2300_H_

#include "Constants.h"
#include "API_Header.h"
#include "TMC2300_Constants.h"
#include "TMC2300_Register.h"
#include "TMC2300_Fields.h"

// Helper macros
#define TMC2300_FIELD_READ(address, mask, shift) \
	FIELD_GET(tmc2300_readInt(address), mask, shift)
#define TMC2300_FIELD_UPDATE(address, mask, shift, value) \
	(tmc2300_writeInt(address, FIELD_SET(tmc2300_readInt(address), mask, shift, value)))

void tmc2300_writeInt(uint8_t address, int32_t value);
int32_t tmc2300_readInt(uint8_t address);

#endif /* TMC_IC_TMC2300_H_ */