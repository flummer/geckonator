#ifndef _IHEX_H
#define _IHEX_H

enum ihex_return {
	IHEX_MORE,
	IHEX_DATA,
	IHEX_EOF,
	IHEX_EPARSE,
	IHEX_ETYPE,
	IHEX_ELENGTH,
	IHEX_ECHECKSUM,
};

struct ihex_parser {
	uint32_t offset;
	union {
		struct {
			uint8_t  byte;
			uint8_t  length;
			uint16_t address;
		};
		enum ihex_return err;
	};
	uint8_t type;
	uint8_t state;
	uint8_t checksum;
	uint8_t index;
	uint8_t data[32];
};

static inline uint32_t __attribute__((pure))
ihex_address(struct ihex_parser *p)
{
	return p->offset + p->address;
}

extern const char *ihex_strerror(enum ihex_return ret);

extern void ihex_init(struct ihex_parser *p);
extern enum ihex_return ihex_feed(struct ihex_parser *p, uint8_t c);

#endif
