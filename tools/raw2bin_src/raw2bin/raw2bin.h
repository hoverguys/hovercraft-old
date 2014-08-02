#ifndef _RAW2BIN_H
#define _RAW2BIN_H

// Endian magic below
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  3412
#endif /* LITTLE_ENDIAN */

#ifndef BIG_ENDIAN
#define BIG_ENDIAN     1234
#endif /* BIGE_ENDIAN */

#ifndef BYTE_ORDER
#define BYTE_ORDER     LITTLE_ENDIAN
#endif /* BYTE_ORDER */

#define _SHIFTL(v,s,w)							((unsigned int)(((unsigned int)(v)&((0x01<<(w))-1))<<(s)))
#define _SHIFTR(v,s,w)							((unsigned int)(((unsigned int)(v)>>(s))&((0x01<<(w))-1)))

#define LOWORD(l)   					        ((unsigned short)(l))
#define HIWORD(l)   					        ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)   					        ((unsigned char)(w))
#define HIBYTE(w)   					        ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

#define SwapInt(n)								(LOBYTE(LOWORD(n))<<24) + (HIBYTE(LOWORD(n))<<16) + (LOBYTE(HIWORD(n))<<8) + HIBYTE(HIWORD(n))
#define SwapShort(n)							(LOBYTE(n)<<8) + HIBYTE(n)

static inline float SwapFloat(const float n) {
	float retVal;
	char *floatToConvert = ( char* ) & n;
	char *returnFloat = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];

	return retVal;
}

#if BYTE_ORDER == LITTLE_ENDIAN
#define EndianFixInt(x) (SwapInt(x))
#define EndianFixShort(x) (SwapShort(x))
#define EndianFixFloat(x) (SwapFloat(x))
#else
#define EndianFixInt(x) (x)
#define EndianFixShort(x) (x)
#define EndianFixFloat(x) (x)
#endif

#endif