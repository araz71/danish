#ifndef DANISH_LINK_H_
#define DANISH_LINK_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

#include "danish_conf.h"
#include "danish.h"

#ifndef DANISH_LINK_MAX_REGISTERS
#error "Please define DANISH_LINK_MAX_REGISTERS to show how many registers you have"
#endif

typedef uint16_t RegID;

typedef void (*filled_callback_ptr)(uint16_t, uint8_t*);
typedef void (*writer_ptr)(uint8_t*, uint8_t);
typedef uint8_t (*writer_busy_ptr)();

#pragma pack(push)
#pragma pack(1)
typedef struct  {
	uint16_t bregID;	// Beginning of RegisterID
	uint16_t eregID;	// End of RegisterID

	uint8_t flags;		// Flags for read/write request
	uint8_t *ptr;		// Pointer of buffer which will read or write in.
	uint8_t size;		// Size of buffer

	uint8_t rwaddr;		// Address of module which we want to read/write

    filled_callback_ptr filled_callback;    	// Will call when buffer is filled by writer
    void (*write_ack_callback)();           	// Will call when destination returns write callback
    uint8_t* (*read_callback)(uint16_t regID);	// Will call when reading requested
} reg_st;
#pragma pack(pop)

void danish_add_register(reg_st *reg);

int8_t danish_write(uint8_t destination, uint16_t regID);
int8_t danish_read(uint8_t addr, uint16_t regID);

uint8_t danish_handle(danish_st* packet, uint8_t* response);

void danish_machine();

void danish_link_init(uint8_t address, writer_ptr write_interface, writer_busy_ptr write_busy_callback);

#endif
