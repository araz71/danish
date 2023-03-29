#ifndef _DANISH_H_
#define _DANISH_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "delay.h"
#include "danish_conf.h"

#ifndef DANISH_MAX_DATA_SIZE
#error "Please define DANISH_MAX_DATA_SIZE"
#endif

#if DANISH_MAX_DATA_SIZE > 196
#error "Data size should be less than 196 bytes"
#endif

#define DANISH_MAX_PACKET_SIZE		DANISH_MAX_DATA_SIZE + 8

typedef enum {
	FUNC_WRITE,
	FUNC_WRITE_ACK,
	FUNC_READ,
	FUNC_READ_ACK
} function_enu;

#pragma pack(push)
#pragma pack(1)
typedef struct {
    uint8_t src;			// Source Address
    uint8_t dst;			// Destination Address
	function_enu function;	// Function
    uint16_t regID;			// Register number
    uint8_t len;			// Length of data
	uint8_t *data;			// Data
} danish_st;
#pragma pack(pop)

/*
 * Makes packet with requested parameters.
 *
 * @param source Source address(Sender)
 * @param destination Destination address (Receiver)
 * @param function Requests type. It should be one of function_enu values
 * @param regID Register number
 * @param len Length of data
 * @param data Pointer to array of data
 * @param packet Pointer to array where created packet will store in it.
 *
 * @return Size of whole packet
 */
uint8_t danish_make(uint8_t source, uint8_t destination, function_enu function,
                    uint16_t regID, uint8_t len, uint8_t *data, uint8_t *packet);

/*
 * Opens given packet and extracts it's fields into result structure.
 *
 * @param packet Received packet from interface
 * @param len Length of received packet
 * @param result Pointer of result where values will store in.
 *
 * @return When incomplete packet is received : 0, when CRC or fields error : -1. in Successfull : 1
 */
int8_t danish_ach(uint8_t *packet, uint8_t len, danish_st *result);

/*
 * Stores every byte received by interface into receive buffer and updates receive timestamp.
 *
 * @param c Received byte from interface
 */
void danish_collect(uint8_t c);

/*
 * Checks validation of data stored in receiver buffer which fills by danish_collect. then opens
 * using danish_ach.
 *
 * @param packet Result of parse will store in this pointer to packet.
 *
 * @return When successfull packet receives : 1, otherwise 0
 */
int danish_parse(danish_st *packet);

uint64_t __attribute__((weak)) get_timestamp();

uint8_t __attribute__((weak)) delay_ms(uint64_t ts, uint32_t delay);

#ifdef DANISH_STATS
uint32_t danish_stats_get_successfull_received();
uint32_t danish_stats_get_checksum_error();
uint32_t danish_stats_get_full_error();
#endif

#endif
