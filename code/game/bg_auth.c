#include "../qcommon/q_shared.h"
/* POINTER defines a generic pointer type */
typedef unsigned char* POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

typedef struct
{
	UINT4 state[4];             /* state (ABCD) */
	UINT4 count[2];             /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];           /* input buffer */
} OSPAUTH_CTX;

static void BG_OSPAuthInit_(OSPAUTH_CTX* context);
static void BG_OSPAuthUpdate_(OSPAUTH_CTX*, const unsigned char*, unsigned int);
static void BG_OSPAuthFinal_(unsigned char [16], OSPAUTH_CTX*);

static void BG_OSPAuthMemset_(void* dest, const int val, const int count);
static void BG_OSPAuthMemcpy_(void* dest, const void* src, const int count);


static void BG_OSPAuthTransform_(UINT4 [4], const unsigned char [64]);
static void BG_OSPAuthEncode_(unsigned char*, UINT4*, unsigned int);
static void BG_OSPAuthDecode_(UINT4*, const unsigned char*, unsigned int);

void BG_OSPAuthGetKey_(unsigned char* str, int* len);

static unsigned char PADDING[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define J(x, y, z) ((y ^ (x | ~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG and HH are transformations for rounds 1, 2 and 3 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, i) {(a) += F ((b), (c), (d)) + (x) + i; (a) = ROTATE_LEFT ((a), (s)); a += b;}

#define GG(a, b, c, d, x, s, i) {(a) += G ((b), (c), (d)) + (x) + i; (a) = ROTATE_LEFT ((a), (s)); a += b;}

#define HH(a, b, c, d, x, s, i) {(a) += H ((b), (c), (d)) + (x) + i; (a) = ROTATE_LEFT ((a), (s)); a += b;}

#define JJ(a, b, c, d, x, s, i) {(a) += J ((b), (c), (d)) + (x) + i; (a) = ROTATE_LEFT ((a), (s)); a += b;}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

OSPAUTH_CTX ospAuthCtx;

void BG_OSPAuthInit(void)
{
	BG_OSPAuthInit_(&ospAuthCtx);
}

void BG_OSPAuthUpdateName(char* str)
{
	BG_OSPAuthUpdate_(&ospAuthCtx, (unsigned char*)str, strlen(str));
}

void BG_OSPAuthGetSecret(char* secret)
{
	int len;
	char tmp[16];
	unsigned char secret_[64];
	unsigned char digest[16];

	BG_OSPAuthGetKey_(&secret_[0], &len);
	BG_OSPAuthUpdate_(&ospAuthCtx, secret_, len);
	BG_OSPAuthFinal_(&digest[0], &ospAuthCtx);

	secret_[0] = 0;
	len = 0;

	do
	{
		Com_sprintf(tmp, sizeof(tmp), "%d", digest[len]);
		strcat((char*)secret_, tmp);
	}
	while (++len < 10);

	strcpy(secret, (char*)secret_);
}

void BG_OSPAuthGetKey_(unsigned char* str, int* len)
{
	int i;
	int block_0[29] = {0xfb, 0xfb, 0xd5, 0xcf, 0xe7, 0xdb, 0x40, 0x6d, 0x0c, 0x68, 0x84, 0x6f, 0x83, 0xbe, 0x65, 0x91, 0x79, 0xbb, 0x3f, 0xf7, 0xd8, 0xc0, 0xe2, 0x77, 0x8d, 0x7f, 0x9d, 0xdf, 0xe7};
	int block_1[11] = {0xa7, 0xa9, 0xe7, 0x85, 0x7b, 0x64, 0x01, 0xfb, 0x79, 0xfb, 0xfb};
	int block_2[13] = {0x65, 0xc8, 0x0d, 0x82, 0x09, 0x84, 0x13, 0x0c, 0x42, 0xdd, 0xc0, 0xee, 0x36};

	(void)block_1;
	(void)block_2;

	*len = 21;

	for (i = 0; i < *len; ++i)
	{
		str[i] = (char)(block_0[6 + i] ^ 0xa2);
	}
}

static void BG_OSPAuthInit_(OSPAUTH_CTX* context)
{
	context->count[0] = context->count[1] = 0;

	/* Load magic initialization constants.*/
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

static void BG_OSPAuthUpdate_(OSPAUTH_CTX* context, const unsigned char* input, unsigned int inputLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
		context->count[1]++;

	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible.*/
	if (inputLen >= partLen)
	{
		BG_OSPAuthMemcpy_((POINTER)&context->buffer[index], (POINTER)input, partLen);
		BG_OSPAuthTransform_(context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			BG_OSPAuthTransform_(context->state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	BG_OSPAuthMemcpy_((POINTER)&context->buffer[index], (POINTER)&input[i], inputLen - i);
}


static void BG_OSPAuthFinal_(unsigned char digest[16], OSPAUTH_CTX* context)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	BG_OSPAuthEncode_(bits, context->count, 8);

	/* Pad out to 56 mod 64.*/
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	BG_OSPAuthUpdate_(context, PADDING, padLen);

	/* Append length (before padding) */
	BG_OSPAuthUpdate_(context, bits, 8);

	/* Store state in digest */
	BG_OSPAuthEncode_(digest, context->state, 16);

	/* Zeroize sensitive information.*/
	BG_OSPAuthMemset_((POINTER)context, 0, sizeof(*context));
}


static void BG_OSPAuthTransform_(UINT4 state[4], const unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	BG_OSPAuthDecode_(x, block, 64);

	/* Round 1 */
	FF(a, b, c, d, x[ 0], S11, 0xd76aa478);
	FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);
	FF(c, d, a, b, x[ 2], S13, 0x242070db);
	FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);
	FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);
	FF(d, a, b, c, x[ 5], S12, 0x4787c62a);
	FF(c, d, a, b, x[ 6], S13, 0xa8304613);
	FF(b, c, d, a, x[ 7], S14, 0xfd469501);
	FF(a, b, c, d, x[ 8], S11, 0x698098d8);
	FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);
	FF(c, d, a, b, x[10], S13, 0xffff5bb1);
	FF(b, c, d, a, x[11], S14, 0x895cd7be);
	FF(a, b, c, d, x[12], S11, 0x6b901122);
	FF(d, a, b, c, x[13], S12, 0xfd987193);
	FF(c, d, a, b, x[14], S13, 0xa679438e);
	FF(b, c, d, a, x[15], S14, 0x49b40821);

	/* Round 2 */
	GG(a, b, c, d, x[ 1], S21, 0xf61e2562);
	GG(d, a, b, c, x[ 6], S22, 0xc040b340);
	GG(c, d, a, b, x[11], S23, 0x265e5a51);
	GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
	GG(a, b, c, d, x[ 5], S21, 0xd62f105d);
	GG(d, a, b, c, x[10], S22, 0x02441453);
	GG(c, d, a, b, x[15], S23, 0xd8a1e681);
	GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
	GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);
	GG(d, a, b, c, x[14], S22, 0xc33707d6);
	GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);
	GG(b, c, d, a, x[ 8], S24, 0x455a14ed);
	GG(a, b, c, d, x[13], S21, 0xa9e3e905);
	GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);
	GG(c, d, a, b, x[ 7], S23, 0x676f02d9);
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

	/* Round 3 */
	HH(a, b, c, d, x[ 5], S31, 0xfffa3942);
	HH(d, a, b, c, x[ 8], S32, 0x8771f681);
	HH(c, d, a, b, x[11], S33, 0x6d9d6122);
	HH(b, c, d, a, x[14], S34, 0xfde5380c);
	HH(a, b, c, d, x[ 1], S31, 0xa4beea44);
	HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);
	HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);
	HH(b, c, d, a, x[10], S34, 0xbebfbc70);
	HH(a, b, c, d, x[13], S31, 0x289b7ec6);
	HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);
	HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);
	HH(b, c, d, a, x[ 6], S34, 0x04881d05);
	HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);
	HH(d, a, b, c, x[12], S32, 0xe6db99e5);
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
	HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);

	/* Round 4 */
	JJ(a, b, c, d, x[ 0], S41, 0xf4292244);
	JJ(d, a, b, c, x[ 7], S42, 0x432aff97);
	JJ(c, d, a, b, x[14], S43, 0xab9423a7);
	JJ(b, c, d, a, x[ 5], S44, 0xfc93a039);
	JJ(a, b, c, d, x[12], S41, 0x655b59c3);
	JJ(d, a, b, c, x[ 3], S42, 0x8f0ccc92);
	JJ(c, d, a, b, x[10], S43, 0xffeff47d);
	JJ(b, c, d, a, x[ 1], S44, 0x85845dd1);
	JJ(a, b, c, d, x[ 8], S41, 0x6fa87e4f);
	JJ(d, a, b, c, x[15], S42, 0xfe2ce6e0);
	JJ(c, d, a, b, x[ 6], S43, 0xa3014314);
	JJ(b, c, d, a, x[13], S44, 0x4e0811a1);
	JJ(a, b, c, d, x[ 4], S41, 0xf7537e82);
	JJ(d, a, b, c, x[11], S42, 0xbd3af235);
	JJ(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
	JJ(b, c, d, a, x[ 9], S44, 0xeb86d391);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	BG_OSPAuthMemset_((POINTER)x, 0, sizeof(x));
}


static void BG_OSPAuthEncode_(unsigned char* output, UINT4* input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}


static void BG_OSPAuthDecode_(UINT4* output, const unsigned char* input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((UINT4)input[j]) | (((UINT4)input[j + 1]) << 8) | (((UINT4)input[j + 2]) << 16) | (((UINT4)input[j + 3]) << 24);
}

static void BG_OSPAuthMemset_(void* dest, const int val, const int count)
{
	int i;
	for (i = 0; i < count; ++i)
	{
		((unsigned char*)dest)[i] = (unsigned char)val;
	}
}

static void BG_OSPAuthMemcpy_(void* dest, const void* src, const int count)
{
	int i;
	for (i = 0; i < count; ++i)
	{
		((unsigned char*)dest)[i] = ((unsigned char*)src)[i];
	}
}


