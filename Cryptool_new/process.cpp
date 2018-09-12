#pragma once
#include "process.h"
#include <thread>

process::process()
{
	t_run = std::bind(&process::run, this, std::placeholders::_1, std::placeholders::_2,
		                                   std::placeholders::_3, std::placeholders::_4);

	freq = new LARGE_INTEGER;
	begin = new LARGE_INTEGER;
	end = new LARGE_INTEGER;
	QueryPerformanceFrequency(freq);
}

process::~process()
{
	delete freq;
	delete begin;
	delete end;
}

uint8_t* process::convert_4_8(const uint8_t *input, uint32_t length)
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

bool process::check_file(int type, file_context *f_ctx)
{
	if (f_ctx->is_file)
	{
		if (f_ctx->iFilePath[0] != '\0')
		{
			// open input file
			f_ctx->iFile = CreateFileW(f_ctx->iFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);// | FILE_FLAG_NO_BUFFERING

			/*
			  hash algorithm's output is shown on dialog
			  it does not need to output to file
			*/
			if (type != TYPE_HASH)
			{
				if (f_ctx->oFilePath[0] != '\0')
				{
					// create output file
					f_ctx->oFile = CreateFileW(f_ctx->oFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
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
			MessageBoxW(NULL, L"failed to open file", L"error", MB_OK);
			CloseHandle(f_ctx->iFile);
			if (f_ctx->oFile == INVALID_HANDLE_VALUE && type != TYPE_HASH)
			{
				MessageBoxW(NULL, L"failed to create file", L"error", MB_OK);
				CloseHandle(f_ctx->oFile);
			}
			return false;
		}
	}
	return true;
}

// read chunk_size of data and process it
int32_t process::process_file(crypto *c, file_context *f_ctx, DWORD ui_tid)
{
	int32_t status = 0;

	// get file size
	DWORD size_high, size_low;
	size_low = GetFileSize(f_ctx->iFile, &size_high);
	f_ctx->size = ((uint64_t)size_high) << 32 | (uint64_t)size_low;

	// set number of bits of input, used for padding
	c->set_input_size((uint32_t)size_high, (uint32_t)size_low);
	PostThreadMessage(ui_tid, WM_SETSIZE, WPARAM(size_high), LPARAM(size_low));

	// processed file size
	uint64_t processed_size = 0;

	//buffer 2M, should be multiple of block size :(
	DWORD chunk_size = 0x200000;
	uint8_t *buffer = new uint8_t[chunk_size];
	uint64_t chunk_size_64 = (uint64_t)chunk_size;
	if (buffer == NULL)
		return -1;

	/*
	  result of one update
	  unused if it's hash algorithm
	  chunk_size * 2 : base64 output_size > input_size
	*/
	uint8_t *output = new uint8_t[chunk_size * 2];
	if (output == NULL)
		return -1;

	// output_size of one update
	uint32_t output_size = 0;

	// end of file
	bool eof;

	// (oFile != NULL) means (!hash && file)
	bool write = f_ctx->oFile != NULL;

	// time
	DWORD time;

	DWORD read = chunk_size, written;
	while (true)
	{
		// resolution < 1us
		QueryPerformanceCounter(begin);

		eof = false;
		if ((processed_size + chunk_size_64) >= f_ctx->size)
		{
			eof = true;
		}
		// read chunk_size bytes
		ReadFile(f_ctx->iFile, buffer, chunk_size, &read, NULL);

		// run algorithm
		status = c->update(output, buffer, read, &output_size, eof);
		if (status < 0)
			break;

		if (write)
			WriteFile(f_ctx->oFile, output, output_size, &written, NULL);

		QueryPerformanceCounter(end);
		// elapsed microseconds
		time = (DWORD)((1000000*(end->QuadPart - begin->QuadPart)) / freq->QuadPart);
		PostThreadMessageW(ui_tid, WM_UPDATEPB, WPARAM(read), LPARAM(time));

		if (eof)
			break;

		processed_size += read;
	}
	delete[]output;
	delete[]buffer;
	CloseHandle(f_ctx->iFile);
	CloseHandle(f_ctx->oFile);
	return status;
}

int32_t process::process_text(crypto *c, uint8_t *output, uint32_t *output_size)
{
	int32_t status;
	uint32_t length;
	LPSTR input = new char[4096];
	memset(input, 0, sizeof(char) * 4096);

	// assume length of input is not very long
	length = GetDlgItemTextA(hDlg, IDC_EDIT2, input, 4096);
	c->set_input_size(0, length);

	//it's the last chunk
	status = c->update((uint8_t*)output, (uint8_t*)input, length, output_size, true);

	delete[]input;
	return status;
}

// read,process,write
void process::run(crypto_context *c_ctx, file_context *f_ctx, DWORD ui_tid, DWORD m_tid)
{
	int32_t status;

	// output of block cipher from process_text()
	uint8_t *output = NULL;

	// open, create file
	bool b = check_file(c_ctx->type, f_ctx);
	if (!b)
	{
		delete c_ctx;
		delete f_ctx;
		return;
	}

	// create algorithm object
	crypto *c = CREATE_CRYPTO(c_ctx->name);
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
		uint8_t *output = new uint8_t[4096];
		memset(output, 0, sizeof(uint8_t) * 4096);
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
			uint8_t *hex = NULL;

			switch (c_ctx->type)
			{
			case TYPE_HASH: hex = c->encode(); break;
			case TYPE_SYMMETRIC: hex = convert_4_8(output, output_size); break;
			case TYPE_BASE: 
				hex = new uint8_t[output_size + 1];
				memcpy(hex, output, sizeof(uint8_t)*output_size);
				hex[output_size] = '\0';
				break;
			}

			if (f_ctx->is_file)
				// pass hex to progress_dialog
				PostThreadMessageW(ui_tid, WM_FINISH, WPARAM(hex), LPARAM(0));
			else
				// pass hex to main_dialog
			    PostThreadMessageW(m_tid, WM_FINISH, WPARAM(hex), LPARAM(0));
		}
		else
		{
			MessageBoxW(NULL, L"error", L"?", MB_OK);
		}
		delete[]output;
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
	delete this;
}

void process::start(crypto_context *c_ctx, file_context *f_ctx, pd_context *pd_ctx, DWORD m_tid)
{
	DWORD ui_tid = 0;
	if (pd_ctx != NULL)
	{
		//create thread start event
		HANDLE hstart = CreateEvent(0, FALSE, FALSE, 0);

		// start ui thread
		progress_dialog *pd = new progress_dialog();
		std::thread t_ui(&progress_dialog::start, pd, (HWND)NULL, hstart);
		ui_tid = GetThreadId(t_ui.native_handle());
		WaitForSingleObject(hstart, INFINITE);
		t_ui.detach();
		PostThreadMessageW(ui_tid, WM_SETDLG, WPARAM(pd_ctx), LPARAM(0));
	}

	std::thread t_worker(t_run, c_ctx, f_ctx, ui_tid, m_tid);
	t_worker.detach();
}

