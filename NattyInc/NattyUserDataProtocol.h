



#ifndef __NATTY_USER_DATA_PROTOCOL_H__
#define __NATTY_USER_DATA_PROTOCOL_H__

#include "NattyAbstractClass.h"




typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*setSuccessor)(void *_self, void *succ);
	void* (*getSuccessor)(const void *_self);
	void (*handleRequest)(const void *_self, unsigned char **table, int length,C_DEVID id);
} ProtocolFilter;

typedef struct {
	const void *_;
	void *succ;
} Packet;

void ntyProcessUserDataPacket(unsigned char *buffer, C_DEVID id);



#endif






