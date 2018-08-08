#pragma once
#include <string>
#include "dialog.h"

#define WM_SETDLG WM_USER+1
#define WM_SETSIZE WM_USER+2
#define WM_UPDATEPB WM_USER+3
#define WM_FINISH WM_USER+4

// pd_size* as wParam passed to ui_thread
typedef struct pd_size
{
	uint32_t size_high;
	uint32_t size_low;
}pd_size;

// pd_context* as wParam passed to ui_thread
typedef struct pd_context
{
	pd_context() {}
	pd_context(std::wstring name, uint32_t key_length, int32_t nmode, bool is_enc, std::wstring path);

	std::wstring algorithm;
	std::wstring file_path;
	bool is_hash;
}pd_context;

class progress_dialog : public dialog
{
public:
	progress_dialog();

	virtual void init();

	// create dialog, run message loop
	void start(HWND parent, HANDLE hstart);

	HINSTANCE hInst;
private:
	progress_dialog *self;

	// input size
	uint64_t size_total;

	// processed input size
	uint64_t size_processed;

	DWORD time_total;

	/* 
	    accumulated processed size and time
		used to calculate speed
	*/
	uint64_t ac_size;
	DWORD ac_time;

	// calculate and show speed
	void show_speed(uint64_t size, DWORD time);

	std::wstring algorithm;
	std::wstring file_name;

	// show hash result?
	bool show_result;

	// 
	double ratio_completed;

	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void set_dlg(pd_context *pd_ctx);

	void set_size(pd_size *size);

	void update_progress(DWORD read, DWORD time);

	void finish(uint8_t *hex);
};