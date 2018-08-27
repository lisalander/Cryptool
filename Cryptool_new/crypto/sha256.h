#pragma once
#include "crypto.h"

class sha256 : public crypto
{
public:
	sha256();
	virtual ~sha256() {}

	// init state
	virtual int32_t init();

	// state(little endian) to hex string
	virtual uint8_t* encode();

	virtual int32_t update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last);
private:
	uint32_t state[8];
	uint32_t X[64];

	// 64 Bytes
	const uint32_t block_size = 64;

	// input size : 64 Bytes
	void transform(const uint8_t input[]);

	// uint8_t to uint32_t(little endian)
	void decode(uint32_t output[], const uint8_t input[]);

	// process the last chunk
	void finalize(const uint8_t input[], uint32_t length);
};