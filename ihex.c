#include <stdint.h>

#ifdef STANDALONE
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#endif

#include "ihex.h"

enum ihex_cclass {
	C_0, /* 0 */
	C_1, /* 1 */
	C_2, /* 2 */
	C_3, /* 3 */
	C_4, /* 4 */
	C_5, /* 5 */
	C_6, /* 6 */
	C_7, /* 7 */
	C_8, /* 8 */
	C_9, /* 9 */
	C_A, /* A */
	C_B, /* B */
	C_C, /* C */
	C_D, /* D */
	C_E, /* E */
	C_F, /* F */
	C_I   = 1 << 4, /* : */
	C_N   = 2 << 4, /* \n, \r */
	C__   = 3 << 4, /* invalid characters */
	C_MAX = 4,
};

/*
 * map the first ASCII characters into character classes
 * remaining characters should be mapped to C__
 */
static const uint8_t ihex_cclass[] = {
/*   0 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*   8 */ C__,C__,C_N,C__,C__,C_N,C__,C__,
/*  16 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*  24 */ C__,C__,C__,C__,C__,C__,C__,C__,

/*  32 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*  40 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*  48 */ C_0,C_1,C_2,C_3,C_4,C_5,C_6,C_7,
/*  56 */ C_8,C_9,C_I,C__,C__,C__,C__,C__,

/*  64 */ C__,C_A,C_B,C_C,C_D,C_E,C_F,C__,
/*  72 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*  80 */ C__,C__,C__,C__,C__,C__,C__,C__,
/*  88 */ C__,C__,C__,C__,C__,C__,C__,C__,

/*  96 */ C__,C_A,C_B,C_C,C_D,C_E,C_F,C__,
/* 104 */ C__,C__,C__,C__,C__,C__,C__,C__,
/* 112 */ C__,C__,C__,C__,C__,C__,C__,C__,
/* 120 */ C__,C__,C__,C__,C__,C__,C__,C__,
};

enum ihex_states {
	S_OK,
	S_CO,
	S_LH,
	S_LL,
	S_AH,
	S_AL,
	S_BH,
	S_BL,
	S_TH,
	S_TL,
	S_DH,
	S_DL,
	S_CS,
	S_CH,
	S_CL,
	S_ER,
	S_MAX,
	X___,
	X_DN,
	X_ER,
};

static const uint8_t ihex_transition[S_MAX][C_MAX] = {
/*            hex   :     NL    inv */
/* S_OK */ { X___, S_CO, S_OK, X___ },
/* S_CO */ { S_LH, X___, X___, X___ },
/* S_LH */ { S_LL, X___, X___, X___ },
/* S_LL */ { S_AH, X___, X___, X___ },
/* S_AH */ { S_AL, X___, X___, X___ },
/* S_AL */ { S_BH, X___, X___, X___ },
/* S_BH */ { S_BL, X___, X___, X___ },
/* S_BL */ { S_TH, X___, X___, X___ },
/* S_TH */ { S_TL, X___, X___, X___ },
/* S_TL */ { S_DH, X___, X___, X___ },
/* S_DH */ { S_DL, X___, X___, X___ },
/* S_DL */ { S_DH, X___, X___, X___ },
/* S_CS */ { S_CH, X___, X___, X___ },
/* S_CH */ { S_CL, X___, X___, X___ },
/* S_CL */ { X___, X___, X_DN, X___ },
/* S_ER */ { S_ER, S_ER, X_ER, S_ER },
};

static const char *const ihex_return_strings[] = {
	[IHEX_MORE]      = "need more bytes",
	[IHEX_DATA]      = "data record complete",
	[IHEX_EOF]       = "end of file",
	[IHEX_EPARSE]    = "record parse error",
	[IHEX_ETYPE]     = "record type error",
	[IHEX_ELENGTH]   = "record too long",
	[IHEX_ECHECKSUM] = "invalid checksum",
};

const char *
ihex_strerror(enum ihex_return ret)
{
	return ihex_return_strings[ret];
}

void
ihex_init(struct ihex_parser *p)
{
	p->offset = 0;
	p->state = S_OK;
}

