#include "md5.h"

#define F(b,c,d)        (((c ^ d) & b) ^ d)
#define G(b,c,d)        (((b ^ c) & d) ^ c)
#define H(b,c,d)        (b ^ c ^ d)
#define I(b,c,d)        ((~d | b) ^ c)

#define ROTATELEFT(a, n) ((a << n) | (a >> (32-n)))

#define R0(a,b,c,d,k,s,t) { \
        a+=(k+t+F(b,c,d)); \
        a=ROTATELEFT(a,s); \
        a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
        a+=(k+t+G(b,c,d)); \
        a=ROTATELEFT(a,s); \
        a+=b; };

#define R2(a,b,c,d,k,s,t) { \
        a+=(k+t+H(b,c,d)); \
        a=ROTATELEFT(a,s); \
        a+=b; };

#define R3(a,b,c,d,k,s,t) { \
        a+=(k+t+I(b,c,d)); \
        a=ROTATELEFT(a,s); \
        a+=b; };

md5::md5()
{

}

int32_t md5::init()
{
	state[0] = 0x67452301U;
	state[1] = 0xefcdab89U;
	state[2] = 0x98badcfeU;
	state[3] = 0x10325476U;
	return 0;
}

void md5::decode(uint32_t output[], const uint8_t input[])
{
	for (uint32_t i = 0, j = 0; j < 64; i++, j += 4)
	{
		//output[i] = input[j] | input[j + 1] << 8 | input[j + 2] << 16 | input[j + 3] << 24;
		output[i] = *(uint32_t*)(input + j);
	}
}

uint8_t* md5::encode()
{
	uint32_t length = 4;
	uint32_t *input = state;

	uint8_t	*output = new uint8_t[length * 8 + 1];
	memset(output, 0, sizeof(uint8_t)*(length * 8 + 1));

	for (uint32_t i = 0, j = 0; i < length * 8; i += 8, j++)
	{
		output[i + 6] = input[j] >> 28;
		output[i + 7] = (input[j] >> 24) & 0xF;
		output[i + 4] = (input[j] >> 20) & 0xF;
		output[i + 5] = (input[j] >> 16) & 0xF;
		output[i + 2] = (input[j] >> 12) & 0xF;
		output[i + 3] = (input[j] >> 8) & 0xF;
		output[i] = (input[j] >> 4) & 0xF;
		output[i + 1] = input[j] & 0xF;
	}

	// now output is just number(0 ~ 15)
	// turn it into character('0' ~ 'F')
	for (uint32_t i = 0; i < length * 8; i++)
	{
		if (output[i] < 0xA)
		{
			output[i] += 0x30;
		}
		else
		{
			output[i] += 0x37;
		}
	}
	return output;
}

