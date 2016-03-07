#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include "Manager.h"
#include "GameManager.h"
#include "EventNetwork.h"
#include "Role.h"
// Change this latter by command line argument.
#define DRAGONFLY_PORT "9000"

#pragma comment (lib, "Ws2_32.lib")

namespace df {
	class NetworkManager : public df::Manager
	{
	private:
		NetworkManager();
		NetworkManager(NetworkManager const&);
		void operator=(NetworkManager const&);
		SOCKET sock;
	public:
		// Get the one and only instance of the NetworkManager.
		static NetworkManager &getInstance();
		// Start up NetworkManager.
		int startUp();
		// Shut down NetworkManager.
		void shutDown();
		// Accept only network events.
		// Returns false for other engin events.
		bool isValid(std::string event_type) const;
		// Block, waiting to accept network connection.
		// Pass in port number inputted from command line argument when call accept.
		int accept(std::string port = DRAGONFLY_PORT);
		// Make network connection.
		// Return 0 on success, -1 otherwise.
		// Invoked by client.
		int connect(std::string host, std::string port = DRAGONFLY_PORT);
		// Close network connection.
		// Return 0 on success, -1 otherwise.
		int close();
		// Return true if connected, false otherwise.
		bool isConnected() const;
		// Return socket.
		SOCKET getSocket() const;
		// Send buffer to connected network.
		// Return 0 on success, -1 otherwise.
		int send(void* buffer, int bytes);
		// Receive from connected network (no more than n bytes).
		// If peek is true, leave data in socket, else, remove.
		// Return number of bytes received, -1 otherwise.
		int recevie(void* buffer, int nbytes, bool peek = false);
		// Check if network data.
		// Return amount of data (0 if no data), -1 if not connected or error.
		int isData() const;
	};

}



