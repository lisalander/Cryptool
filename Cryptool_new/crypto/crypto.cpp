#include <thread>
#include <mutex>
#include "crypto.h"
#include "md5.h"
#include "sha1.h"
#include "aes.h"

crypto* crypto_factory::create(const std::wstring& name)
{
	auto it = map.find(name);
	if (it == map.end())
		return NULL;
	return it->second->create();
}

crypto_context::crypto_context(std::wstring name, int32_t m, bool h, bool b)
{
	algorithm = name;
	if (h)
	    mode = -1;
	else
		mode = m;
	is_hash = h;
	is_enc = b;
}

void crypto::set_input_size(uint32_t high, uint32_t low)
{
	input_size_high = (high << 3) | ((low & 0xE0000000) >> 29);
	input_size_low = low << 3;
}

void crypto::copy_context(const crypto_context *ctx)
{
	algorithm = ctx->algorithm;
	c_ctx.algorithm = ctx->algorithm;
	c_ctx.is_enc = ctx->is_enc;
	c_ctx.is_hash = ctx->is_hash;
	c_ctx.mode = ctx->mode;
	c_ctx.key_length = ctx->key_length;
	c_ctx.user_key = ctx->user_key;
	c_ctx.iv_length = ctx->iv_length;
	c_ctx.iv = ctx->iv;
}

REGISTER_CRYPTO(md5);
REGISTER_CRYPTO(sha1);
REGISTER_CRYPTO(aes);


