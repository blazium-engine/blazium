#include "pty_proxy_win.h"
#include <process.h>
#include <cstdio>
#include <mutex>

static const int STATE_NONE = 0;
static const int STATE_RUNNING = 1;
static const int STATE_EXITING = 2;

unsigned __stdcall RunToThread(void *arg) {
	PtyProxyWin *proxy = (PtyProxyWin *)arg;
	proxy->_run_to_thread();
	return 0;
}

unsigned __stdcall RunFromThread(void *arg) {
	PtyProxyWin *proxy = (PtyProxyWin *)arg;
	proxy->_run_from_thread();
	return 0;
}

PtyProxyWin::PtyProxyWin() {
	_pty_console = { INVALID_HANDLE_VALUE };
	_to_pty = { INVALID_HANDLE_VALUE };
	_from_pty = { INVALID_HANDLE_VALUE };
	_startup_info = STARTUPINFOEX{};
	_startup_info.lpAttributeList = nullptr;
	_client.hThread = { INVALID_HANDLE_VALUE };
	_client.hProcess = { INVALID_HANDLE_VALUE };

	_state = STATE_NONE;
	_to_buffer_pos = 0;
	_to_thread_started = false;

	_from_thread_started = false;

	_set_state(STATE_RUNNING);
	_create_pipes_and_pty();
	_create_process();
	_create_to_pty_thread();
	_create_from_pty_thread();

	while (!_to_thread_started || !_from_thread_started) {
		std::unique_lock<std::mutex> lock(_start_complete_mutex);
		_start_complete.wait(lock);
	}

	// TODO - wait on synchronization variable that indicates
	//        both to and from threads have started.

	/*
	DWORD consoleMode {};
	HANDLE console = { GetStdHandle(STD_OUTPUT_HANDLE) };
	GetConsoleMode(console, &consoleMode);
	if (SetConsoleMode(console, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
		_set_state(STATE_RUNNING);
		_create_pipes_and_pty();
		_create_process();
		_create_to_pty_thread();
		_create_from_pty_thread();
	} else {
		_print_last_error("unable to set console mode:");
	}
	*/
}

PtyProxyWin::~PtyProxyWin() {
	_exit_pty();
}

void PtyProxyWin::_exit_pty() {
	_set_state(STATE_EXITING);
	_wake_to_thread();
	_wake_from_thread();
	_join_threads();
	_terminate_process();
	_close_pty();

	_handle_pty_exited();
}

void PtyProxyWin::_set_state(int p_state) {
	const std::lock_guard<std::mutex> lock(_state_mutex);
	_state = p_state;
}

void PtyProxyWin::_wake_to_thread() {
	if (!CancelSynchronousIo(_to_thread)) {
		if (GetLastError() != ERROR_NOT_FOUND) {
			_print_last_error("unable to cancel i/o on write thread");
		}
	}
	const std::lock_guard<std::mutex> lock(_to_mutex);
	_to_ready.notify_all();
}

void PtyProxyWin::_notify_to_started() {
	const std::lock_guard<std::mutex> lock(_start_complete_mutex);
	_to_thread_started = true;
	_start_complete.notify_all();
}

void PtyProxyWin::_notify_from_started() {
	const std::lock_guard<std::mutex> lock(_start_complete_mutex);
	_from_thread_started = true;
	_start_complete.notify_all();
}

void PtyProxyWin::_wake_from_thread() {
	if (!CancelSynchronousIo(_from_thread)) {
		_print_last_error("unable to cancel i/o on read thread");
	}
}

void PtyProxyWin::_join_threads() {
	HANDLE thread_handles[2];
	thread_handles[0] = _to_thread;
	thread_handles[1] = _from_thread;
	if (WaitForMultipleObjects(2, thread_handles, TRUE, INFINITE) != S_OK) {
		_print_last_error("unable to wait on thread handles");
	}
}

