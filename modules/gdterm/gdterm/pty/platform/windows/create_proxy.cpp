#include "./pty_proxy_win.h"

PtyProxy *
create_proxy(TermRenderer *renderer) {
	PtyProxy *proxy = new PtyProxyWin();
	proxy->set_renderer(renderer);
	return proxy;
}
