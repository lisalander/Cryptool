#include "sha1.h"

#define SWAP(x) (_rotl(x, 8) & 0x00ff00ff | _rotr(x, 8) & 0xff00ff00)

#define GETU32(p) SWAP(*((uint32_t*)(p)))

#define rol(a, n) ((a << n) | (a >> (32-n)))

#define R0(a,b,c,d,e,s){ \
        e+=((b&(c^d))^d)+rol(a,5)+s+0x5A827999U; \
        b=rol(b,30); }; \

#define R1(a,b,c,d,e,s){ \
        e+=(b^c^d)+rol(a,5)+s+0x6ED9EBA1U; \
        b=rol(b,30); }; \

#define R2(a,b,c,d,e,s){ \
        e+=((b&c)|((b|c)&d))+rol(a,5)+s+0x8F1BBCDCU; \
        b=rol(b,30); }; \

#define R3(a,b,c,d,e,s){ \
        e+=(b^c^d)+rol(a,5)+s+0xCA62C1D6U; \
        b=rol(b,30); }; \

sha1::sha1()
{

}

int32_t sha1::init()
{
	state[0] = 0x67452301U;
	state[1] = 0xefcdab89U;
	state[2] = 0x98badcfeU;
	state[3] = 0x10325476U;
	state[4] = 0xc3d2e1f0U;
	return 0;
}