void PtyProxyWin::_terminate_process() {
	if (_client.hProcess != INVALID_HANDLE_VALUE) {
		TerminateProcess(_client.hProcess, 0);
		CloseHandle(_client.hProcess);
		_client.hProcess = INVALID_HANDLE_VALUE;
	}
	if (_client.hThread != INVALID_HANDLE_VALUE) {
		CloseHandle(_client.hThread);
		_client.hThread = INVALID_HANDLE_VALUE;
	}
	if (_startup_info.lpAttributeList) {
		DeleteProcThreadAttributeList(_startup_info.lpAttributeList);
		free(_startup_info.lpAttributeList);
		_startup_info.lpAttributeList = nullptr;
	}
}

void PtyProxyWin::_close_pty() {
	if (_pty_console != INVALID_HANDLE_VALUE) {
		ClosePseudoConsole(_pty_console);
		_pty_console = { INVALID_HANDLE_VALUE };
	}
	if (_to_pty != INVALID_HANDLE_VALUE) {
		CloseHandle(_to_pty);
	}
	if (_from_pty != INVALID_HANDLE_VALUE) {
		CloseHandle(_from_pty);
	}
}

int PtyProxyWin::send_string(const char *data) {
	const std::lock_guard<std::mutex> lock(_to_mutex);

	int len = strlen(data);
	int old_size = _to_buffer_pos;
	if (_to_buffer_pos + len > TO_BUFFER_MAX_SIZE) {
		fprintf(stderr, "Too much data, not appending\n");
		return -1;
	}
	std::memcpy(&_to_buffer[_to_buffer_pos], data, len);
	_to_buffer_pos += len;
	_to_ready.notify_all();
	return old_size;
}

int PtyProxyWin::available_to_send() {
	const std::lock_guard<std::mutex> lock(_to_mutex);
	return TO_BUFFER_MAX_SIZE - _to_buffer_pos;
}

void PtyProxyWin::resize_screen(int nrows, int ncols) {
	_num_rows = nrows;
	_num_cols = ncols;
	COORD console_size{};
	console_size.X = _num_cols;
	console_size.Y = _num_rows;
	HRESULT result = ResizePseudoConsole(_pty_console, console_size);
	if (result != S_OK) {
		_print_last_error("unable to resize pseudo console");
	}
	if (_renderer != nullptr) {
		_apply_resize();
	}
}

void PtyProxyWin::_create_to_pty_thread() {
	_to_thread = (HANDLE)_beginthreadex(NULL, 0, &RunToThread, this, 0, NULL);
}

void PtyProxyWin::_create_from_pty_thread() {
	_from_thread = (HANDLE)_beginthreadex(NULL, 0, &RunFromThread, this, 0, NULL);
}

bool PtyProxyWin::_can_read() {
	const std::lock_guard<std::mutex> lock(_state_mutex);
	if (_state == STATE_RUNNING) {
		return true;
	}
	return false;
}

const static int MAX_READ_BUFFER = 1024;

void PtyProxyWin::_run_from_thread() {
	_notify_from_started();
	while (_can_read()) {
		DWORD read_bytes;
		unsigned char buffer[MAX_READ_BUFFER]{};
		BOOL result = ReadFile(_from_pty, buffer, MAX_READ_BUFFER, &read_bytes, NULL);
		_handle_from_pty(buffer, read_bytes);
	}
}

bool PtyProxyWin::_can_write() {
	const std::lock_guard<std::mutex> lock(_state_mutex);
	if (_state == STATE_RUNNING) {
		return true;
	}
	return false;
}

