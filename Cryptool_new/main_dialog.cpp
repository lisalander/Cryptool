#include "main_dialog.h"
#include "progress_dialog.h"
#include <thread>
#include <string.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")

main_dialog::main_dialog()
{

}

void main_dialog::start(HWND parent)
{
	// create main_dialog
	hDlg = CreateDialogParamW(hInst, MAKEINTRESOURCE(IDD_MAIN), parent, StaticDialogProc, (LPARAM)this);

	dlgmsgloop();
}

void main_dialog::init()
{
	HWND htab = GetDlgItem(hDlg, IDC_TAB1);
	set_widget_value();
	fill_tab(htab);

	// some initial state
	// file
	CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1); 
	// encrypt
	CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
	// ecb
	CheckRadioButton(hDlg, IDC_RADIO5, IDC_RADIO8, IDC_RADIO5);

	//
	t_run = std::bind(&main_dialog::run, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void main_dialog::fill_tab(HWND htab)
{
	TCITEMW ti;
	ti.mask = TCIF_TEXT;
	for (uint32_t i = 0; i < algorithms.size(); i++)
	{
		ti.pszText = (LPWSTR)algorithms[i].first.c_str();
		TabCtrl_InsertItem(htab, i, &ti);
	}
}

// algorithm name should be upper case
void main_dialog::set_widget_value()
{
	algorithms.emplace_back(std::make_pair(L"MD5", true));
	algorithms.emplace_back(std::make_pair(L"SHA1", true));
	algorithms.emplace_back(std::make_pair(L"SHA256", true));
	algorithms.emplace_back(std::make_pair(L"SM3", true));
	algorithms.emplace_back(std::make_pair(L"DES", false));
	algorithms.emplace_back(std::make_pair(L"AES", false));

	filefilters.emplace_back(L"all(*.*)\0*.*\0\0");
}

INT_PTR main_dialog::DialogProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	int tab_id;
	switch (message)
	{
	case WM_INITDIALOG:
		init();
		break;
	case WM_DROPFILES:
		dropfile((HDROP)wParam);
		break;
	case WM_LBUTTONDOWN:
		HWND h;
		HWND h1;
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hDlg, &point);
		h = ChildWindowFromPoint(hDlg, point);
		h1 = GetDlgItem(hDlg, IDC_STATIC2);
		if (h == h1)
		{
			choose_open_file();
		}
		h1 = GetDlgItem(hDlg, IDC_STATIC3);
		if (h == h1)
		{
			choose_save_file();
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
		if ((INT)wParam == IDC_TAB1)
		{
			if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
			{
				tab_id = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB1));
				SetDlgItemTextW(hDlg, IDC_STATIC1, (LPWSTR)algorithms[tab_id].first.c_str());
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			io_main();
			break;
		case IDC_BUTTON2:
			choose_open_file();
			break;
		case IDC_BUTTON3:
			choose_save_file();
			break;
		case IDC_RADIO1:
			CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
			break;
		case IDC_RADIO2:
			CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
			break;
		case IDC_RADIO3:
			CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
			break;
		case IDC_RADIO4:
			CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
			break;
		case IDC_RADIO5:
			CheckRadioButton(hDlg, IDC_RADIO5, IDC_RADIO8, IDC_RADIO5);
			break;
		case IDC_RADIO6:
			CheckRadioButton(hDlg, IDC_RADIO5, IDC_RADIO8, IDC_RADIO6);
			break;
		case IDC_RADIO7:
			CheckRadioButton(hDlg, IDC_RADIO5, IDC_RADIO8, IDC_RADIO7);
			break;
		case IDC_RADIO8:
			CheckRadioButton(hDlg, IDC_RADIO5, IDC_RADIO8, IDC_RADIO8);
			break;
		}
		break;
	}
	return 0;
}

// uint8_t to hex
uint8_t* main_dialog::convert_4_8(const uint8_t *input, uint32_t length)
{
	uint8_t *hex = new uint8_t[length * 2 + 1];
	hex[length * 2] = '\0';
	for (uint32_t i = 0, j = 0; j < length; i += 2, j++)
	{
		hex[i] = input[j] >> 4;
		hex[i + 1] = input[j] & 0xf;
	}
	for (uint32_t i = 0; i < length * 2; i++)
	{
		if (hex[i] < 0xA)
		{
			hex[i] += 0x30;
		}
		else
		{
			hex[i] += 0x37;
		}
	}
	return hex;
}

