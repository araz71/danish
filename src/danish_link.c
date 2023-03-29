#include "danish_link.h"
#include <assert.h>

typedef enum {
    write_flag      = 0x01,
    read_flag       = 0x02,
    write_ack_flag  = 0x04,
    read_ack_flag   = 0x08,
} danish_link_flags_enu;

reg_st __attribute__((weak)) registers[DANISH_LINK_MAX_REGISTERS];
static uint8_t number_of_registered_ids = 0;

static writer_ptr danish_writer;
static writer_busy_ptr danish_writer_is_busy;

static uint8_t danish_address;

/*
 * Tx buffer should be at least tow bytes greater than maximum packet size.
 * Because It is hard to handle SR pin /when using DMA on RS485 bus on interrupt.
 * These two bytes extra makes sure transition is completed for requested packet.
 */
static uint8_t tx_buffer[DANISH_MAX_PACKET_SIZE + 1];
static uint16_t tx_len = 0;

void danish_add_register(reg_st *reg)
{
    assert(!(number_of_registered_ids >= DANISH_LINK_MAX_REGISTERS));

    for (int i = 0; i < DANISH_LINK_MAX_REGISTERS; i++)
        assert(!(registers[i].regID == reg->regID));

	reg->rwaddr = 0;
	reg->flags = 0;

    memcpy((uint8_t*)&registers[number_of_registered_ids], (uint8_t*)reg, sizeof(reg_st));

	number_of_registered_ids++;
}

static reg_st* find_register_inf(uint16_t regID) {
	for (uint8_t i = 0; i < DANISH_LINK_MAX_REGISTERS; i++) {
		if (registers[i].regID == regID)
			return &registers[i];
	}

	return NULL;
}

int8_t danish_write(uint8_t destination, uint16_t regID) {
    // Finds register
    reg_st *reg = find_register_inf(regID);
    if (reg == NULL)
        return -1;	// Unknow register id

    if (reg->flags) {
        return 0;	// Busy for read/write
    }

    reg->rwaddr = destination;
    reg->flags |= write_flag;

	return 1;
}

int8_t danish_read(uint8_t addr, uint16_t regID) {
    // Finds register
    reg_st *reg = find_register_inf(regID);
    if (reg == NULL)
        return -1;	// Unknown register id

    if (reg->flags)
        return 0;	// Busy for read/write

	reg->rwaddr = addr;
    reg->flags |= read_flag;

	return 1;
}

void danish_link_init(uint8_t address, writer_ptr write_interface,writer_busy_ptr writer_busy_callback) {
	danish_writer = write_interface;
	danish_address = address;
	danish_writer_is_busy = writer_busy_callback;
}

uint8_t danish_handle(danish_st* packet, uint8_t* response) {
    if (packet->dst == danish_address) {
        uint8_t return_size = 0;

        // Checks for register number
        reg_st* reg = find_register_inf(packet->regID);
        if (reg == NULL)
            return 0;

        if (packet->function == FUNC_WRITE) {
            if (packet->len != reg->size)
            	return 0;

            // FIXME : We should have accessing level here. Maybe register is only readable.

            // Copys data into registers buffer and return WRITE_ACK
            memcpy(reg->ptr, packet->data, reg->size);
            if (reg->filled_callback != NULL)
                reg->filled_callback(packet->src);

            return_size = danish_make(danish_address, packet->src, FUNC_WRITE_ACK,
                                      	  packet->regID, 0, NULL, response);

        } else if (packet->function == FUNC_READ) {
        	// FIXME : We should have accessing level here. Maybe register is only writable.

            // Returns READ_ACK with registers data
            return_size = danish_make(danish_address, packet->src, FUNC_READ_ACK,
                                      	  packet->regID, reg->size, reg->ptr, response);

        } else if (packet->function == FUNC_WRITE_ACK) {
            if (reg->rwaddr == packet->src && (reg->flags & write_ack_flag)) {
                reg->flags &= ~write_ack_flag;

                if (reg->write_ack_callback != NULL)
                    reg->write_ack_callback();
            }

        } else if (packet->function == FUNC_READ_ACK) {
            if (packet->len != reg->size)
            	return 0;

            if (reg->rwaddr == packet->src && (reg->flags & read_ack_flag)) {
                reg->flags &= ~read_ack_flag;

                memcpy(reg->ptr, packet->data, reg->size);
                if (reg->filled_callback != NULL)
                    reg->filled_callback(packet->src);
            }
        }

        return return_size;
    }

    return 0;
}

// Use this machine when implementing in embedded systems
void danish_machine() {
	static danish_st rcv_packet;

	int8_t fret = danish_parse(&rcv_packet);
	if (fret == 1)
        tx_len = danish_handle(&rcv_packet, tx_buffer);

    if (tx_len != 0) {
    	if (!danish_writer_is_busy()) {
    		danish_writer(tx_buffer, tx_len + 2);	// Two-bytes extra for SR pin
    		tx_len = 0;
    	}

    }
#ifdef DANISH_MASTER
    else if (!danish_writer_is_busy()) {
        reg_st* reg;
        for (int i = 0; i < DANISH_LINK_MAX_REGISTERS; i++) {
            reg = &registers[i];

            if (reg->flags & 0x3) {
                if (reg->flags & read_flag) {
                    reg->flags |= read_ack_flag;
                    reg->flags &= ~read_flag;
                    tx_len = danish_make(danish_address, reg->rwaddr, FUNC_READ,
                                         	 reg->regID, 0, NULL, tx_buffer);

                } else {
                    reg->flags |= write_ack_flag;
                    reg->flags &= ~write_flag;
                    tx_len = danish_make(danish_address, reg->rwaddr, FUNC_WRITE,
                                         	 reg->regID, reg->size, reg->ptr, tx_buffer);
                }

                if (tx_len != 0) {
                    danish_writer(tx_buffer, tx_len + 2);	// Two-bytes extra for SR pin
                    tx_len = 0;
                }
            }
        }
    }
#endif
}