void md5::transform(const uint8_t input[])
{
	uint32_t A = state[0];
	uint32_t B = state[1];
	uint32_t C = state[2];
	uint32_t D = state[3];
	decode(X, input);

	/* Round 1 */
	R0(A, B, C, D, X[0], 7, 0xd76aa478L);
	R0(D, A, B, C, X[1], 12, 0xe8c7b756L);
	R0(C, D, A, B, X[2], 17, 0x242070dbL);
	R0(B, C, D, A, X[3], 22, 0xc1bdceeeL);
	R0(A, B, C, D, X[4], 7, 0xf57c0fafL);
	R0(D, A, B, C, X[5], 12, 0x4787c62aL);
	R0(C, D, A, B, X[6], 17, 0xa8304613L);
	R0(B, C, D, A, X[7], 22, 0xfd469501L);
	R0(A, B, C, D, X[8], 7, 0x698098d8L);
	R0(D, A, B, C, X[9], 12, 0x8b44f7afL);
	R0(C, D, A, B, X[10], 17, 0xffff5bb1L);
	R0(B, C, D, A, X[11], 22, 0x895cd7beL);
	R0(A, B, C, D, X[12], 7, 0x6b901122L);
	R0(D, A, B, C, X[13], 12, 0xfd987193L);
	R0(C, D, A, B, X[14], 17, 0xa679438eL);
	R0(B, C, D, A, X[15], 22, 0x49b40821L);

	/* Round 2 */
	R1(A, B, C, D, X[1], 5, 0xf61e2562L);
	R1(D, A, B, C, X[6], 9, 0xc040b340L);
	R1(C, D, A, B, X[11], 14, 0x265e5a51L);
	R1(B, C, D, A, X[0], 20, 0xe9b6c7aaL);
	R1(A, B, C, D, X[5], 5, 0xd62f105dL);
	R1(D, A, B, C, X[10], 9, 0x2441453L);
	R1(C, D, A, B, X[15], 14, 0xd8a1e681L);
	R1(B, C, D, A, X[4], 20, 0xe7d3fbc8L);
	R1(A, B, C, D, X[9], 5, 0x21e1cde6L);
	R1(D, A, B, C, X[14], 9, 0xc33707d6L);
	R1(C, D, A, B, X[3], 14, 0xf4d50d87L);
	R1(B, C, D, A, X[8], 20, 0x455a14edL);
	R1(A, B, C, D, X[13], 5, 0xa9e3e905L);
	R1(D, A, B, C, X[2], 9, 0xfcefa3f8L);
	R1(C, D, A, B, X[7], 14, 0x676f02d9L);
	R1(B, C, D, A, X[12], 20, 0x8d2a4c8aL);

	/* Round 3 */
	R2(A, B, C, D, X[5], 4, 0xfffa3942L);
	R2(D, A, B, C, X[8], 11, 0x8771f681L);
	R2(C, D, A, B, X[11], 16, 0x6d9d6122L);
	R2(B, C, D, A, X[14], 23, 0xfde5380cL);
	R2(A, B, C, D, X[1], 4, 0xa4beea44L);
	R2(D, A, B, C, X[4], 11, 0x4bdecfa9L);
	R2(C, D, A, B, X[7], 16, 0xf6bb4b60L);
	R2(B, C, D, A, X[10], 23, 0xbebfbc70L);
	R2(A, B, C, D, X[13], 4, 0x289b7ec6L);
	R2(D, A, B, C, X[0], 11, 0xeaa127faL);
	R2(C, D, A, B, X[3], 16, 0xd4ef3085L);
	R2(B, C, D, A, X[6], 23, 0x4881d05L);
	R2(A, B, C, D, X[9], 4, 0xd9d4d039L);
	R2(D, A, B, C, X[12], 11, 0xe6db99e5L);
	R2(C, D, A, B, X[15], 16, 0x1fa27cf8L);
	R2(B, C, D, A, X[2], 23, 0xc4ac5665L);

	/* Round 4 */
	R3(A, B, C, D, X[0], 6, 0xf4292244L);
	R3(D, A, B, C, X[7], 10, 0x432aff97L);
	R3(C, D, A, B, X[14], 15, 0xab9423a7L);
	R3(B, C, D, A, X[5], 21, 0xfc93a039L);
	R3(A, B, C, D, X[12], 6, 0x655b59c3L);
	R3(D, A, B, C, X[3], 10, 0x8f0ccc92L);
	R3(C, D, A, B, X[10], 15, 0xffeff47dL);
	R3(B, C, D, A, X[1], 21, 0x85845dd1L);
	R3(A, B, C, D, X[8], 6, 0x6fa87e4fL);
	R3(D, A, B, C, X[15], 10, 0xfe2ce6e0L);
	R3(C, D, A, B, X[6], 15, 0xa3014314L);
	R3(B, C, D, A, X[13], 21, 0x4e0811a1L);
	R3(A, B, C, D, X[4], 6, 0xf7537e82L);
	R3(D, A, B, C, X[11], 10, 0xbd3af235L);
	R3(C, D, A, B, X[2], 15, 0x2ad7d2bbL);
	R3(B, C, D, A, X[9], 21, 0xeb86d391L);

	state[0] += A;
	state[1] += B;
	state[2] += C;
	state[3] += D;
}

/*
  maybe you should declare a buffer
  copy input to buffer
  transform buffer

  output, output_size is ignored
*/
int32_t md5::update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
{
	uint32_t i;
	uint32_t L = length / block_size;

	for (i = 0; i < L; i++)
	{
		transform(input);
		input += block_size;
	}

	if (last)
	{ 
		length = length % block_size;
		finalize(input, length);
	}
	return 0;
}

void md5::finalize(const uint8_t input[], uint32_t length)
{
    uint8_t buffer[64];
	uint32_t size_high = input_size_high;
	uint32_t size_low = input_size_low;
	memset(buffer, 0, sizeof(uint8_t) * 64);
	memcpy(buffer, input, length);

	if (length < 55)
	{
		buffer[length] = 0x80;
	}
	else // 55 <= length && length < 64, length == 64 will not happen
	{
		buffer[length] = 0x80;
		transform(buffer);
		memset(buffer, 0, sizeof(uint8_t) * 64);
		buffer[0] = 0x80;
	}

	// fill in the length of input
	buffer[56] = size_low;
	buffer[57] = size_low >> 8;
	buffer[58] = size_low >> 16;
	buffer[59] = size_low >> 24;
	buffer[60] = size_high >> 24;
	buffer[61] = size_high >> 16;
	buffer[62] = size_high >> 8;
	buffer[63] = size_high;
	transform(buffer);
}