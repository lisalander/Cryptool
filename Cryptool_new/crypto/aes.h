#pragma once
#include "crypto.h"

/*
    lisence

*/

// 0xAABBCCDD -> 0xDDCCBBAA
#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)

#define GETU32(p) SWAP(*((uint32_t*)(p)))

#define PUTU32(ct, st) { *((uint32_t *)(ct)) = SWAP((st)); }

class aes : public crypto
{
public:
	aes();
	virtual ~aes() {}

	virtual int32_t init();

	virtual void update(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);
private:
	uint8_t buffer[16];

	const uint32_t block_size = 16;

	// initial vector, it's treated as the result of previous encryption or decryption
	uint8_t *iv = NULL;

	typedef struct aes_key
	{
		// encryption round key or decryption round key, 60 is maximum size of key
		uint32_t rd_key[60];

		uint32_t rounds;
	}aes_key;
	aes_key key;

	static const uint32_t Te0[256];
	static const uint32_t Te1[256];
	static const uint32_t Te2[256];
	static const uint32_t Te3[256];
	static const uint32_t Td0[256];
	static const uint32_t Td1[256];
	static const uint32_t Td2[256];
	static const uint32_t Td3[256];
	static const uint8_t Td4[256];
	static const uint32_t rcon[10];

	// expand encryption key
	int32_t set_enc_key(uint8_t *user_key, uint32_t length);

	// expand decryption key
	int32_t set_dec_key(uint8_t *user_key, uint32_t length);

	// initial vector
	int32_t set_initial_vector(uint8_t *ivec, uint32_t length);

	// which function to use in update, it depends on mode
	void (aes::*func)(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);

	// encryption
	void encrypt(uint8_t *output, const uint8_t *input);

	// decryption
	void decrypt(uint8_t *output, const uint8_t *input);

	// ecb
	void ecb_encrypt(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);
	void ecb_decrypt(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);

	// cbc
	void cbc_encrypt(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);
	void cbc_decrypt(uint8_t *output, const uint8_t *input, uint32_t length, DWORD *output_size, bool last);
};

