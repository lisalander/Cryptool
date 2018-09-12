#include <thread>
#include <mutex>
#include "crypto.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "aes.h"

crypto* crypto_factory::create(const std::wstring& name)
{
	std::wstring up = name;
	std::transform(name.begin(), name.end(), up.begin(), ::toupper);
	auto it = map.find(up);
	if (it == map.end())
		return NULL;
	return it->second->create();
}

crypto_context::crypto_context(std::wstring s, int m, int t, bool b)
{
	name = s;
	if (t != TYPE_SYMMETRIC)
	    mode = -1;
	else
		mode = m;
	type = t;
	is_enc = b;
}

void crypto::set_input_size(uint32_t high, uint32_t low)
{
	input_size_high = (high << 3) | ((low & 0xE0000000) >> 29);
	input_size_low = low << 3;
}

void crypto::copy_context(const crypto_context *ctx)
{
	name = ctx->name;
	c_ctx.name = ctx->name;
	c_ctx.is_enc = ctx->is_enc;
	c_ctx.type = ctx->type;
	c_ctx.mode = ctx->mode;
	c_ctx.key_length = ctx->key_length;
	c_ctx.user_key = ctx->user_key;
	c_ctx.iv_length = ctx->iv_length;
	c_ctx.iv = ctx->iv;
}

REGISTER_CRYPTO(base64);
REGISTER_CRYPTO(md5);
REGISTER_CRYPTO(sha1);
REGISTER_CRYPTO(sha256);
REGISTER_CRYPTO(aes);


