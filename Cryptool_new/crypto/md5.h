#pragma once
#include "crypto.h"

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

class md5 : public crypto
{
public:
	md5();
	virtual ~md5() {}

	virtual int32_t init();

	/*
	state(little endian) to hex string
	maybe it shoule be implemented elsewhere
	*/
	virtual uint8_t* encode();

	virtual void update(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);
private:
	uint32_t state[4];
	uint32_t X[16];

	// 64 Bytes
	const uint32_t block_size = 64;

	/*
	size of output or length of output hex string
	it's used in display(), which displays result on dialog
	output_size does not exceed 0xFFFFFFFF and it's usually small
	*/
	uint32_t output_size;

	// input size : 64 Bytes
	void transform(const uint8_t input[]);

	// uint8_t to uint32_t(little endian)
	void decode(uint32_t output[], const uint8_t input[], uint32_t length);

	// process the last chunk
	void finalize(const uint8_t input[], uint32_t length);
};