uint8_t* main_dialog::precheck_hex_input(uint8_t *input, uint32_t length)
{
	// key length shoule be multiple of 2 and not empty
	if (length % 2 != 0 || length == 0)
		return NULL;

	// input may contain both upper case and lower case character
	for (uint32_t j = 0; j < length; j++)
	{
		if ('a' <= input[j] && input[j] <= 'f')
		{
			input[j] -= 0x57;
		}
		else if ('A' <= input[j] && input[j] <= 'F')
		{
			input[j] -= 0x37;
		}
		else if ('0' <= input[j] && input[j] <= '9')
		{
			input[j] -= 0x30;
		}
		else
		{
			return NULL;
		}
	}
	uint8_t *output = new uint8_t[length / 2];

	// hex to uint8_t
	for (uint32_t i = 0, j = 0; j < length; i++, j += 2)
	{
		output[i] = input[j] << 4 | input[j + 1];
	}
	return output;
}

int32_t main_dialog::precheck_hex_key_and_iv(crypto_context *c_ctx, uint8_t *hex_key, uint8_t *hex_iv,
	                                                                uint32_t key_length, uint32_t iv_length)
{
	// check and convert hex key to uint8_t array
	c_ctx->user_key = precheck_hex_input(hex_key, key_length);
	if (c_ctx->user_key == NULL)
		return -1;
	c_ctx->key_length = key_length / 2;

	// ecb mode does not need iv
	if (c_ctx->mode > 0)
	{
		// check and convert hex initial vector to uint8_t array
		c_ctx->iv = precheck_hex_input(hex_iv, iv_length);
		if (c_ctx->user_key == NULL)
			return -2;
		c_ctx->iv_length = iv_length / 2;
	}
	return 0;
}

/* io */

void main_dialog::choose_open_file()
{
	WCHAR szFileName[MAX_PATH] = { 0 };
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = filefilters[0].c_str();
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;;
	ofn.nFilterIndex = 0;
	ofn.lpstrTitle = L"open file";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	if (GetOpenFileNameW(&ofn))
	{
		SetDlgItemTextW(hDlg, IDC_STATIC2, szFileName);
	}
}

void main_dialog::choose_save_file()
{
	int z;
	HANDLE oFile;
	WCHAR szFileName[MAX_PATH] = { 0 };
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = filefilters[0].c_str();
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"save file";
	ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	if (GetSaveFileNameW(&ofn))
	{
		oFile = CreateFileW(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);

		// file exists
		if (oFile != INVALID_HANDLE_VALUE)
		{
			z = MessageBoxW(NULL, L"file exists, overwrite?", NULL, MB_YESNO);
			if (z == IDYES)
			{
				SetDlgItemTextW(hDlg, IDC_STATIC3, szFileName);
			}
		}
		else
		{
			SetDlgItemTextW(hDlg, IDC_STATIC3, szFileName);
		}
		CloseHandle(oFile);
	}
}

