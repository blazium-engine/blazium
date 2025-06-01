#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef USE_PTY_UTIL_H
#include <util.h>
#else
#include <pty.h>
#endif
#include <poll.h>

#include "poller.h"

const unsigned char commands[] = {
	'X', // CMD_EXIT
	'S', // CMD_SEND
	'R', // CMD_RESIZE
};

Poller::Poller(int poll_fd, pid_t poll_pid, PollHandler *handler) {
	_poll_fd = poll_fd;
	_poll_pid = poll_pid;
	_handler = handler;

	// int rc = pipe2(_cmd, O_NONBLOCK);
	int rc = pipe(_cmd);
	if (rc < 0) {
		fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
	}

	rc = fcntl(_cmd[0], F_SETFL, O_NONBLOCK);
	if (rc < 0) {
		fprintf(stderr, "error setting pipe to non-blocking: %s\n", strerror(errno));
		_cmd[0] = -1;
	}
}

Poller::~Poller() {
	write_cmd(CMD_EXIT, nullptr, 0);
	if (_thread.joinable()) {
		_thread.join();
	}
}

void Poller::start() {
	_thread = std::thread(&Poller::run, this);
}

int Poller::write_cmd(int cmd, const unsigned char *data, int len) {
	int old_size = 0;
	if (data != nullptr) {
		old_size = _append_cmd_data(data, len);
		if (old_size < 0) {
			return -1;
		}
	}
	if ((data == nullptr) || (old_size == 0)) {
		return write(_cmd[1], &commands[cmd], 1);
	} else {
		return 0;
	}
}

void Poller::run() {
	bool quit = false;
	while (!quit) {
		struct pollfd p[2];
		p[0].fd = _poll_fd;
		p[0].events = POLLIN;
		if (_cmd_pos > 0) {
			p[0].events |= POLLOUT;
		}
		p[0].revents = 0;

		p[1].fd = _cmd[0];
		p[1].events = POLLIN;
		p[1].revents = 0;

		int rc = poll(p, 2, 5000);
		if (rc < 0) {
			fprintf(stderr, "rc = %d", rc);
			if (errno != EAGAIN) {
				quit = true;
			}
		}
		if (rc > 0) {
			if (p[0].revents & POLLIN) {
				unsigned char buffer[101];
				int amt = read(_poll_fd, buffer, 100);
				if (amt < 0) {
					fprintf(stderr, "error on read %s\n", strerror(errno));
					if (errno != EAGAIN) {
						quit = true;
					}
				} else {
					buffer[amt] = '\0';
					_handler->handle(buffer, amt);
				}
			}
			if (p[0].revents & POLLOUT) {
				int amt = write(_poll_fd, _cmd_data, _cmd_pos);
				if (amt < 0) {
					fprintf(stderr, "error on write %s\n", strerror(errno));
					if (errno != EAGAIN) {
						quit = true;
					}
				} else {
					_remove_cmd_data(amt);
				}
			}
			if (p[0].revents & POLLHUP) {
				quit = true;
			}
			if (p[1].revents & POLLIN) {
				unsigned char buffer[2];
				int amt = read(_cmd[0], buffer, 1);
				if (amt < 0) {
					fprintf(stderr, "error on read from pipe %s\n", strerror(errno));
					if (errno != EAGAIN) {
						quit = true;
					}
				} else {
					buffer[amt] = '\0';
					if (buffer[0] == commands[CMD_EXIT]) {
						kill(_poll_pid, SIGHUP);
						close(_poll_fd);
						usleep(500000);
						waitpid(_poll_pid, nullptr, WNOHANG);
						quit = true;
					} else if (buffer[0] == commands[CMD_RESIZE]) {
						_handler->apply_size();
					}
				}
			}
		}
	}
	_handler->exited();
}

int Poller::available_to_send() {
	const std::lock_guard<std::mutex> lock(_cmd_mutex);
	return CMD_DATA_MAX_SIZE - _cmd_pos;
}

int Poller::_append_cmd_data(const unsigned char *data, int data_len) {
	const std::lock_guard<std::mutex> lock(_cmd_mutex);
	int old_size = _cmd_pos;
	if (_cmd_pos + data_len > CMD_DATA_MAX_SIZE) {
		fprintf(stderr, "Too much data, not appending\n");
		return -1;
	}
	std::memcpy(&_cmd_data[_cmd_pos], data, data_len);
	_cmd_pos += data_len;
	return old_size;
}

bool Poller::_remove_cmd_data(int amount) {
	const std::lock_guard<std::mutex> lock(_cmd_mutex);
	if (amount >= _cmd_pos) {
		_cmd_pos = 0;
		return true;
	}
	memcpy(_cmd_data, _cmd_data + amount, _cmd_pos - amount);
	_cmd_pos -= amount;

	return _cmd_pos == 0;
}
