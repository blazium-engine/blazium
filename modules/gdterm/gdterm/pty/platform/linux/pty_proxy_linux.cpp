#include "pty_proxy_linux.h"
#include "poller.h"
#include <cstdlib>
#include <cstring>
#ifdef USE_PTY_UTIL_H
#include <util.h>
#else
#include <pty.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>

PtyProxyLinux::PtyProxyLinux() {
	_init_pty();
}

PtyProxyLinux::~PtyProxyLinux() {
	if (_poller != nullptr) {
		delete _poller;
		_poller = nullptr;
	}
}

/**
 * This is invoked on the poller thread, so it is safe to manipulate the TMT data
 * structure and the file descriptor for the pseudo terminal
 */
void PtyProxyLinux::apply_size() {
	struct winsize size;
	size.ws_col = _num_cols;
	size.ws_row = _num_rows;
	size.ws_xpixel = 0;
	size.ws_ypixel = 0;
	ioctl(_pty_fd, TIOCSWINSZ, &size);
	_apply_resize();
}

void PtyProxyLinux::handle(unsigned char *data, int data_len) {
	_handle_from_pty(data, data_len);
}

void PtyProxyLinux::exited() {
	_handle_pty_exited();
	if (_renderer != nullptr) {
		_renderer->exited();
	}
}

void PtyProxyLinux::_init_pty() {
	struct winsize wsize;

	wsize.ws_col = _num_cols;
	wsize.ws_row = _num_rows;
	wsize.ws_xpixel = 0;
	wsize.ws_ypixel = 0;

	pid_t pid = forkpty(&_pty_fd, nullptr, nullptr, &wsize);
	if (!pid) {
		static char termstr[] = "TERM=ansi";
		putenv(termstr);
		execl(std::getenv("SHELL"), std::getenv("SHELL"), "-l", "-i", nullptr);
	}
	fcntl(_pty_fd, F_SETFL, fcntl(_pty_fd, F_GETFL) | O_NONBLOCK);

	_poller = new Poller(_pty_fd, pid, this);
	_poller->start();
}

int PtyProxyLinux::send_string(const char *data) {
	return _poller->write_cmd(Poller::CMD_SEND, (const unsigned char *)data, strlen(data));
}

int PtyProxyLinux::available_to_send() {
	if (_poller != nullptr) {
		return _poller->available_to_send();
	}
	return 0;
}

void PtyProxyLinux::resize_screen(int nrows, int ncols) {
	_num_rows = nrows;
	_num_cols = ncols;
	if (_poller != nullptr) {
		_poller->write_cmd(Poller::CMD_RESIZE, nullptr, 0);
	}
}