int main_dialog::dropfile(HDROP hDrop)
{
	HWND h;
	HWND h1;
	POINT point;

	//get widget handle
	GetCursorPos(&point);
	ScreenToClient(hDlg, &point);
	h = ChildWindowFromPoint(hDlg, point);

	//get file name
	WCHAR FilePath[MAX_PATH];
	DragQueryFileW(hDrop, 0, FilePath, MAX_PATH);  

	// dragged file as input file
	h1 = GetDlgItem(hDlg, IDC_STATIC2);
	if (h == h1)
	{
		SetDlgItemTextW(hDlg, IDC_STATIC2, FilePath);
		DragFinish(hDrop);
		return 0;
	}

	// dragged file as output file
	h1 = GetDlgItem(hDlg, IDC_STATIC3);
	if (h == h1)
	{

		HANDLE oFile = CreateFileW(FilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (oFile != INVALID_HANDLE_VALUE) //file exists
		{
			int z = MessageBoxW(NULL, L"file exists, overwrite?", L" ", MB_YESNO);
			if (z == IDYES)
			{
				SetDlgItemTextW(hDlg, IDC_STATIC3, FilePath);
			}
			CloseHandle(oFile);
		}
		else
		{
			SetDlgItemTextW(hDlg, IDC_STATIC3, FilePath);
		}
		DragFinish(hDrop);
		return 0;
	}

	DragFinish(hDrop);
	return -1;
}

bool main_dialog::check_file(bool hash, file_context *f_ctx)
{
	if (f_ctx->is_file)
	{
		if (f_ctx->iFilePath[0] != '\0')
		{
			// open input file
			f_ctx->iFile = CreateFile(f_ctx->iFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);

			/*
			   hash algorithm's output is shown on main_dialog
			   so it does not need to output to file
			*/
			if (!hash)
			{
				if (f_ctx->oFilePath[0] != '\0')
				{
					// create output file
					f_ctx->oFile = CreateFile(f_ctx->oFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, NULL);
				}
				else
				{
					MessageBoxW(NULL, L"no save path", L"error", MB_OK);
					CloseHandle(f_ctx->oFile);
					return false;
				}
			}
		}
		else
		{
			MessageBoxW(NULL, L"no input file", L"error", MB_OK);
			return false;
		}

		if (f_ctx->iFile == INVALID_HANDLE_VALUE)
		{
			MessageBoxW(NULL, L"fail to open file", L"error", MB_OK);
			CloseHandle(f_ctx->iFile);
			if (f_ctx->oFile == INVALID_HANDLE_VALUE && !hash)
			{
				MessageBoxW(NULL, L"fail to create file", L"error", MB_OK);
				CloseHandle(f_ctx->oFile);
			}
			return false;
		}
	}
	return true;
}

int32_t main_dialog::process_file(crypto *c, file_context *f_ctx, DWORD ui_tid)
{
	int32_t status = 0;

	DWORD size_high;
	DWORD size_low = GetFileSize(f_ctx->iFile, &size_high);

	// set number of bits of input, used for padding
	c->set_input_size((uint32_t)size_high, (uint32_t)size_low);

	// file size
	pd_size *total = new pd_size;
	total->size_high = (uint32_t)size_high;
	total->size_low = (uint32_t)size_low;
	PostThreadMessage(ui_tid, WM_SETSIZE, WPARAM(total), LPARAM(0));

	// processed file size
	pd_size *processed = new pd_size;
	processed->size_high = 0;
	processed->size_low = 0;

	//buffer 2M, should be multiple of block size
	DWORD chunk_size = 0x200000;
	uint8_t *buffer = new uint8_t[chunk_size];

	/*
	   result of one update
	   unused if it's hash algorithm
	*/
	uint8_t *output = new uint8_t[chunk_size];

	// end of file
	bool eof;

	// (oFile != NULL) means (!hash && file)
	bool write = f_ctx->oFile != NULL;

	// output_size of one chunk
	uint32_t output_size;

	// time
	DWORD time;

	DWORD read, written;
	while (true)
	{
		// timeGetTime is preciser than GetTickCount
		time = timeGetTime();

		// read chunk_size bytes
		ReadFile(f_ctx->iFile, buffer, chunk_size, &read, NULL);

		eof = false;
		if (0xffffffffU - processed->size_low >= read)
			processed->size_low += read;
		else
		{
			processed->size_low += read;
			processed->size_high++;
		}
		// reach end of file
		if (processed->size_high == total->size_high && processed->size_low == total->size_low)
			eof = true;

		// run algorithm
		status = c->update(output, buffer, read, &output_size, eof);
		if (status < 0)
			break;

		if (write)
			WriteFile(f_ctx->oFile, output, output_size, &written, NULL);

		// assume read <= 0xffffffffU
		time = timeGetTime() - time;
		PostThreadMessage(ui_tid, WM_UPDATEPB, WPARAM(read), LPARAM(time));

		if (eof)
			break;
	}
	delete[]output;
	delete[]buffer;
	CloseHandle(f_ctx->iFile);
	CloseHandle(f_ctx->oFile);
	return status;
}

int32_t main_dialog::process_text(crypto *c, uint8_t *output, uint32_t *output_size)
{
	int32_t status;
	uint32_t length;
	LPSTR input = new char[1024];
	memset(input, 0, sizeof(char) * 1024);

	// assume length of input is not very long
	length = GetDlgItemTextA(hDlg, IDC_EDIT2, input, 1024);
	c->set_input_size(0, length);

	// length < 4M, so it's the last chunk
	status = c->update((uint8_t*)output, (uint8_t*)input, length, output_size, true);

	delete[]input;
	return status;
}

/*
   c : crypto_context
   output : output from process_text
   ui_tid : id of ui thread
*/
void main_dialog::display(crypto *c, uint8_t *output, uint32_t output_size, DWORD ui_tid, bool is_hash, bool is_file)
{
	uint8_t *hex = NULL;

	if (!is_file)
	{
		// hash algorithm has method "encode" to produce hex string from state
		if (is_hash)
		{
			hex = c->encode();
			SetDlgItemTextA(hDlg, IDC_EDIT3, LPCSTR(hex));
			delete[]hex;
		}

		// output is not NULL
		else
		{
			hex = convert_4_8(output, output_size);
			SetDlgItemTextA(hDlg, IDC_EDIT3, LPCSTR(hex));
			delete[]hex;
			delete[]output;
		}
	}
	else
	{
		hex = c->encode();
		PostThreadMessage(ui_tid, WM_FINISH, WPARAM(hex), LPARAM(0));
	}
}

/* 
    worker thread
	read,process,and write 
*/
void main_dialog::run(crypto_context *c_ctx, file_context *f_ctx, DWORD ui_tid)
{
	int32_t status;

	// output of block cipher from process_text()
	uint8_t *output = NULL;

	// open, create file
	bool b = check_file(c_ctx->is_hash, f_ctx);
	if (!b)
	{
		delete c_ctx;
		delete f_ctx;
		return;
	}

	// create algorithm object
	crypto *c = CREATE_CRYPTO(c_ctx->algorithm);
	if (c != NULL)
	{
		c->copy_context(c_ctx);

		// set round key and initial vector
		int32_t ret = c->init();
		if (ret < 0)
		{
			CloseHandle(f_ctx->iFile);
			CloseHandle(f_ctx->oFile);
			if (ret == -1)
			    MessageBoxW(NULL, L"wrong key length", L"error", MB_OK);
			if (ret == -2)
				MessageBoxW(NULL, L"wrong iv length", L"error", MB_OK);
			return;
		}

		// result of process_text
		LPSTR output = new char[1024 + 256];
		memset(output, 0, sizeof(char) * (1024 + 256));
		uint32_t output_size = 0;

		if (f_ctx->is_file)
			status = process_file(c, f_ctx, ui_tid);
		else
		{
			// output may contain '\0', so i need to know output size
			status = process_text(c, (uint8_t*)output, &output_size);
		}

		if (status == 0)
		{
			// display output in main_dialog or progress_dialog
			display(c, (uint8_t*)output, output_size, ui_tid, c_ctx->is_hash, f_ctx->is_file);
		}
		else
		{
			MessageBoxW(NULL, L"error", L"?", MB_OK);
		}

		delete c_ctx;
		delete f_ctx;
		delete c;
	}
	else
	{
		MessageBoxW(NULL, L"NO IMPLEMENTATION", L"?_?", MB_OK);
		delete c_ctx;
		delete f_ctx;
	}
}

void main_dialog::io_main()
{
	// prepare crypto_context
	int tab_id = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB1));
	std::wstring algorithm = algorithms[tab_id].first;
	bool is_hash = algorithms[tab_id].second;
	bool is_enc = IsDlgButtonChecked(hDlg, IDC_RADIO3) == BST_CHECKED;

	int32_t mode;
	if (IsDlgButtonChecked(hDlg, IDC_RADIO5) == BST_CHECKED)
		mode = 0; //ECB
	else if (IsDlgButtonChecked(hDlg, IDC_RADIO6) == BST_CHECKED)
		mode = 1; //CBC
	else if (IsDlgButtonChecked(hDlg, IDC_RADIO7) == BST_CHECKED)
		mode = 2; //CFB
	else
		mode = 3; //OFB
	crypto_context *c_ctx = new crypto_context(algorithm, mode, is_hash, is_enc);

	if (!c_ctx->is_hash)
	{
		// read key, it's hex string
		char *hex_key = new char[2048];
		uint32_t key_length = GetDlgItemTextA(hDlg, IDC_EDIT1, hex_key, 2048);

		// read initial vector, it's hex string
		char *hex_iv = new char[256];
		uint32_t iv_length = GetDlgItemTextA(hDlg, IDC_EDIT4, hex_iv, 256);

		// check hex string and convert it to uint8_t array
		int32_t ret = precheck_hex_key_and_iv(c_ctx, (uint8_t*)hex_key, (uint8_t*)hex_iv, key_length, iv_length);
		delete[]hex_key;
		delete[]hex_iv;

		// conversion failed
		if (ret < 0)
		{
			delete c_ctx;
			if (ret == -1)
			    MessageBox(NULL, L"input key is wrong", L"error", MB_OK);
			if (ret == -2)
				MessageBox(NULL, L"input initial vector is wrong", L"error", MB_OK);
			return;
		}
	}

	// prepare file_context
	file_context *f_ctx = new file_context;
	bool is_file = IsDlgButtonChecked(hDlg, IDC_RADIO1) == BST_CHECKED;
	f_ctx->is_file = is_file;
	if (is_file)
	{
		// input file path
		GetDlgItemTextW(hDlg, IDC_STATIC2, f_ctx->iFilePath, MAX_PATH);
		// output file path
		GetDlgItemTextW(hDlg, IDC_STATIC3, f_ctx->oFilePath, MAX_PATH);
	}

	// if input is file, create progress dialog
	DWORD ui_tid = 0;
	if (is_file)
	{
		//create thread start event
		HANDLE hstart = CreateEvent(0, FALSE, FALSE, 0);

		progress_dialog *pd = new progress_dialog();
		std::thread t_ui(&progress_dialog::start, pd, (HWND)NULL, hstart);
		ui_tid = GetThreadId(t_ui.native_handle());
		WaitForSingleObject(hstart, INFINITE);
		t_ui.detach();

		// prepare pd_context
		pd_context *pd_ctx = new pd_context(c_ctx->algorithm, c_ctx->key_length, c_ctx->mode, c_ctx->is_enc, f_ctx->iFilePath);
		PostThreadMessage(ui_tid, WM_SETDLG, WPARAM(pd_ctx), LPARAM(0));
	}

	std::thread t_worker(t_run, c_ctx, f_ctx, ui_tid);
	t_worker.detach();
}

