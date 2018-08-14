#pragma once
#include <windows.h>
#include <Winbase.h>
#include <string>
#include <functional>
#include "crypto\crypto.h"
#include "progress_dialog.h"

typedef struct file_context
{
	// i:input o:output
	wchar_t iFilePath[MAX_PATH];
	wchar_t oFilePath[MAX_PATH];
	HANDLE iFile = NULL;
	HANDLE oFile = NULL;
	bool is_file;
}file_context;

class process
{
public:
	process();
	~process();
	void start(crypto_context *c_ctx, file_context *f_ctx, pd_context *pd_ctx, DWORD m_tid);
private:
	HWND hDlg;

	// acquire high-resolution time stamps
	LARGE_INTEGER *freq;
	LARGE_INTEGER *begin;
	LARGE_INTEGER *end;

	uint8_t* convert_4_8(const uint8_t *input, uint32_t length);

	int32_t process_text(crypto *c, uint8_t *output, uint32_t *output_size);

	int32_t process_file(crypto *c, file_context *f_ctx, DWORD ui_tid);

	bool check_file(bool hash, file_context *f_ctx);

	// ui_tid : progress_dialog's thread id
	void run(crypto_context *c_ctx, file_context *f_ctx, DWORD ui_tid, DWORD m_tid);
	std::function<void(crypto_context*, file_context*, DWORD, DWORD)> t_run;
};