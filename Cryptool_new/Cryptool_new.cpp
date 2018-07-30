#include "Cryptool_new.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	//main_dialog *d = new main_dialog();
	main_dialog d;
	std::thread t(&main_dialog::start, d, (HWND)NULL);
	t.join();

	//delete d;
	return 0;
}