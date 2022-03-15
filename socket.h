#pragma once
#include <future>
#include "WebsocketServer.h"


namespace Socket {
	bool Start();
	void ThreadProc();
	int GetPort();
	bool isDefaultPort();

	bool onStatusChanged();
	json getStatusJson();
}