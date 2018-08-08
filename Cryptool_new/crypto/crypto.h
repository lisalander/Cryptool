#pragma once
#include <ctype.h>
#include <map>
#include <algorithm>

typedef struct crypto_context
{
	// constructor
	crypto_context() {}
	crypto_context(std::wstring s, int32_t m, bool h, bool b);

	// algorithm name
	std::wstring algorithm;

	/*
	   mode 
	   0 : ecb
	   1 : cbc
	   2 : cfb
	   3 : ofb
	*/
	int32_t mode;

	// input key and key length
	uint8_t *user_key = NULL;
	uint32_t key_length;

	/* 
	   initial vector, it will be copied to subclass because type might be diffrent:
	   aes : uint8_t*, des : uint32_t
	*/
	uint8_t *iv = NULL;
	uint32_t iv_length;

	// hash or not
	bool is_hash; 

	// enc or dec
	bool is_enc;
}crypto_context;

class crypto
{
public:
	crypto() {}
	virtual ~crypto() {}

	// algorithm name
	std::wstring algorithm;

	// input size is 64bit
	void set_input_size(uint32_t high, uint32_t low);

	//
	void copy_context(const crypto_context *ctx);

	// only for hash algorithm, convert state to hex
	virtual uint8_t* encode() { return NULL; }

	// set initial state, encryption key, decryption key and initial vector
	virtual int32_t init() { return 0; };

	/*
	    process one chunk
		output : output, unused if it's hash algorithm
	    input : input chunk
	    length : input chunk size
	    last : the last chunk?

		maybe it should return integer indicating success or failure
	*/
	virtual int32_t update(uint8_t *output, const uint8_t *input, uint32_t length, uint32_t *output_size, bool last) = 0;
protected:

	// size of input
	uint32_t input_size_high;
	uint32_t input_size_low;

	crypto_context c_ctx;
};

class crypto_factory_impl_base;

class crypto_factory
{
public:
	// map name to algorithm
	typedef std::map<std::wstring, typename crypto_factory_impl_base*> FactoryImplMap;

	static crypto_factory &instance()
	{
		static crypto_factory factory;
		return factory;
	}

	//register [name, algorithm]
	void RegisterFactoryImpl(const std::wstring& name, crypto_factory_impl_base *impl)
	{
		std::wstring up = name;
		std::transform(name.begin(), name.end(), up.begin(), ::toupper);
		map.insert(std::make_pair(up, impl));
	}

	crypto* create(const std::wstring& name);
private:
	FactoryImplMap map;
};

//
class crypto_factory_impl_base
{
public:
	explicit crypto_factory_impl_base(const std::wstring& name)
	{
		crypto_factory::instance().RegisterFactoryImpl(name, this);
	}

	~crypto_factory_impl_base() {}

	virtual crypto* create() = 0;
};

//
template <typename T>
class crypto_factory_impl : crypto_factory_impl_base
{
public:
	crypto_factory_impl(const std::wstring& name) : crypto_factory_impl_base(name) {}
	//~crypto_factory_impl() {}
	T* create()
	{
		T *p = new T();
		return p;
	}
};

/*
   register algorithm:
   REGISTER_CRYPTO(name)
   name is name of algorithm class, it's lower case
*/
#define REGISTER_CRYPTO(name) static crypto_factory_impl<name> crypto_factory_##name(L#name);

/*
   create algorithm object:
   CREATE_CRYPTO(name)
*/
#define CREATE_CRYPTO(name) crypto_factory::instance().create(name)




