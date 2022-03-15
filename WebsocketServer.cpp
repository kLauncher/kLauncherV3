#include "pch.h"
#include "WebsocketServer.h"




//The name of the special JSON field that holds the message type for messages
//#define MESSAGE_FIELD "__MESSAGE__"
#define MESSAGE_FIELD "__MESSAGE__"
json WebsocketServer::parseJson(const string& json) {
	return json::parse(json);
}

string WebsocketServer::stringifyJson(const json& val) {
	//When we transmit JSON data, we omit all whitespace
	/*Json::StreamWriterBuilder wbuilder;
	wbuilder["commentStyle"] = "None";
	wbuilder["indentation"] = "";*/

	return val.dump();
}

WebsocketServer::WebsocketServer() {
	//Wire up our event handlers
	this->endpoint.set_open_handler(std::bind(&WebsocketServer::onOpen, this, std::placeholders::_1));
	this->endpoint.set_close_handler(std::bind(&WebsocketServer::onClose, this, std::placeholders::_1));
	this->endpoint.set_message_handler(std::bind(&WebsocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
	//this->endpoint.set_error_channels(websocketpp::log::elevel::none);
	//this->endpoint.set_access_channels(websocketpp::log::elevel::none);
	this->endpoint.set_error_channels(websocketpp::log::elevel::none);
	this->endpoint.set_access_channels(websocketpp::log::elevel::none);

	//Initialise the Asio library, using our own event loop object
	this->endpoint.init_asio(&(this->eventLoop));
}

void WebsocketServer::run(int port, HANDLE hListenEvent) {
	try {
		//Listen on the specified port number and start accepting connections
		this->endpoint.listen("127.0.0.1", std::to_string(port));

		//this->endpoint.listen(port);
		this->endpoint.start_accept();

		if (hListenEvent)
			SetEvent(hListenEvent);

		//Start the Asio event loop
		this->endpoint.run();
	}
	catch (...) {
		throw;
	}
}

size_t WebsocketServer::numConnections() {
	//Prevent concurrent access to the list of open connections from multiple threads
	std::lock_guard<std::mutex> lock(this->connectionListMutex);

	return this->openConnections.size();
}

void WebsocketServer::sendMessage(ClientConnection conn, const string& messageType, const json& arguments) {
	//Copy the argument values, and bundle the message type into the object
	json messageData = arguments;
	messageData["code"] = messageType;

	//Send the JSON data to the client (will happen on the networking thread's event loop)
	this->endpoint.send(conn, WebsocketServer::stringifyJson(messageData), websocketpp::frame::opcode::text);
}

void WebsocketServer::broadcastMessage(const string& messageType, const json& arguments) {
	//Prevent concurrent access to the list of open connections from multiple threads
	std::lock_guard<std::mutex> lock(this->connectionListMutex);

	for (auto conn : this->openConnections) {
		this->sendMessage(conn, messageType, arguments);
	}
}

void WebsocketServer::onOpen(ClientConnection conn) {
	{
		//Prevent concurrent access to the list of open connections from multiple threads
		std::lock_guard<std::mutex> lock(this->connectionListMutex);

		//Add the connection handle to our list of open connections
		this->openConnections.push_back(conn);
	}

	//Invoke any registered handlers
	for (auto handler : this->connectHandlers) {
		handler(conn);
	}
}

void WebsocketServer::onClose(ClientConnection conn) {
	{
		//Prevent concurrent access to the list of open connections from multiple threads
		std::lock_guard<std::mutex> lock(this->connectionListMutex);

		//Remove the connection handle from our list of open connections
		auto connVal = conn.lock();
		auto newEnd = std::remove_if(this->openConnections.begin(), this->openConnections.end(), [&connVal](ClientConnection elem) {
			//If the pointer has expired, remove it from the vector
			if (elem.expired() == true) {
				return true;
			}

			//If the pointer is still valid, compare it to the handle for the closed connection
			auto elemVal = elem.lock();
			if (elemVal.get() == connVal.get()) {
				return true;
			}

			return false;
		});

		//Truncate the connections vector to erase the removed elements
		this->openConnections.resize(std::distance(openConnections.begin(), newEnd));
	}

	//Invoke any registered handlers
	for (auto handler : this->disconnectHandlers) {
		handler(conn);
	}
}

void WebsocketServer::onMessage(ClientConnection conn, WebsocketEndpoint::message_ptr msg) {
	//Validate that the incoming message contains valid JSON
	json messageObject = WebsocketServer::parseJson(msg->get_payload());
	if (!messageObject.is_null()) {
		//Validate that the JSON object contains the message type field
		if (messageObject.contains("command")) {
			//Extract the message type and remove it from the payload
			std::string messageType = messageObject["command"].get<std::string>();
			//messageObject.erase(MESSAGE_FIELD);
			//If any handlers are registered for the message type, invoke them
			auto& handlers = this->messageHandlers[messageType];
			for (auto handler : handlers) {
				handler(conn, messageObject);
			}
		}
	}
}
