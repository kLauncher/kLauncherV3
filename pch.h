#pragma once

/**********************************************************************/
// Configure
#define VERSION "v2.7.6"
#define CLASSNAME "kLauncherV3"

// Socket Configure
#define LOCAL_PORT 8011

// Compile Configure
#define CONSOLE
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
#define __DEBUG__
#define SERVER_ADDRESS "http://localhost"
#else
#define SERVER_ADDRESS "https://klauncher.kr"
#endif

#pragma region "import Socket-io"
// @reference https://github.com/socketio/socket.io-client-cpp
#include <sio/sio_client.h>
#ifdef __DEBUG__
#pragma comment(lib,"sio/lib/Release/sioclientMD.lib")
#else
#pragma comment(lib,"sio/lib/Release/sioclient_tlsMT.lib")
#endif

#pragma endregion

#pragma region "import OpenSSL"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#pragma comment(lib,"openssl/lib/libcrypto_static.lib")
#pragma comment(lib,"openssl/lib/libssl_static.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma endregion

#pragma region "import Websocketpp"
// @reference https://github.com/zaphoyd/websocketpp
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/roles/server_endpoint.hpp>
#include <asio/io_service.hpp>
#pragma endregion

#pragma region "import httplib"
// @reference https://github.com/yhirose/cpp-httplib
#ifndef __DEBUG__
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <cpp-httplib/httplib.h>
#pragma endregion


#ifdef CONSOLE // using Console
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#endif

// windows
#include <Windows.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <wintrust.h>
#pragma comment(lib, "Wintrust.lib")

#pragma comment(lib, "version.lib")
#pragma comment(lib, "crypt32.lib")


// std
#include <iostream>
#include <functional>
#include <exception>
#include <future>
#include <ctime>
#include <thread>
#include <algorithm>
#include <unordered_map>

#include <cstdlib>
#include <Softpub.h>

#include <random>
