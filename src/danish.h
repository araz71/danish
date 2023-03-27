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
    uint8_t src;
    uint8_t dst;
	function_enu function;
    uint16_t regID;
    uint8_t len;
	uint8_t *data;
} danish_st;
#pragma pack(pop)

uint8_t danish_make(uint8_t source, uint8_t destination, function_enu function,
                    uint16_t regID, uint8_t len, uint8_t *data, uint8_t *packet);

void danish_collect(uint8_t c);

/*
 * return
 * 	1 	: successfull
 *	0 	: failed
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
