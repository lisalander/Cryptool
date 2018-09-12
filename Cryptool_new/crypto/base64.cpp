#include "base64.h"

int32_t base64::init()
{
	buffer[0] = 0x0;
	buffer[1] = 0x0;
	bytes = 0;
	if (c_ctx.is_enc)
	{
		func = &base64::encode;
	}
	else
	{
		func = &base64::decode;
	}
	return 0;
}

/* encode */

// 3 bytes to 4 bytes
void base64::b3_b4(uint32_t t, uint8_t *output)
{
	output[0] = encode_table[t >> 18];
	output[1] = encode_table[(t >> 12) & 0x3f];
	output[2] = encode_table[(t >> 6) & 0x3f];
	output[3] = encode_table[t & 0x3f];
}

void base64::enc_finalize(uint8_t *output)
{
	uint32_t t = 0;
	if (bytes == 1)
	{
		output[0] = encode_table[buffer[0] >> 2];
		output[1] = 'Q';
		output[2] = '=';
		output[3] = '=';
	}
	if (bytes == 2)
	{
		t = buffer[0] << 16;
		t |= buffer[1] << 8;
		b3_b4(t, output);
		output[3] = '=';
	}
}

int32_t base64::encode(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
{
	length += bytes;
	uint32_t t = 0;
	uint8_t *start = output;

	// is this implementation good? :(
	if (length >= 3)
	{
		// process fisrt 3 bytes...
		if (bytes == 1)
		{
			t = buffer[0] << 16;
			t |= (input[0] << 8);
			t |= input[1];
			b3_b4(t, output);
			output += 4;
			input += 3;
			length -= 3;
		}
		else if (bytes == 2)
		{
			t = buffer[0] << 16;
			t |= buffer[1] << 8;
			t |= input[0];
			b3_b4(t, output);
			output += 4;
			input += 3;
			length -= 3;
		}
		while (length >= 3)
		{
			t = input[0] << 16;
			t |= (input[1] << 8);
			t |= input[2];
			b3_b4(t, output);
			output += 4;
			input += 3;
			length -= 3;
		}
		memcpy(buffer, input, length);
		bytes = length;
	}
	else
	{
		memcpy(buffer + bytes, input, length - bytes);
		bytes = length;
	}
	*output_size = (output - start);

	if (last && (bytes > 0))
	{
		*output_size += 4;
		enc_finalize(output);
	}
	return 0;
}

/* decode */

// 4 bytes to 3 bytes
void base64::b4_b3(uint32_t t, uint8_t *output)
{
	output[0] = t >> 16;
	output[1] = (t >> 8) & 0xff;
	output[2] = t & 0xff;
}

int32_t base64::decode(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
{
	length += bytes;
	uint32_t t = 0;
	uint8_t *start = output;

	// is this implementation good? :(
	if (length >= 4)
	{
		// decode fisrt 4 bytes...
		if (bytes == 1)
		{
			buffer[1] = input[0];
			buffer[2] = input[1];
			buffer[3] = input[2];
		}
		else if (bytes == 2)
		{
			buffer[2] = input[0];
			buffer[3] = input[1];
		}
		else if (bytes == 3)
		{
			buffer[3] = input[0];
		}
		if (bytes > 0)
		{
			t = decode_table[buffer[0]] << 18;
			t |= decode_table[buffer[1]] << 12;
			t |= decode_table[buffer[2]] << 6;
			t |= decode_table[buffer[3]];
			b4_b3(t, output);
			output += 3;
			input += 4;
			length -= 4;
		}

		// then bytes in input
		while (length >= 4)
		{
			t = decode_table[input[0]] << 18;
			t |= decode_table[input[1]] << 12;
			t |= decode_table[input[2]] << 6;
			t |= decode_table[input[3]];
			b4_b3(t, output);
			output += 3;
			input += 4;
			length -= 4;
		}
		memcpy(buffer, input, length);
		bytes = length;
	}
	else
	{
		memcpy(buffer + bytes, input, length - bytes);
		bytes = length;
	}
	*output_size = (output - start);

	if (last)
	{
		// bytes should be 0 because length % 4 should be 0
		if (bytes > 0)
			return -1;

		if (*(output - 1) == '\0')
			(*output_size)--;
		if (*(output - 2) == '\0')
			(*output_size)--;
	}
	return 0;
}

int32_t base64::update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last)
{
	return (this->*func)(output, input, length, output_size, last);
}











const uint8_t base64::encode_table[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

const uint8_t base64::decode_table[128] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//  0..15
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//  16..31
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 255, 255,  63,		//  32..47
	52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 0, 255, 255,		//  48..63
	255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,		//  64..79
	15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 255,		//  80..95
	255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,		//  96..111
	41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255,		//  112..127
};
