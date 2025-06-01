#ifndef POLLER_H
#define POLLER_H

#include <fcntl.h>
#include <mutex>
#include <thread>

class PollHandler {
public:
	virtual void handle(unsigned char *data, int len) = 0;
	virtual void apply_size() = 0;
	virtual void exited() = 0;
	virtual ~PollHandler() {}
};

class Poller {
public:
	static const int CMD_EXIT = 0;
	static const int CMD_SEND = 1;
	static const int CMD_RESIZE = 2;

	Poller(int poll_fd, pid_t poll_pid, PollHandler *handler);

	virtual ~Poller();

	void start();

	int write_cmd(int cmd, const unsigned char *data, int len);
	int available_to_send();

	void run();

private:
	int _poll_fd;
	pid_t _poll_pid;
	PollHandler *_handler;

	std::thread _thread;

	int _cmd[2];

	static const int CMD_DATA_MAX_SIZE = 1024;

	std::mutex _cmd_mutex;
	int _cmd_pos = 0;
	unsigned char _cmd_data[CMD_DATA_MAX_SIZE];

	int _append_cmd_data(const unsigned char *data, int data_len);
	bool _remove_cmd_data(int amount);
};

#endif // POLLER_H