enum ihex_return
ihex_feed(struct ihex_parser *p, uint8_t c)
{
	c = (c < sizeof(ihex_cclass)) ? ihex_cclass[c] : C__;

	p->state = ihex_transition[p->state][c >> 4];

	switch (p->state) {
	case S_LH:
	case S_AH:
	case S_BH:
	case S_TH:
	case S_DH:
	case S_CH:
		p->byte = c << 4;
		break;

	case S_LL:
		c |= p->byte;
		p->checksum = c;
		p->length = c;
		if (p->length > sizeof(p->data)) {
			p->state = S_ER;
			p->err = IHEX_ELENGTH;
		}
		break;

	case S_AL:
		c |= p->byte;
		p->checksum += c;
		p->address = ((uint16_t)c) << 8;
		break;

	case S_BL:
		c |= p->byte;
		p->checksum += c;
		p->address |= c;
		break;

	case S_TL:
		c |= p->byte;
		p->checksum += c;
		p->type = c;
		p->index = 0;
		if (p->length == 0)
			p->state = S_CS;
		break;

	case S_DL:
		c |= p->byte;
		p->checksum += c;
		p->data[p->index++] = c;
		if (p->index == p->length)
			p->state = S_CS;
		break;

	case S_CL:
		c |= p->byte;
		p->checksum += c;
		if (p->checksum) {
			p->state = S_ER;
			p->err = IHEX_ECHECKSUM;
		}
		break;

	case X___:
		p->state = S_ER;
		p->err = IHEX_EPARSE;
		break;

	case X_DN:
		p->state = S_OK;
		switch (p->type) {
		case 0x00: /* regular data record */
			return IHEX_DATA;

		case 0x01: /* end-of-file record */
			if (p->length || p->address)
				return IHEX_ETYPE;
			return IHEX_EOF;

		case 0x02: /* set extended segment address */
			if (p->length != 2 || p->address)
				return IHEX_ETYPE;

			p->offset = (p->data[0] << 12) | (p->data[1] << 4);
			break;

		case 0x03: /* set 8086 cs and ip registers */
			if (p->length != 4 || p->address)
				return IHEX_ETYPE;
			/* just ignore such records */
			break;

		case 0x04: /* set linear base address */
			if (p->length != 2 || p->address)
				return IHEX_ETYPE;

			p->offset = (p->data[0] << 24) | (p->data[1] << 16);
			break;

		case 0x05: /* set 80386 eip register */
			if (p->length != 4 || p->address)
				return IHEX_ETYPE;
			/* just ignore such records */
			break;

		default:
			return IHEX_ETYPE;
		}
		break;

	case X_ER:
		p->state = S_OK;
		return p->err;
	}

	return IHEX_MORE;
}

#ifdef STANDALONE
static int
usage(const char *self, FILE *f, int ret)
{
	fprintf(f, "usage: %s <filename>\n", self);

	return ret;
}

int
main(int argc, char *argv[])
{
	struct ihex_parser p;
	const char *filename;
	FILE *f;
	int c;

	if (argc < 2)
		return usage(argv[0], stderr, EXIT_FAILURE);

	if (strcmp(argv[1], "-") == 0) {
		filename = "stdin";
		f = stdin;
	} else {
		filename = argv[1];
		f = fopen(filename, "r");
		if (f == NULL) {
			fprintf(stderr, "error opening '%s': %s\n",
			        filename, strerror(errno));
			return EXIT_FAILURE;
		}
	}

	ihex_init(&p);

	for (c = getc(f); c != EOF; c = getc(f)) {
		enum ihex_return ret = ihex_feed(&p, (uint8_t)c);

		switch (ret) {
		case IHEX_MORE:
			break;

		case IHEX_DATA:
			printf("0x%08x: %hhu bytes\n", ihex_address(&p), p.length);
			break;

		case IHEX_EOF:
			printf("EOF\n");
			goto out;

		default:
			printf("ERROR: %s\n", ihex_strerror(ret));
		}
	}

out:
	if (ferror(f)) {
		fprintf(stderr, "error reading from '%s'\n",
				filename);
	}

	if (fclose(f)) {
		fprintf(stderr, "error closing '%s': %s\n",
				filename, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#endif
