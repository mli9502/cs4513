#include "NetworkManager.h"
#include "LogManager.h"
#include "iostream"
#include <sstream>

df::NetworkManager::NetworkManager() {}

// Get the one and only instance of the NetworkManager.
df::NetworkManager& df::NetworkManager::getInstance() {
	static df::NetworkManager network_manager;
	return network_manager;
}
// Start up NetworkManager.
int df::NetworkManager::startUp() {
	this->sock = INVALID_SOCKET;
	return 0;
}
// Shut down NetworkManager.
void df::NetworkManager::shutDown() {
	if (this->sock != INVALID_SOCKET) {
		this->close();
	}
}
// Accept only network events.
// Returns false for other engin events.
bool df::NetworkManager::isValid(std::string event_type) const {
	if (strcmp(event_type.c_str(), df::NETWORK_EVENT.c_str()) == 0) {
		return true;
	}
	return false;
}
// Block, waiting to accept network connection.
// Pass in port number inputted from command line argument when call accept.
int df::NetworkManager::accept(std::string port) {
	df::GameManager &game_manager = df::GameManager::getInstance();
	// Get LogManager for recording errors.
	df::LogManager &log_manager = df::LogManager::getInstance();
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		log_manager.writeLog("df::NetworkManager::accept(%s): WSAStartup() return %d", port, iResult);
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	// Create SOCKET for listening for incoming connections.
	SOCKET listenSocket;
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		log_manager.writeLog("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	// Specifies the address family, IP address and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;
	// Bind to any avaliable address.
	service.sin_addr.s_addr = INADDR_ANY;
	// Convert port from string to unsigned short.
	service.sin_port = htons((unsigned short)strtoul(port.c_str(), NULL, 0));
	log_manager.writeLog("converted port is %u", service.sin_port);
	// bind.
	if (bind(listenSocket, (SOCKADDR *)&service, sizeof(service)) == SOCKET_ERROR) {
		log_manager.writeLog("bind() failed with error: %ld\n", WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	// Listen for incoming connections.
	if (listen(listenSocket, 1) == SOCKET_ERROR) {
		log_manager.writeLog("listen() failed with error: %ld\n", WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	this->sock = ::accept(listenSocket, NULL, NULL);
	if (this->sock == INVALID_SOCKET) {
		log_manager.writeLog("accept() failed with error: %ld\n", WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	// Set socket to unblocking after accept.
	u_long iMode = 1;
	iResult = ioctlsocket(this->sock, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		log_manager.writeLog("ioctlsocket failed with error: %ld\n", iResult);
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	log_manager.writeLog("Host successfully accepted connection.\n");
	// Close listen socket.
	closesocket(listenSocket);
	return 0;
}
// Make network connection.
// Return 0 on success, -1 otherwise.
int df::NetworkManager::connect(std::string host, std::string port) {
	df::GameManager &game_manager = df::GameManager::getInstance();
	// Get LogManager for recording errors.
	df::LogManager &log_manager = df::LogManager::getInstance();
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): WSAStartup() return %d", host.c_str(), port.c_str(), iResult);
		WSACleanup();
		game_manager.setGameOver();
	}
	struct in_addr addr;
	struct addrinfo hints;
	struct addrinfo* result;
	// Convert host name to ip address string.
	hostent *h = gethostbyname(host.c_str());
	if (h == NULL) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): gethostbyname() failed with %ld", host.c_str(), port.c_str(), WSAGetLastError());
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	addr.s_addr = *(u_long *)h->h_addr_list[0];
	std::string strAddr = inet_ntoa(addr);
	// Set hints.
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// getaddrinfo() call to set address info.
	int gRtn = getaddrinfo(strAddr.c_str(), port.c_str(), &hints, &result);
	if (gRtn != 0) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): getaddrinfo() failed with %ld", host.c_str(), port.c_str(), WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	// Create client socket.
	this->sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (this->sock == INVALID_SOCKET) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): socket() failed with %ld", host.c_str(), port.c_str(), WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	int cRtn = ::connect(this->sock, result->ai_addr, result->ai_addrlen);
	if (cRtn == SOCKET_ERROR) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): connect() failed with %ld", host.c_str(), port.c_str(), WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	log_manager.writeLog("df::NetworkManager::connect(%s, %s): Successfully connected!", host.c_str(), port.c_str());
	// Set socket to nonblocking.
	u_long iMode = 1;
	iResult = ioctlsocket(this->sock, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		log_manager.writeLog("df::NetworkManager::connect(%s, %s): ioctlsocket() failed with %ld", host.c_str(), port.c_str(), iResult);
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	freeaddrinfo(result);
	return 0;
}
// Close network connection.
// Return 0 on success, -1 otherwise.
int df::NetworkManager::close() {
	if (this->sock == INVALID_SOCKET) {
		return -1;
	}
	closesocket(this->sock);
	return 0;
}
// Return true if connected, false otherwise.
bool df::NetworkManager::isConnected() const {
	if (this->sock == INVALID_SOCKET) {
		return false;
	}
	return true;
}
// Return socket.
SOCKET df::NetworkManager::getSocket() const {
	return this->sock;
}
// Send buffer to connected network.
// Return 0 on success, -1 otherwise.
int df::NetworkManager::send(void* buffer, int bytes) {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::GameManager &game_manager = df::GameManager::getInstance();
	int sRtn = ::send(this->sock, (char*)buffer, bytes, 0);
	std::stringstream ss;
	if (sRtn == SOCKET_ERROR) {
		log_manager.writeLog("df::NetworkManager::send(): send() failed with error: %ld\n", WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	Role &role = Role::getInstance();
	if (role.isHost()) {
		log_manager.writeLog("df::NetworkManager::send(): Host sending %d bytes to client", bytes);
	}
	else {
		log_manager.writeLog("df::NetworkManager::send(): Client sending %d bytes to host", bytes);
	}
	return 0;
}
// Receive from connected network (no more than n bytes).
// If peek is true, leave data in socket, else, remove.
// Return number of bytes received, -1 otherwise.
int df::NetworkManager::recevie(void* buffer, int nbytes, bool peek) {
	// TODO: Modifiy this.
	// nbytes is the number of bytes want to read.
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::GameManager &game_manager = df::GameManager::getInstance();
	Role &role = Role::getInstance();
	int rRtn;
	if (peek) {
		rRtn = ::recv(this->sock, (char*)buffer, nbytes, MSG_PEEK);
	}
	else {
		rRtn = ::recv(this->sock, (char*)buffer, nbytes, 0);
	}
	if (rRtn == 0) {
		log_manager.writeLog("df::NetworkManager::receive(): Connection closed");
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	else if (rRtn < 0) {
		log_manager.writeLog("df::NetworkManager::receive(): recv() failed with error: %ld", WSAGetLastError());
		this->close();
		WSACleanup();
		game_manager.setGameOver();
		return -1;
	}
	else {
		if (!peek) {
			if (role.isHost()) {
				log_manager.writeLog("df::NetworkManager::receive(): Host pull %d bytes out from socket", rRtn);
			}
			else {
				log_manager.writeLog("df::NetworkManager::receive(): Client pull %d bytes out socket", rRtn);
			}
		}
		return rRtn;
	}
}
// Check if network data.
// Return amount of data (0 if no data), -1 if not connected or error.
int df::NetworkManager::isData() const {
	df::GameManager &game_manager = df::GameManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	Role &role = Role::getInstance();
	unsigned long n = 0;
	int iResult = ioctlsocket(this->sock, FIONREAD, &n);
	if (iResult < 0) {
		log_manager.writeLog("df::NetworkManager::isData(): ioctlsocket() failed with error: %ld\n", WSAGetLastError());
		game_manager.setGameOver();
		return -1;
	}
	if (role.isHost()) {
		log_manager.writeLog("df::NetworkManager::isData(): Host has %d bytes in socket", n);
	}
	else {
		log_manager.writeLog("df::NetworkManager::isData(): Client has %d bytes in socket", n);
	}
	
	return n;
}
