#pragma once
#include "crypto.h"

class base64 : public crypto
{
public:
	base64() {};
	virtual ~base64() {}

	// init state
	virtual int32_t init();

	virtual int32_t update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last);
private:
	// internal buffer, stores previous input's last 1 or 2 or 3 bytes
	uint8_t buffer[4];

	// 0 or 1 or 2 or 3
	uint32_t bytes;

	// 64 Bytes
	const uint32_t block_size = 64;

	void inline b3_b4(uint32_t t, uint8_t *output);

	void inline b4_b3(uint32_t t, uint8_t *output);

	int32_t encode(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last);

	int32_t decode(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last);

	// which function to use in update, it depends on mode
	int32_t (base64::*func)(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last);

	// process the last few bytes
	void enc_finalize(uint8_t *output);

	static const uint8_t encode_table[64];

	static const uint8_t decode_table[128];
};
