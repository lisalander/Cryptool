#pragma once
#include <vector>
#include <string>
#include <functional>
#include "dialog.h"
#include "process.h"

class main_dialog : public dialog
{
public:
	main_dialog();
	virtual ~main_dialog() {};

	HINSTANCE hInst;

	uint8_t* precheck_hex_input(uint8_t *input, uint32_t length);

	/* 
	   user key and initial vector are hex string
	   check user key/initial vector is legal or not
	   and convert user key/initial vector to uint8_t array
	 */
	int32_t precheck_hex_key_and_iv(crypto_context *c_ctx, uint8_t *hex_key, uint8_t *hex_iv, 
		                                                 uint32_t key_length, uint32_t iv_length);

	// 
	void start(HWND parent);
protected:
	// (algorithm, is_hash)
	std::vector<std::pair<std::wstring, int>> ciphers;

	std::vector<std::wstring> filefilters;
private:
	// main thread id
	DWORD m_tid;

	// initialization
	virtual void init();
	void set_widget_value();
	void fill_tab(HWND htab);

	// callback function
	virtual INT_PTR DialogProc(UINT message, WPARAM wParam, LPARAM lParam);

	void choose_open_file();
	void choose_save_file();
	int32_t dropfile(HDROP hDrop);

	void prepare();

	void display(LPCSTR hex);
};