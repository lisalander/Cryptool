#include "dialog.h"

INT_PTR CALLBACK dialog::StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	dialog* self = NULL;
	if (message == WM_INITDIALOG)
	{
		// lParam is dialog pointer here
		self = (dialog*)lParam;
		self->hDlg = hDlg;

		// store the pointer
		SetWindowLongPtrW(hDlg, DWLP_USER, lParam);
	}
	else
	{
		// get the pointer
		self = (dialog*)GetWindowLongPtrW(hDlg, DWLP_USER);
	}

	if (self)
		return self->DialogProc(message, wParam, lParam);

	return 0;
}

void dialog::dlgmsgloop()
{
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (msg.message > WM_USER && msg.message < WM_USER + 5)
			msg.hwnd = hDlg;
		if (true)//IsDialogMessage(hDlg, &msg)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}