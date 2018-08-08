#pragma once
#include <windows.h>
#include <Commctrl.h>
#include <windowsx.h>
#include <winuser.h>
#include "resource.h"

// dialog base 
class dialog
{
public:
	dialog() {};
	virtual ~dialog() {};

	// handle of this dialog
	HWND hDlg;
protected:

	static INT_PTR CALLBACK StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	void dlgmsgloop();

	virtual void init() = 0;

	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};