uint8_t* sha1::encode()
{
	uint32_t length = 5;
	uint32_t *input = state;

	uint8_t	*output = new uint8_t[length * 8 + 1];
	memset(output, 0, sizeof(uint8_t)*(length * 8 + 1));

	for (uint32_t i = 0, j = 0; i < length * 8; i += 8, j++)
	{
		output[i] = input[j] >> 28;
		output[i + 1] = (input[j] >> 24) & 0xF;
		output[i + 2] = (input[j] >> 20) & 0xF;
		output[i + 3] = (input[j] >> 16) & 0xF;
		output[i + 4] = (input[j] >> 12) & 0xF;
		output[i + 5] = (input[j] >> 8) & 0xF;
		output[i + 6] = (input[j] >> 4) & 0xF;
		output[i + 7] = input[j] & 0xF;
	}

	/* 
	   now output is just number(0 ~ 15)
	   turn it into character('0' ~ 'F')
	*/ 
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

void sha1::decode(uint32_t output[], const uint8_t input[])
{
	for (uint32_t i = 0, j = 0; j < 64; i++, j += 4)
	{
		output[i] = input[j] << 24 | input[j + 1] << 16 | input[j + 2] << 8 | input[j + 3];
		//output[i] = GETU32(input + j);
	}
	for (uint32_t i = 16; i < 80; i++)
	{
		output[i] = rol((output[i - 3] ^ output[i - 8] ^ output[i - 14] ^ output[i - 16]), 1);
	}
}

void sha1::transform(const uint8_t input[])
{
	register uint32_t A = state[0];
	register uint32_t B = state[1];
	register uint32_t C = state[2];
	register uint32_t D = state[3];
	register uint32_t E = state[4];
	decode(X, input);

	/* Round 1 */
	R0(A, B, C, D, E, X[0]);
	R0(E, A, B, C, D, X[1]);
	R0(D, E, A, B, C, X[2]);
	R0(C, D, E, A, B, X[3]);
	R0(B, C, D, E, A, X[4]);
	R0(A, B, C, D, E, X[5]);
	R0(E, A, B, C, D, X[6]);
	R0(D, E, A, B, C, X[7]);
	R0(C, D, E, A, B, X[8]);
	R0(B, C, D, E, A, X[9]);
	R0(A, B, C, D, E, X[10]);
	R0(E, A, B, C, D, X[11]);
	R0(D, E, A, B, C, X[12]);
	R0(C, D, E, A, B, X[13]);
	R0(B, C, D, E, A, X[14]);
	R0(A, B, C, D, E, X[15]);
	R0(E, A, B, C, D, X[16]);
	R0(D, E, A, B, C, X[17]);
	R0(C, D, E, A, B, X[18]);
	R0(B, C, D, E, A, X[19]);

	/* Round 2 */
	R1(A, B, C, D, E, X[20]);
	R1(E, A, B, C, D, X[21]);
	R1(D, E, A, B, C, X[22]);
	R1(C, D, E, A, B, X[23]);
	R1(B, C, D, E, A, X[24]);
	R1(A, B, C, D, E, X[25]);
	R1(E, A, B, C, D, X[26]);
	R1(D, E, A, B, C, X[27]);
	R1(C, D, E, A, B, X[28]);
	R1(B, C, D, E, A, X[29]);
	R1(A, B, C, D, E, X[30]);
	R1(E, A, B, C, D, X[31]);
	R1(D, E, A, B, C, X[32]);
	R1(C, D, E, A, B, X[33]);
	R1(B, C, D, E, A, X[34]);
	R1(A, B, C, D, E, X[35]);
	R1(E, A, B, C, D, X[36]);
	R1(D, E, A, B, C, X[37]);
	R1(C, D, E, A, B, X[38]);
	R1(B, C, D, E, A, X[39]);

	/* Round 3 */
	R2(A, B, C, D, E, X[40]);
	R2(E, A, B, C, D, X[41]);
	R2(D, E, A, B, C, X[42]);
	R2(C, D, E, A, B, X[43]);
	R2(B, C, D, E, A, X[44]);
	R2(A, B, C, D, E, X[45]);
	R2(E, A, B, C, D, X[46]);
	R2(D, E, A, B, C, X[47]);
	R2(C, D, E, A, B, X[48]);
	R2(B, C, D, E, A, X[49]);
	R2(A, B, C, D, E, X[50]);
	R2(E, A, B, C, D, X[51]);
	R2(D, E, A, B, C, X[52]);
	R2(C, D, E, A, B, X[53]);
	R2(B, C, D, E, A, X[54]);
	R2(A, B, C, D, E, X[55]);
	R2(E, A, B, C, D, X[56]);
	R2(D, E, A, B, C, X[57]);
	R2(C, D, E, A, B, X[58]);
	R2(B, C, D, E, A, X[59]);

	/* Round 4 */
	R3(A, B, C, D, E, X[60]);
	R3(E, A, B, C, D, X[61]);
	R3(D, E, A, B, C, X[62]);
	R3(C, D, E, A, B, X[63]);
	R3(B, C, D, E, A, X[64]);
	R3(A, B, C, D, E, X[65]);
	R3(E, A, B, C, D, X[66]);
	R3(D, E, A, B, C, X[67]);
	R3(C, D, E, A, B, X[68]);
	R3(B, C, D, E, A, X[69]);
	R3(A, B, C, D, E, X[70]);
	R3(E, A, B, C, D, X[71]);
	R3(D, E, A, B, C, X[72]);
	R3(C, D, E, A, B, X[73]);
	R3(B, C, D, E, A, X[74]);
	R3(A, B, C, D, E, X[75]);
	R3(E, A, B, C, D, X[76]);
	R3(D, E, A, B, C, X[77]);
	R3(C, D, E, A, B, X[78]);
	R3(B, C, D, E, A, X[79]);

	state[0] += A;
	state[1] += B;
	state[2] += C;
	state[3] += D;
	state[4] += E;

	A = B = C = D = E = 0;
}

int32_t sha1::update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
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

void sha1::finalize(const uint8_t input[], uint32_t length)
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
	buffer[56] = size_high >> 24;
	buffer[57] = size_high >> 16;
	buffer[58] = size_high >> 8;
	buffer[59] = size_high;
	buffer[60] = size_low >> 24;
	buffer[61] = size_low >> 16;
	buffer[62] = size_low >> 8;
	buffer[63] = size_low;
	transform(buffer);
}