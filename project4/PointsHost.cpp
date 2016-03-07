#include "PointsHost.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "Role.h"
#include "Host.h"

PointsHost::PointsHost() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	setLocation(df::TOP_LEFT);
	setViewString(HOST_POINTS_STRING);
	setColor(df::BLUE);
	setType("Points-Host");
	// Gain 1 point every second survived.
	registerInterest(df::STEP_EVENT);
}

int PointsHost::eventHandler(const df::Event* p_e) {
	Role &role = Role::getInstance();
	if (!role.isHost()) {
		return 0;
	}
	df::LogManager &log_manager = df::LogManager::getInstance();
	// Add call to parent to handle events parent handle.
	if (df::ViewObject::eventHandler(p_e)) {
		sendUpdatePoints(getValue());
		return 1;
	}
	if (p_e->getType() == df::STEP_EVENT) {
		if (dynamic_cast <const df::EventStep *>(p_e)->getStepCount() % 30 == 0) {
			setValue(getValue() + 1);
			sendUpdatePoints(getValue());
		}
		return 1;
	}
	return 0;
}

int PointsHost::sendUpdatePoints(int pts) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	int msg_size = 4 * sizeof(int);
	int obj_id = this->getId();
	HostMessageType msg_type = UPDATE_POINTS;

	char buffer[4096];
	char sizeArr[sizeof(int)];
	char typeArr[sizeof(int)];
	char idArr[sizeof(int)];
	char ptsArr[sizeof(int)];
	memcpy(sizeArr, &msg_size, sizeof(int));
	memcpy(typeArr, &msg_type, sizeof(int));
	memcpy(idArr, &obj_id, sizeof(int));
	memcpy(ptsArr, &pts, sizeof(int));
	// Assign size to buffer.
	for (int i = 0; i < sizeof(int); i++) {
		buffer[i] = sizeArr[i];
	}
	// Assign type to buffer.
	for (int i = sizeof(int); i < 2 * sizeof(int); i++) {
		buffer[i] = typeArr[i - sizeof(int)];
	}
	// Assign id to buffer.
	for (int i = 2 * sizeof(int); i < 3 * sizeof(int); i++) {
		buffer[i] = idArr[i - 2 * sizeof(int)];
	}
	for (int i = 3 * sizeof(int); i < 4 * sizeof(int); i++) {
		buffer[i] = ptsArr[i - 3 * sizeof(int)];
	}
	int rtn = network_manager.send((void*)buffer, msg_size);
	return rtn;
}