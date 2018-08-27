#include "sha256.h"

#define ror(a, n) ((a >> n) | (a << (32-n)))

#define maj(a, b, c) ((a&b)^(b&c)^(a&c))

#define ch(a, b, c) ((a & b) ^ ((~ a) & c))

#define R(a,b,c,d,e,f,g,h,r,t,s) {\
    r=(ror(e,6)^ror(e,11)^ror(e,25))+ch(e,f,g)+h+t+s;\
    d+=r;\
    h=maj(a,b,c)+(ror(a,2)^ror(a,13)^ror(a,22))+r;};\

sha256::sha256()
{

}

int32_t sha256::init()
{
	state[0] = 0x6a09e667U;
	state[1] = 0xbb67ae85U;
	state[2] = 0x3c6ef372U;
	state[3] = 0xa54ff53aU;
	state[4] = 0x510e527fU;
	state[5] = 0x9b05688cU;
	state[6] = 0x1f83d9abU;
	state[7] = 0x5be0cd19U;
	return 0;
}

uint8_t* sha256::encode()
{
	uint32_t length = 8;
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

void sha256::decode(uint32_t output[], const uint8_t input[])
{
	uint32_t s0, s1;
	for (uint32_t i = 0, j = 0; j < 64; i++, j += 4)
	{
		output[i] = input[j] << 24 | input[j + 1] << 16 | input[j + 2] << 8 | input[j + 3];
		//output[i] = GETU32(input + j);
	}
	for (uint32_t i = 16; i < 64; i++)
	{
		s0 = ror(output[i - 15], 7) ^ ror(output[i - 15], 18) ^ (output[i - 15] >> 3);
		s1 = ror(output[i - 2], 17) ^ ror(output[i - 2], 19) ^ (output[i - 2] >> 10);
		output[i] = output[i - 16] + s0 + output[i - 7] + s1;
	}
}

void sha256::transform(const uint8_t input[])
{
	register uint32_t A = state[0];
	register uint32_t B = state[1];
	register uint32_t C = state[2];
	register uint32_t D = state[3];
	register uint32_t E = state[4];
	register uint32_t F = state[5];
	register uint32_t G = state[6];
	register uint32_t H = state[7];
	register uint32_t Z;
	decode(X, input);

	R(A, B, C, D, E, F, G, H, Z, 0x428a2f98U, X[0]);
	R(H, A, B, C, D, E, F, G, Z, 0x71374491U, X[1]);
	R(G, H, A, B, C, D, E, F, Z, 0xb5c0fbcfU, X[2]);
	R(F, G, H, A, B, C, D, E, Z, 0xe9b5dba5U, X[3]);
	R(E, F, G, H, A, B, C, D, Z, 0x3956c25bU, X[4]); 
	R(D, E, F, G, H, A, B, C, Z, 0x59f111f1U, X[5]);
	R(C, D, E, F, G, H, A, B, Z, 0x923f82a4U, X[6]);
	R(B, C, D, E, F, G, H, A, Z, 0xab1c5ed5U, X[7]);

	R(A, B, C, D, E, F, G, H, Z, 0xd807aa98U, X[8]);
	R(H, A, B, C, D, E, F, G, Z, 0x12835b01U, X[9]);
	R(G, H, A, B, C, D, E, F, Z, 0x243185beU, X[10]);
	R(F, G, H, A, B, C, D, E, Z, 0x550c7dc3U, X[11]);
	R(E, F, G, H, A, B, C, D, Z, 0x72be5d74U, X[12]);
	R(D, E, F, G, H, A, B, C, Z, 0x80deb1feU, X[13]);
	R(C, D, E, F, G, H, A, B, Z, 0x9bdc06a7U, X[14]);
	R(B, C, D, E, F, G, H, A, Z, 0xc19bf174U, X[15]);

	R(A, B, C, D, E, F, G, H, Z, 0xe49b69c1U, X[16]);
	R(H, A, B, C, D, E, F, G, Z, 0xefbe4786U, X[17]);
	R(G, H, A, B, C, D, E, F, Z, 0x0fc19dc6U, X[18]);
	R(F, G, H, A, B, C, D, E, Z, 0x240ca1ccU, X[19]);
	R(E, F, G, H, A, B, C, D, Z, 0x2de92c6fU, X[20]);
	R(D, E, F, G, H, A, B, C, Z, 0x4a7484aaU, X[21]);
	R(C, D, E, F, G, H, A, B, Z, 0x5cb0a9dcU, X[22]);
	R(B, C, D, E, F, G, H, A, Z, 0x76f988daU, X[23]);

	R(A, B, C, D, E, F, G, H, Z, 0x983e5152U, X[24]);
	R(H, A, B, C, D, E, F, G, Z, 0xa831c66dU, X[25]);
	R(G, H, A, B, C, D, E, F, Z, 0xb00327c8U, X[26]);
	R(F, G, H, A, B, C, D, E, Z, 0xbf597fc7U, X[27]);
	R(E, F, G, H, A, B, C, D, Z, 0xc6e00bf3U, X[28]);
	R(D, E, F, G, H, A, B, C, Z, 0xd5a79147U, X[29]);
	R(C, D, E, F, G, H, A, B, Z, 0x06ca6351U, X[30]);
	R(B, C, D, E, F, G, H, A, Z, 0x14292967U, X[31]);

	R(A, B, C, D, E, F, G, H, Z, 0x27b70a85U, X[32]);
	R(H, A, B, C, D, E, F, G, Z, 0x2e1b2138U, X[33]);
	R(G, H, A, B, C, D, E, F, Z, 0x4d2c6dfcU, X[34]);
	R(F, G, H, A, B, C, D, E, Z, 0x53380d13U, X[35]);
	R(E, F, G, H, A, B, C, D, Z, 0x650a7354U, X[36]);
	R(D, E, F, G, H, A, B, C, Z, 0x766a0abbU, X[37]);
	R(C, D, E, F, G, H, A, B, Z, 0x81c2c92eU, X[38]);
	R(B, C, D, E, F, G, H, A, Z, 0x92722c85U, X[39]);

	R(A, B, C, D, E, F, G, H, Z, 0xa2bfe8a1U, X[40]);
	R(H, A, B, C, D, E, F, G, Z, 0xa81a664bU, X[41]);
	R(G, H, A, B, C, D, E, F, Z, 0xc24b8b70U, X[42]);
	R(F, G, H, A, B, C, D, E, Z, 0xc76c51a3U, X[43]);
	R(E, F, G, H, A, B, C, D, Z, 0xd192e819U, X[44]);
	R(D, E, F, G, H, A, B, C, Z, 0xd6990624U, X[45]);
	R(C, D, E, F, G, H, A, B, Z, 0xf40e3585U, X[46]);
	R(B, C, D, E, F, G, H, A, Z, 0x106aa070U, X[47]);

	R(A, B, C, D, E, F, G, H, Z, 0x19a4c116U, X[48]);
	R(H, A, B, C, D, E, F, G, Z, 0x1e376c08U, X[49]);
	R(G, H, A, B, C, D, E, F, Z, 0x2748774cU, X[50]);
	R(F, G, H, A, B, C, D, E, Z, 0x34b0bcb5U, X[51]);
	R(E, F, G, H, A, B, C, D, Z, 0x391c0cb3U, X[52]);
	R(D, E, F, G, H, A, B, C, Z, 0x4ed8aa4aU, X[53]);
	R(C, D, E, F, G, H, A, B, Z, 0x5b9cca4fU, X[54]);
	R(B, C, D, E, F, G, H, A, Z, 0x682e6ff3U, X[55]);

	R(A, B, C, D, E, F, G, H, Z, 0x748f82eeU, X[56]);
	R(H, A, B, C, D, E, F, G, Z, 0x78a5636fU, X[57]);
	R(G, H, A, B, C, D, E, F, Z, 0x84c87814U, X[58]);
	R(F, G, H, A, B, C, D, E, Z, 0x8cc70208U, X[59]);
	R(E, F, G, H, A, B, C, D, Z, 0x90befffaU, X[60]);
	R(D, E, F, G, H, A, B, C, Z, 0xa4506cebU, X[61]);
	R(C, D, E, F, G, H, A, B, Z, 0xbef9a3f7U, X[62]);
	R(B, C, D, E, F, G, H, A, Z, 0xc67178f2U, X[63]);

	state[0] += A;
	state[1] += B;
	state[2] += C;
	state[3] += D;
	state[4] += E;
	state[5] += F;
	state[6] += G;
	state[7] += H;

	A = B = C = D = E = F = G = H = 0;
}

int32_t sha256::update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
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

void sha256::finalize(const uint8_t input[], uint32_t length)
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