#pragma once

#include "Object.h"
#include "EventStep.h"
#include "EventNetwork.h"

#define HEADER_SIZE 2 * sizeof(int)
/*
Class for Host.
*/
enum HostMessageType {
	ADD_OBJECT,
	UPDATE_OBJECT,
	DELETE_OBJECT,
	// Update points.
	UPDATE_POINTS,
	// Update nuke count.
	UPDATE_NUKE,
};

class Host : public df::Object
{
public:
	Host(std::string port);
	// Handles sending object when change.
	int eventHandler(const df::Event *p_e);
	// Check whether an object needs to be synched.
	bool needSynch(df::Object *p_o);
	// Send an object.
	int sendObject(df::Object *p_o, HostMessageType msg_type);
	// Append size, type and id then send through network manager.
	int appSizeTypeIdSend(int msg_id, HostMessageType msg_type, std::string ser_obj);
	// Handles network event.
	void handleNetwork(const df::EventNetwork *p_network_event);
};

