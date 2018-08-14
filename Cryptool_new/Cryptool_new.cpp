#include "Cryptool_new.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	main_dialog *md = new main_dialog();
	std::thread t(&main_dialog::start, md, (HWND)NULL);
	t.join();

	delete md;
	return 0;
}