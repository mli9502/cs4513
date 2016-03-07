#include "Sentry.h"
#include "EventNetwork.h"
#include "NetworkManager.h"
#include "LogManager.h"
#include <sstream>
#include <vector>
df::Sentry::Sentry() {
	registerInterest(df::STEP_EVENT);
	setSolidness(df::SPECTRAL);
	setVisible(false);
}

int df::Sentry::eventHandler(const Event *p_e) {
	if (p_e->getType() == df::STEP_EVENT) {
		this->doStep();
		return 1;
	}
	return 0;
}
// Check the data size in socket.
// Get all completed msgs and generate corresponding network events.
void df::Sentry::doStep() {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	int dataBytes = network_manager.isData();
	if (dataBytes < 0) {
		log_manager.writeLog("Sentry::doStep(): NetworkManager::isData() error.");
		return;
	}
	// Generate a network event.
	if (dataBytes > sizeof(int)) {
		// Vector of events. 
		// Keep track of all the events and generate the events later.
		std::vector<df::EventNetwork> events;
		int tmpBytes = dataBytes;
		int startingPt = 0;
		char buffer[8192 * 100];
		while (tmpBytes > sizeof(int)) {
			// Peek for int size of data.
			int rRtn = network_manager.recevie((void*)buffer, sizeof(int) + startingPt, true);
			if (rRtn != sizeof(int) + startingPt) {
				log_manager.writeLog("df::Sentry::doStep(): peek for int data failed.");
				return;
			}
			int msgSize;
			// Convert from byte array to int.
			std::memcpy(&msgSize, buffer + startingPt, sizeof(int));
			// If desired number of bytes has arrived, record network event with msg size.
			if (msgSize <= dataBytes - startingPt) {
				df::EventNetwork en(msgSize);
				events.push_back(en);
			}
			tmpBytes -= msgSize;
			startingPt += msgSize;
		}
		// Construct all the network events.
		for (int i = 0; i < events.size(); i++) {
			network_manager.onEvent(&events.at(i));
		}
		return;
	}
}
