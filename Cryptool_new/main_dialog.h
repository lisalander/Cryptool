#pragma once
#include <vector>
#include <string>
#include <functional>
#include "crypto\crypto.h"
#include "dialog.h"

typedef struct file_context
{
	// i:input o:output
    TCHAR iFilePath[MAX_PATH];
	TCHAR oFilePath[MAX_PATH];
	HANDLE iFile = NULL;
	HANDLE oFile = NULL;
	bool is_file;
}file_context;

class main_dialog : public dialog
{
public:
	main_dialog();
	virtual ~main_dialog() {};

	/*
	   static pointer to itself
	   used in callback function
	*/

	HINSTANCE hInst;

	/* some type conversion for io */
	/*
	   convert_x_y():
	   x, y are integers, represent output and input format
           1->binary
		   4->hex
		   8->uint8_t
		   32->uint32_t
		   example : convert_32_4 is conversion from hex to uint32_t
	   assume uint32_t is little endian
	*/
	uint8_t* convert_4_8(const uint8_t *input, uint32_t length);

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
	std::vector<std::pair<std::wstring, bool>> algorithms;

	std::vector<std::wstring> filefilters;
private:
	/* basic */

	// initialization
	virtual void init();
	void set_widget_value();
	void fill_tab(HWND htab);

	// callback function
	virtual INT_PTR DialogProc(UINT message, WPARAM wParam, LPARAM lParam);

	/* io */
	void choose_open_file();
	void choose_save_file();
	int dropfile(HDROP hDrop);

	void io_main();

	// ui_tid : progress_dialog's thread id
	void run(crypto_context *c_ctx, file_context *f_ctx, DWORD ui_tid);
	std::function<void(crypto_context*, file_context*, DWORD)> t_run;

	void display(crypto *c, uint8_t *output, uint32_t output_size, DWORD ui_tid, bool is_hash, bool is_file);

	int32_t process_text(crypto *c, uint8_t *output, uint32_t *output_size);

	int32_t process_file(crypto *c, file_context *f_ctx, DWORD ui_tid);

	bool check_file(bool hash, file_context *f_ctx);
};