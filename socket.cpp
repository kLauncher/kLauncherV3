#include "pch.h"
#include "socket.h"
#include "common.h"
#include "monitor.h"
#include "config.h"

int current_port = 0;
HANDLE hListenEvent = nullptr;

bool bDefaultPort = false;
std::future<void> f;
WebsocketServer* m_server = nullptr;

json Socket::getStatusJson() {
	Monitor::lock();

	json j;
	j["status"] = Monitor::status;

	switch (Monitor::status) {

		case eGameStatus::RUN_STARCRAFT:
			j["is64bit"] = Monitor::sc_64bit;
			j["isElevation"] = Monitor::isElevation;
			break;

		case eGameStatus::RUN_APP:
			j["bnetModule"] = Monitor::bMutexBattleNet;
			break;

		case eGameStatus::NOTFOUND_DLL:
			j["message"] = Monitor::statusMessage;
			break;
	}

	Monitor::unlock();
	return j;
}

bool Socket::onStatusChanged() {
	if (!m_server)
		return false;

	json j = getStatusJson();
	m_server->broadcastMessage("client-status", j);
	return false;
}

void Socket::ThreadProc() {
	asio::io_service mainEventLoop;
	WebsocketServer server;
	m_server = &server;

	server.connect([&mainEventLoop, &server](ClientConnection conn) {
		mainEventLoop.post([conn, &server]() {
			json j = {
				{"data", VERSION}
			};

			// 버전 전송
			server.sendMessage(conn, "client-version", j);

			// 현재상태 전송
			j = getStatusJson();
			server.sendMessage(conn, "client-status", j);


		});
	});

	server.disconnect([&mainEventLoop, &server](ClientConnection conn) {
		mainEventLoop.post([conn, &server]() {
			//std::clog << "Connection closed." << std::endl;
			//std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
		});
	});


	server.message("runGame", [&mainEventLoop, &server](ClientConnection conn, const json& args) {
		mainEventLoop.post([conn, args, &server]() {
			ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);

			unsigned long long startTime = GetTickCount64();
			// 고치기
			// 시작으로부터 30초 전이면
			while (GetTickCount64() - startTime < 20000) {

				Monitor::lock();
				int app_pid = Monitor::sc_pid;
				bool is64bit = Monitor::sc_64bit;
				Monitor::unlock();

				if (app_pid) {
					// 스타가 실행된지 5초가 지났다면

					try {
						uint64_t running_time = GetProcessRunningTime(app_pid);
						if (running_time > 5000) {
							// 64비트가 아니면
							if (!is64bit) {
								try {
									bool result = InjectionDll(app_pid, Config::realCurrentDirectory + "\\Lib\\load.dll");
									if (!result) {
										throw std::exception();
									}
								}
								catch (...) {
									ShellExecute(nullptr, "runas", Config::exePath.c_str(), "-load", nullptr, SW_SHOW);
								}
							}
							break;
						}
					}
					catch (...) {
						ShellExecute(nullptr, "runas", Config::exePath.c_str(), "-load", nullptr, SW_SHOW);
						break;
					}



				}

				Sleep(500);
			}


		});
	});

	server.message("runGameApp", [&mainEventLoop, &server](ClientConnection conn, const json& args) {
		mainEventLoop.post([conn, args, &server]() {
			Monitor::lock();
			ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);
			Monitor::unlock();
		});
	});

	server.message("fixGameAppModule", [&mainEventLoop, &server](ClientConnection conn, const json& args) {
		mainEventLoop.post([conn, args, &server]() {
			ShellExecute(nullptr, "runas", Config::exePath.c_str(), "-fixGameAppModule", nullptr, SW_SHOW);
		});
	});
	server.message("runGame", [&mainEventLoop, &server](ClientConnection conn, const json& args) {
		mainEventLoop.post([conn, args, &server]() {

			//std::thread runGameThread([&server]() {

			Monitor::lock();
			eGameStatus status = Monitor::status;
			Monitor::unlock();

			switch (status) {
				case eGameStatus::NOTHING:
					ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);
					break;

				case eGameStatus::RUN_APP:
					//ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);
					break;

				case eGameStatus::RUN_STARCRAFT:
					//ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);
					break;
			}


			//if (!(Monitor::status == 4 || Monitor::status == 5)) {



				//server.broadcastMessage("userInput", payload);
				// ShellExecute(nullptr, "open", Monitor::gameAppLocation.c_str(), "--exec=\"launch S1\"", nullptr, SW_SHOW);
				// printf("%d %s\n", Monitor::status, Monitor::gameAppLocation.c_str());


			//}

		//});
		//runGameThread.join();


		//printf("[message] %s\n", args.dump().c_str());
		//printf("runGame\n");
		});
	});

	//Start the networking thread
	std::thread serverThread([&server]() {

		try {
			bDefaultPort = true;
			current_port = LOCAL_PORT;
			server.run(LOCAL_PORT, hListenEvent); // await 구동이되면 안빠져나옴
			return;
		}
		catch (...) {}

		// 10개정도 난수를 돌아서 루프를 돌음
		bDefaultPort = false;
		for (int i = 0; i < 10; i++) {
			try {
				int port = GetRandom(1, 65535);
				current_port = port;
				server.run(port, hListenEvent); // await 구동이되면 안빠져나옴
				return;
			}
			catch (...) {}
		}
	});


	//Start a keyboard input thread that reads from stdin
	//std::thread inputThread([&server, &mainEventLoop]()
	//	{
	//		string input;
	//		while (1) {
	//			//Read user input from stdin
	//			std::getline(std::cin, input);

	//			//Broadcast the input to all connected clients (is sent on the network thread)
	//			Json::Value payload;
	//			payload["input"] = input;
	//			server.broadcastMessage("userInput", payload);

	//			//Debug output on the main thread
	//			mainEventLoop.post([]() {
	//				std::clog << "User input debug output on the main thread" << std::endl;
	//				});
	//		}
	//	});

	//Start the event loop for the main thread
	asio::io_service::work work(mainEventLoop);
	mainEventLoop.run();
}

int Socket::GetPort() {
	return current_port;
}

bool Socket::isDefaultPort() {
	return bDefaultPort;
}

bool Socket::Start() {
	std::srand((unsigned int)time(NULL));

	hListenEvent = CreateEvent(nullptr, false, false, nullptr);
	if (hListenEvent) {
		f = std::async(ThreadProc);
		Sleep(1000);

		// 10초간 
		DWORD dwState = WaitForSingleObject(hListenEvent, 10000);
		printf("current_port: %d\n", current_port);
		if (dwState == WAIT_OBJECT_0)
			return true;

		if (dwState == WAIT_TIMEOUT)
			return false;

	}
	return false;
}