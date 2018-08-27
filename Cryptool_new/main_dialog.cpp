#include "main_dialog.h"
#include "progress_dialog.h"
#include <thread>
//#include <string.h>

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

	m_tid = GetCurrentThreadId();
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
	algorithms.emplace_back(std::make_pair(L"RSA", false));

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
	case WM_FINISH:
		// show result in dialog
		display(LPCSTR(wParam));
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
			prepare();
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

int32_t main_dialog::dropfile(HDROP hDrop)
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

// display output of process_text
void main_dialog::display(LPCSTR hex)
{
	SetDlgItemTextA(hDlg, IDC_EDIT3, hex);
	delete[]hex;
}

void main_dialog::prepare()
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

		// precheck failed
		if (ret < 0)
		{
			delete c_ctx;
			if (ret == -1)
			    MessageBox(NULL, L"key is wrong", L"error", MB_OK);
			if (ret == -2)
				MessageBox(NULL, L"initial vector is wrong", L"error", MB_OK);
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

	pd_context *pd_ctx = NULL;
	// if input is file, prepare pd_context
	if (is_file)
		pd_ctx = new pd_context(c_ctx->algorithm, c_ctx->key_length, c_ctx->mode, c_ctx->is_enc, f_ctx->iFilePath);

	// start
	process *pc = new process();
	pc->hDlg = hDlg;
	pc->start(c_ctx, f_ctx, pd_ctx, m_tid);
}

