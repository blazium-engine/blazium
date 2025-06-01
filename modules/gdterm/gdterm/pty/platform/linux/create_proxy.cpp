#include "pty_proxy_linux.h"

PtyProxy *
create_proxy(TermRenderer *renderer) {
	PtyProxy *proxy = new PtyProxyLinux();
	proxy->set_renderer(renderer);
	return proxy;
}
