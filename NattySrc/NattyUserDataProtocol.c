

#include "NattyUserDataProtocol.h"
#include "NattyUtils.h"

#include "NattyNodeAgent.h"
#include "NattyMain.h"

extern ntyWatchConfig ntyWatchIntance;

void *ntyPacketCtor(void *_self, va_list *params) {
	Packet *self = _self;
	return self;
}


void *ntyPacketDtor(void *_self) {
	Packet *self = _self;
	self->succ = NULL;
	
	return self;
}

void ntyPacketSetSuccessor(void *_self, void *_succ) {
	Packet *self = _self;
	self->succ = _succ;
}

void* ntyPacketGetSuccessor(const void *_self) {
	const Packet *self = _self;
	return self->succ;
}


static void ntySetSuccessor(void *_filter, void *_succ) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->setSuccessor) {
		(*filter)->setSuccessor(_filter, _succ);
	}
}

static void ntyHandleRequest(const void *_filter, unsigned char **table, int count, C_DEVID id) {
	ProtocolFilter * const *filter = _filter;
	if (_filter && (*filter) && (*filter)->handleRequest) {
		(*filter)->handleRequest(_filter, table, count, id);
	}
}

static void ntyGetOperaHandleRequest(const void *_self, unsigned char **table, int count, C_DEVID id) {
	//const UdpClient *client = obj;
	if (ntyCompareGetOpera(table[0])) {
		U8 devIdBuf[9] = {0};
		int index = ntyNodeCompare(table[1]);
		if (index == -1) {
			kal_prompt_trace(MOD_BT," Opera: %s %s is not Exist\n", table[0], table[1]);
			return ;
		}

		kal_prompt_trace(MOD_BT," Opera: index:%d, taken:%s\n", index, table[1]);
		
		ntyU64Datacpy(devIdBuf, id);
		switch (index) {
			case DB_NODE_POWER: {
				//NTY_PROTO_OPERA_GET_POWER_VALUE
				ntySendMsgToMMIMod(NTY_PROTO_OPERA_GET_POWER_VALUE, devIdBuf, sizeof(C_DEVID));
				break;
			}
			case DB_NODE_SIGNAL: {
				ntySendMsgToMMIMod(NTY_PROTO_OPERA_GET_SIGNAL_VALUE, devIdBuf, sizeof(C_DEVID));
				break;
			}
			case DB_NODE_PHONEBOOK:
				break;
			case DB_NODE_FAMILYNUMBER:
				break;
			case DB_NODE_FALLEN:
				break;
			case DB_NODE_GPS:
				break;
			case DB_NODE_WIFI:
				break;
			case DB_NODE_LAB:
				break;
			default:
				break;
		}
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, table, count, id);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", table[0]);
	}

}

static const ProtocolFilter ntyGetOperaFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyGetOperaHandleRequest,
};



static void ntySetOperaHandleRequest(const void *_self, unsigned char **table, int count, C_DEVID id) {
	if (ntyCompareSetOpera(table[0])) {
		int index = ntyNodeCompare(table[1]);
		if (index == -1) {
			kal_prompt_trace(MOD_BT," Opera: %s %s is not Exist\n", table[0], table[1]);
			return ;
		}

		kal_prompt_trace(MOD_BT," Opera: index:%d, taken:%s\n", index, table[1]);
			
		switch (index) {
			case DB_NODE_PHONEBOOK:
				break;
			case DB_NODE_FAMILYNUMBER: {
				int id = 0;
				if (count < 4) break;
				id = atoi(table[2]);

				kal_prompt_trace(MOD_BT," family: id:%d, number:%s\n", id, table[3]);
				strcpy(ntyWatchIntance.u8FamilyNumber[id], table[3]);

				ntyWatchSave();
				break;
			}
			case DB_NODE_FALLEN:
				break;
			case DB_NODE_GPS:
				break;
			case DB_NODE_WIFI:
				break;
			case DB_NODE_LAB:
				break;
			default:
				break;
		}
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, table, count, id);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", table[0]);
	}

}

static const ProtocolFilter ntySetOperaFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntySetOperaHandleRequest,
};


const void *pNtyGetOperaFilter = &ntyGetOperaFilter;
const void *pNtySetOperaFilter = &ntySetOperaFilter;


void* ntyProtocolFilterInit(void) {
	void *pGetOperaFilter = New(pNtyGetOperaFilter);
	void *pSetOperaFilter = New(pNtySetOperaFilter);

	ntySetSuccessor(pGetOperaFilter, pSetOperaFilter);
	ntySetSuccessor(pSetOperaFilter, NULL);
	/*
	 * add your Filter
	 * for example:
	 * void *pFilter = New(NtyFilter);
	 * ntySetSuccessor(pLogoutFilter, pFilter);
	 */

	return pGetOperaFilter;
}


static void *ntyProtocolFilter = NULL;

void* ntyProtocolFilterInstance(void) {
	if (ntyProtocolFilter == NULL) {
		ntyProtocolFilter = ntyProtocolFilterInit();
	}
	return ntyProtocolFilter;
}


void ntyProcessUserDataPacket(unsigned char *buffer, C_DEVID id) {
	void* filter = ntyProtocolFilterInstance();
	unsigned char **pTable = (unsigned char**)OslMalloc(sizeof(unsigned char**));
	int Count = 0;

	ntySeparation(' ', buffer, &pTable, &Count);
	if (Count <= 1) return ;

	ntyHandleRequest(filter, pTable, Count, id);
	//ntyGetOperaHandleRequest(filter, pTable, Count, id);

	ntyFreeTable(&pTable, Count);
	OslMfree(pTable);
}