void PtyProxyWin::_run_to_thread() {
	_notify_to_started();
	while (_can_write()) {
		std::unique_lock<std::mutex> lock(_to_mutex);
		if (_to_buffer_pos == 0) {
			_to_ready.wait(lock);
		}
		if (_to_buffer_pos > 0) {
			int cur_len = _to_buffer_pos;
			int cur_pos = 0;
			lock.unlock();

			DWORD bytes_written{};
			while (cur_pos < cur_len) {
				if (WriteFile(_to_pty, &_to_buffer[cur_pos], cur_len, &bytes_written, NULL)) {
					cur_pos += bytes_written;
				} else {
					_print_last_error("unable to write to pseudo terminal");
					return;
				}
			}

			lock.lock();
			if (cur_len >= _to_buffer_pos) {
				_to_buffer_pos = 0;
			} else {
				memcpy(_to_buffer, _to_buffer + cur_len, _to_buffer_pos - cur_len);
				_to_buffer_pos -= cur_len;
			}
		}
	}
}

void PtyProxyWin::_create_process() {
	_init_startup_info();

	wchar_t szCommand[4];
	std::memcpy(szCommand, L"cmd", sizeof(wchar_t) * 4);
	if (!CreateProcess(NULL, szCommand, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT,
				NULL, NULL, &_startup_info.StartupInfo, &_client)) {
		_print_last_error("unable to create process");
		return;
	}
}

void PtyProxyWin::_create_pipes_and_pty() {
	HANDLE input;
	HANDLE output;
	if (CreatePipe(&input, &_to_pty, NULL, 0) && CreatePipe(&_from_pty, &output, NULL, 0)) {
		COORD consoleSize{};
		consoleSize.X = _num_cols;
		consoleSize.Y = _num_rows;

		if (CreatePseudoConsole(consoleSize, input, output, 0, &_pty_console) != S_OK) {
			_print_last_error("unable to create pseudo console");
			return;
		}
		if (input != INVALID_HANDLE_VALUE) {
			CloseHandle(input);
		}
		if (output != INVALID_HANDLE_VALUE) {
			CloseHandle(output);
		}
	}
}

void PtyProxyWin::_print_last_error(const char *prefix) {
	HRESULT errorMessageID = GetLastError();
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	fprintf(stderr, "%s: %s", prefix, messageBuffer);
	LocalFree(messageBuffer);
}

HRESULT CreatePseudoConsoleAndPipes(HPCON *phPC, HANDLE *phPipeIn, HANDLE *phPipeOut) {
	HRESULT hr{ E_UNEXPECTED };
	HANDLE hPipePTYIn{ INVALID_HANDLE_VALUE };
	HANDLE hPipePTYOut{ INVALID_HANDLE_VALUE };

	// Create the pipes to which the ConPTY will connect
	if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
			CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0)) {
		// Determine required size of Pseudo Console
		COORD consoleSize{};
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		HANDLE hConsole{ GetStdHandle(STD_OUTPUT_HANDLE) };
		if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}

		// Create the Pseudo Console of the required size, attached to the PTY-end of the pipes
		hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, phPC);

		// Note: We can close the handles to the PTY-end of the pipes here
		// because the handles are dup'ed into the ConHost and will be released
		// when the ConPTY is destroyed.
		if (INVALID_HANDLE_VALUE != hPipePTYOut) {
			CloseHandle(hPipePTYOut);
		}
		if (INVALID_HANDLE_VALUE != hPipePTYIn) {
			CloseHandle(hPipePTYIn);
		}
	}

	return hr;
}

void PtyProxyWin::_init_startup_info() {
	size_t attrListSize{};

	_startup_info.StartupInfo.cb = sizeof(STARTUPINFOEX);

	// Get the size of the thread attribute list.
	InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

	// Allocate a thread attribute list of the correct size
	_startup_info.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

	// Initialize thread attribute list
	if (_startup_info.lpAttributeList) {
		if (InitializeProcThreadAttributeList(_startup_info.lpAttributeList, 1, 0, &attrListSize)) {
			// Set Pseudo Console attribute
			if (!UpdateProcThreadAttribute(_startup_info.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
						_pty_console, sizeof(HPCON), NULL, NULL)) {
				_print_last_error("Unable to update proc thread attribute");
			}
		} else {
			_print_last_error("unable to initialize attribute list");
		}
	} else {
		fprintf(stderr, "Unable to allocate attribute list\n");
	}
}
