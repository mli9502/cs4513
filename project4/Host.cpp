#include "Host.h"
#include "Client.h"
#include "NetworkManager.h"
#include "WorldManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "Saucer.h"
#include "HeroHost.h"
#include "HeroClient.h"
#include "PointsHost.h"
#include "PointsClient.h"
#include "Star.h"
#include <sstream>
#include <climits>

Host::Host(std::string port) {
	setSolidness(df::SPECTRAL);
	setVisible(false);
	setType("host");
	registerInterest(df::STEP_EVENT);
	// Register for network event.
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	network_manager.registerInterest(this, df::NETWORK_EVENT);
	// Start up network manager.
	network_manager.startUp();
	// Call accept to wait for connection.
	network_manager.accept(port);
	// Create stars.
	for (int i = 0; i < 16; i++) {
		new Star;
	}
	// Create a saucer.
	for (int i = 0; i < 16; i++) {
		new Saucer;
	}
	// Create host hero.
	new HeroHost;
	// Create client hero.
	new HeroClient;
	// Create host points.
	PointsHost *ph = new PointsHost;
	PointsClient *pc = new PointsClient;
	// Create Nuke counter for host and client.
	df::ViewObject *p_voh = new df::ViewObject;
	p_voh->setLocation(df::BOTTOM_LEFT);
	p_voh->setViewString("hostnuke");
	p_voh->setType("Nuke-Host");
	p_voh->setValue(10);
	p_voh->setColor(df::BLUE);
	df::ViewObject *p_voc = new df::ViewObject;
	p_voc->setLocation(df::BOTTOM_RIGHT);
	p_voc->setViewString("clientnuke");
	p_voc->setType("Nuke-Client");
	p_voc->setValue(10);
	p_voc->setColor(df::YELLOW);
}

int Host::eventHandler(const df::Event *p_e) {
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	// Check for step event.
	if (p_e->getType() == df::STEP_EVENT) {
		// Iterate through all objects and find the objects need to sync.
		df::ObjectList object_list = world_manager.getAllObjects(true);
		df::ObjectListIterator i(&object_list);
		for (i.first(); !i.isDone(); i.next()) {
			df::Object *p_o = i.currentObject();
			if (needSynch(p_o)) {
				HostMessageType msg_type;
				// If id is modified, a new object is created.
				if (p_o->isModified(df::ID)) {
					msg_type = ADD_OBJECT;
				}
				else {
					msg_type = UPDATE_OBJECT;
				}
				sendObject(p_o, msg_type);
			}
		}
		return 1;
	}
	if (p_e->getType() == df::NETWORK_EVENT) {
		const df::EventNetwork *p_network_event = dynamic_cast<const df::EventNetwork *>(p_e);
		handleNetwork(p_network_event);
	}
	return 0;
}

void Host::handleNetwork(const df::EventNetwork *p_network_event) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();

	int msgSize = p_network_event->getBytes();
	// Pull out the message from network manager.
	char buffer[4096];
	network_manager.recevie((void*)buffer, msgSize, false);
	buffer[msgSize] = '\0';
	// Get msg type.
	int msg_type;
	std::memcpy(&msg_type, buffer + sizeof(int), sizeof(int));
	// If mouse action is detected, pull out the position and fire the client hero.
	if (msg_type == DETECT_MOUSE) {
		std::string msg = std::string(buffer + 2 * sizeof(int));
		std::string pos_x = df::match(msg, "xpos");
		std::string pos_y = df::match(msg, "ypos");
		df::Position pos(std::atoi(pos_x.c_str()), std::atoi(pos_y.c_str()));
		// Get client hero.
		df::ObjectList object_list = world_manager.getAllObjects(true);
		df::ObjectListIterator i(&object_list);
		for (i.first(); !i.isDone(); i.next()) {
			df::Object *p_o = i.currentObject();
			// If the object is client hero, call fire().
			if (strcmp((p_o->getType()).c_str(), "Hero-Client") == 0) {
				HeroClient *hc_p = dynamic_cast<HeroClient *>(p_o);
				hc_p->fire(pos);
			}
		}
	}
	// If key action stroke is detected, get the key and call kbd() for client hero.
	if (msg_type == DETECT_KEY) {
		std::string msg = std::string(buffer + 2 * sizeof(int));
		int key = std::atoi(msg.c_str());
		// Get client hero.
		df::ObjectList object_list = world_manager.getAllObjects(true);
		df::ObjectListIterator i(&object_list);
		for (i.first(); !i.isDone(); i.next()) {
			df::Object *p_o = i.currentObject();
			// If the object is client hero.
			if (strcmp((p_o->getType()).c_str(), "Hero-Client") == 0) {
				HeroClient *hc_p = dynamic_cast<HeroClient *>(p_o);
				hc_p->kbd(key);
			}
		}
	}
}
// Send object to client.
int Host::sendObject(df::Object *p_o, HostMessageType msg_type) {
	std::string tmp = p_o->serialize();
	return this->appSizeTypeIdSend(p_o->getId(), msg_type, tmp);
}
// Check whether the object need sync.
bool Host::needSynch(df::Object *p_o) {
	// Sync bullet when it is created.
	if (p_o->getType() == "Bullet-Host" || p_o->getType() == "Bullet-Client") {
		if (p_o->isModified(df::ID)) {
			return true;
		}
		return false;
	}
	// Sync hero when move or created.
	if (p_o->getType() == "Hero-Host" || p_o->getType() == "Hero-Client") {
		if (p_o->isModified(df::ID) || p_o->isModified(df::POS)) {
			return true;
		}
		return false;
	}
	// Sync saucer when created.
	if (p_o->getType() == "Saucer") {
		if (p_o->isModified(df::ID)) {
			return true;
		}
		return false;
	}
	// Sync points ViewObject when created.
	if (p_o->getType() == "Points-Host" || p_o->getType() == "Points-Client") {
		if (p_o->isModified(df::ID)) {
			return true;
		}
		return false;
	}
	// Sync nuke count ViewObject when created.
	if (p_o->getType() == "Nuke-Client" || p_o->getType() == "Nuke-Host") {
		if (p_o->isModified(df::ID)) {
			return true;
		}
		return false;
	}
	// Sync star when created.
	if (p_o->getType() == "Star") {
		if (p_o->isModified(df::ID)) {
			return true;
		}
		return false;
	}
	return false;
}
// Append size, type and id to the actual msg and send.
int Host::appSizeTypeIdSend(int msg_id, HostMessageType msg_type, std::string ser_obj) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	int msg_size = 3 * sizeof(int) + strlen(ser_obj.c_str());
	char buffer[4096];
	char sizeArr[sizeof(int)];
	char typeArr[sizeof(int)];
	char idArr[sizeof(int)];
	memcpy(sizeArr, &msg_size, sizeof(int));
	memcpy(typeArr, &msg_type, sizeof(int));
	memcpy(idArr, &msg_id, sizeof(int));
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
	// Assign the result of msg to buffer.
	for (int i = 3 * sizeof(int); i < msg_size; i++) {
		buffer[i] = ser_obj.c_str()[i - 3 * sizeof(int)];
	}
	// Send msg through network manager.
	int rtn = network_manager.send((void*)buffer, msg_size);
	return rtn;
}
