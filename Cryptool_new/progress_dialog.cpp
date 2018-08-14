#include "progress_dialog.h"

pd_context::pd_context(std::wstring name, uint32_t key_length, int32_t nmode, bool is_enc, std::wstring path)
{
	std::wstring mode = L"";
	if (nmode >= 0)
	{
		switch (nmode)
		{
		case 0:mode = L"ECB"; break;
		case 1:mode = L"CBC"; break;
		case 2:mode = L"CFB"; break;
		case 3:mode = L"OFB"; break;
		}

		if (name == L"AES")
		{
			algorithm = name + std::to_wstring(key_length * 8) + L"_" + mode + L" " + (is_enc ? L"ENCRYPT" : L"DECRYPT");
		}
		else
		{
			algorithm = name + L"_" + mode + L" " + (is_enc ? L"ENCRYPT" : L"DECRYPT");
		}
		is_hash = false;
	}
	else
	{
		is_hash = true;
		algorithm = name;
	}
	file_path = path;
}

progress_dialog::progress_dialog()
{
	ratio_completed = 0;
	size_processed = 0;
	ac_size = 0;
	ac_time = 0;
	show_result = false;
}

void progress_dialog::start(HWND parent, HANDLE hstart)
{
	// create progress_dialog
	hDlg = CreateDialogParamW(hInst, MAKEINTRESOURCE(IDD_PROGRESS), parent, StaticDialogProc, (LPARAM)this);

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// set thread start event
	SetEvent(hstart);

	dlgmsgloop();

	delete this;
}

void progress_dialog::init()
{

}

INT_PTR progress_dialog::DialogProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		init();
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SETDLG:
		set_dlg((pd_context*)wParam);
		break;
	case WM_SETSIZE:
		set_size((uint32_t)wParam, (uint32_t)lParam);
		break;
	case WM_UPDATEPB:
		update_progress((DWORD)wParam, (DWORD)lParam);
		break;
	case WM_FINISH:
		finish((uint8_t*)wParam);
		break;
	}
	return 0;
}

void progress_dialog::update_progress(DWORD read, DWORD time)
{
	time_total += (uint64_t)time;

	// accumulate processed size(Byte) and time(microsecond)
	ac_size += (uint64_t)read;
	ac_time += (uint64_t)time;

	// show speed about every 1 second
	if (ac_time > 1000000)
	{
		show_speed(ac_size, ac_time);
	}

	// calculate ratio
	size_processed += (uint64_t)read;
	ratio_completed = (double)size_processed / (double)size_total;

	// update progress bar
	HWND hpb= GetDlgItem(hDlg, IDC_PROGRESS1);
	SendMessage(hpb, PBM_SETPOS, (WPARAM)(ratio_completed * 100), 0);
}

void progress_dialog::set_dlg(pd_context *pd_ctx)
{
	algorithm = pd_ctx->algorithm;

	// get file name
	int32_t pos = pd_ctx->file_path.find_last_of(L"\\");
	file_name = pd_ctx->file_path.substr(pos + 1);

	// show hash?
	show_result = pd_ctx->is_hash;

	SetDlgItemTextW(hDlg, IDC_STATIC7, algorithm.c_str());
	SetDlgItemTextW(hDlg, IDC_STATIC8, file_name.c_str());
	delete pd_ctx;
}

void progress_dialog::set_size(uint32_t high, uint32_t low)
{
	size_total = (uint64_t)high << 32 | (uint64_t)low;

	// size_total to string
	std::wstring integer;
	std::wstring decimal;
	std::wstring size_string;
	if (size_total >> 20 > 0)
	{
		integer = std::to_wstring(size_total >> 20);
		decimal = std::to_wstring((size_total & 0x7ffff) * 100 / (0x1 << 19));
		size_string = integer + L"." + decimal + L"M";
	}
	else if (size_total >> 10 > 0)
	{
		integer = std::to_wstring(size_total >> 10);
		decimal = std::to_wstring((size_total & 0x1ff) * 100 / (0x1 << 9));
		size_string = integer + L"." + decimal + L"K";
	}
	else
	{
		integer = std::to_wstring(size_total & 0x1ff);
		size_string = integer + L"B";
	}
	SetDlgItemTextW(hDlg, IDC_STATIC9, size_string.c_str());
}

// calculate speed = size(Byte) / time(microsecond)
void progress_dialog::show_speed(uint64_t size, uint64_t time)
{
	double speed = (double)(size) * 1000000 / (double)time;
	std::wstring speed_s;
	int32_t pos;
	if (speed >= (double)0x100000)
	{
		speed_s = std::to_wstring((speed / (double)0x100000));
		pos = speed_s.find_last_of(L".");
		speed_s = speed_s.substr(0, pos + 3) + L"MB/s";
	}
	else if (speed >= (double)0x400)
	{
		speed_s = std::to_wstring((speed / (double)0x400));
		pos = speed_s.find_last_of(L".");
		speed_s = speed_s.substr(0, pos + 3) + L"KB/s";
	}
	else
	{
		speed_s = std::to_wstring((speed / (double)0x400));
		pos = speed_s.find_last_of(L".");
		speed_s = speed_s.substr(0, pos) + L"B/s";
	}
	SetDlgItemTextW(hDlg, IDC_STATIC10, speed_s.c_str());

	//set size and time to 0
	ac_size = 0;
	ac_time = 0;
}

// hex : hash result
void progress_dialog::finish(uint8_t *hex)
{
	// show average speed
	show_speed(size_total, time_total);

	HWND hpb = GetDlgItem(hDlg, IDC_PROGRESS1);
	SendMessage(hpb, PBM_SETPOS, (WPARAM)(100), 0);

	if (show_result)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_STATIC11), SW_SHOW);
		SetDlgItemTextW(hDlg, IDC_STATIC11, algorithm.c_str());

		// show hash
		ShowWindow(GetDlgItem(hDlg, IDC_EDIT1), SW_SHOW);
		SetDlgItemTextA(hDlg, IDC_EDIT1, LPCSTR(hex));
		delete[]hex;
	}
}