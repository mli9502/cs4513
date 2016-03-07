#pragma once

#include "Object.h"
#include "EventMouse.h"
#include "EventNetwork.h"
#include "EventKeyboard.h"
// Define message type.
enum ClientMessageType {
	DETECT_MOUSE,
	DETECT_KEY,
};
/*
Object constructed at client for client functionalities.
*/
class Client : public df::Object
{
public:
	// Need to pass in host name and port for client to connect.
	Client(std::string host_name, std::string port);
	// Handles mouse event, keyboard event, network event.
	int eventHandler(const df::Event *p_e);
	void handleMouse(const df::EventMouse *p_mouse_event);
	void handleKeyboard(const df::EventKeyboard *p_keyboard_event);
	void handleNetwork(const df::EventNetwork *p_network_event);
	// Send a mouse or keyboard event to host.
	int sendEvent(ClientMessageType msg_type, std::string msg);
};
