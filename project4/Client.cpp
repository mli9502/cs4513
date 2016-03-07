#include "Object.h"
#include "Saucer.h"
#include "HeroClient.h"
#include "HeroHost.h"
#include "Client.h"
#include "Host.h"
#include "Star.h"
#include "NetworkManager.h"
#include "WorldManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "BulletHost.h"
#include "BulletClient.h"
#include "GraphicsManager.h"
#include "Config.h"
#include "PointsHost.h"
#include "PointsClient.h"
#include <sstream>
#include <climits>

#undef MOUSE_EVENT
// Constructor for Client. Client takes in host name and port number
// from command line argument.
Client::Client(std::string host_name, std::string port)
{
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	setSolidness(df::SPECTRAL);
	setVisible(false);
	setType("client");
	network_manager.registerInterest(this, df::NETWORK_EVENT);
	registerInterest(df::KEYBOARD_EVENT);
	registerInterest(df::MOUSE_EVENT);
	network_manager.startUp();
	// Connect to server.
	int cRtn = network_manager.connect(host_name, port);
	if (cRtn < 0) {
		log_manager.writeLog("Client::Client(): connect error.");
	}
}
// Handles mouse, keyboard and network event.
int Client::eventHandler(const df::Event* p_e) {
	df::LogManager &log_manager = df::LogManager::getInstance();
	// Handle network event.
	if (p_e->getType() == df::NETWORK_EVENT) {
		const df::EventNetwork *p_network_event = dynamic_cast<const df::EventNetwork *>(p_e);
		handleNetwork(p_network_event);
		return 1;
	}
	// Ignor keyboard and mouse event if mouse is not inside the window.
	sf::RenderWindow *p_win = df::GraphicsManager::getInstance().getWindow();
	sf::Vector2i lp = sf::Mouse::getPosition(*p_win);
	if (lp.x > df::Config::getInstance().getWindowHorizontalPixels() ||
		lp.x < 0 ||
		lp.y > df::Config::getInstance().getWindowVerticalPixels() ||
		lp.y < 0) {
		return 0;
	}
	else {
		if (p_e->getType() == df::MOUSE_EVENT) {
			const df::EventMouse *p_mouse_event = dynamic_cast<const df::EventMouse *>(p_e);
			handleMouse(p_mouse_event);
			return 1;
		}
		if (p_e->getType() == df::KEYBOARD_EVENT) {
			SYSTEMTIME st;
			GetSystemTime(&st);
			char ct[84] = "";
			sprintf(ct, "%d/%d/%d  %d:%d:%d %d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			log_manager.writeLog(ct);
			log_manager.writeLog("Client key stroke detected.");
			const df::EventKeyboard *p_keyboard_event = dynamic_cast<const df::EventKeyboard *>(p_e);
			handleKeyboard(p_keyboard_event);
			return 1;
		}
	}
	return 0;
}
// Handles network event.
void Client::handleNetwork(const df::EventNetwork *p_network_event) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	// Get the message size. 
	int msgSize = p_network_event->getBytes();
	// Pull out the message from socket.
	char buffer[4096];
	network_manager.recevie((void*)buffer, msgSize, false);
	buffer[msgSize] = '\0';
	// Check from message type.
	int msg_type;
	std::memcpy(&msg_type, buffer + sizeof(int), sizeof(int));
	// Get object id.
	int obj_id;
	std::memcpy(&obj_id, buffer + 2 * sizeof(int), sizeof(int));
	// If add object.
	if (msg_type == ADD_OBJECT) {
		// Convert char buffer to string.
		std::string msg_str = std::string(buffer + 3 * sizeof(int));
		// Get object type.
		std::string obj_type = df::match(msg_str, "type");
		// Construct accoring objects.
		if (strcmp(obj_type.c_str(), "Star") == 0) {
			Star *st = new Star;
			st->deserialize(msg_str.c_str());
		}
		if (strcmp(obj_type.c_str(), "Saucer") == 0) {
			Saucer *s = new Saucer;
			s->deserialize(msg_str.c_str());
		}
		if (strcmp(obj_type.c_str(), "Hero-Client") == 0) {
			HeroClient *hc = new HeroClient;
			hc->deserialize(msg_str.c_str());
		}
		if (strcmp(obj_type.c_str(), "Hero-Host") == 0) {
			HeroHost *hh = new HeroHost;
			hh->deserialize(msg_str.c_str());
		}
		if (strcmp(obj_type.c_str(), "Bullet-Client") == 0) {
			BulletClient *bc = new BulletClient;
			bc->deserialize(msg_str.c_str());
			// Play fire sound when receiving a bullet.
			df::Sound *p_sound = df::ResourceManager::getInstance().getSound("fire");
			if (p_sound != NULL) {
				p_sound->play();
			}
		}
		if (strcmp(obj_type.c_str(), "Bullet-Host") == 0) {
			BulletHost *bh = new BulletHost;
			bh->deserialize(msg_str.c_str());
			// Play fire sound when receiving a bullet.
			df::Sound *p_sound = df::ResourceManager::getInstance().getSound("fire");
			if (p_sound != NULL) {
				p_sound->play();
			}
		}
		// Host point ViewObject.
		if (strcmp(obj_type.c_str(), "Points-Host") == 0) {
			PointsHost *ph = new PointsHost;
			ph->deserialize(msg_str.c_str());
		}
		// Client point ViewObject.
		if (strcmp(obj_type.c_str(), "Points-Client") == 0) {
			PointsClient *pc = new PointsClient;
			pc->deserialize(msg_str.c_str());
		}
		// Host nuke count ViewObject.
		if (strcmp(obj_type.c_str(), "Nuke-Host") == 0) {
			df::ViewObject *p_voh = new df::ViewObject;
			p_voh->deserialize(msg_str.c_str());
		}
		// Client nuke count ViewObject.
		if (strcmp(obj_type.c_str(), "Nuke-Client") == 0) {
			df::ViewObject *p_voc = new df::ViewObject;
			p_voc->deserialize(msg_str.c_str());
		}
	}
	// If message is update object.
	if (msg_type == UPDATE_OBJECT) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		df::LogManager &log_manager = df::LogManager::getInstance();
		// Get the id of the object.
		std::string msg_str = std::string(buffer + 3 * sizeof(int));
		int id;
		memcpy(&id, buffer + 2 * sizeof(int), sizeof(int));
		// Get the object with id.
		Object *p_obj = world_manager.objectWithId(id);
		if (p_obj == NULL) {
			log_manager.writeLog("No such object found.");
		}
		else {
			SYSTEMTIME st;
			GetSystemTime(&st);
			char ct[84] = "";
			sprintf(ct, "%d/%d/%d  %d:%d:%d %d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			log_manager.writeLog(ct);
			log_manager.writeLog("Client Object update received");
			// Update the object.
			p_obj->deserialize(msg_str);
		}
	}
	// If message is delete object.
	if (msg_type == DELETE_OBJECT) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		int id;
		memcpy(&id, buffer + 2 * sizeof(int), sizeof(int));
		Object *p_obj = world_manager.objectWithId(id);
		if (p_obj == NULL) {
			log_manager.writeLog("No such object found for delete.");
		}
		else {
			world_manager.markForDelete(p_obj);
		}
	}
	// If message is update points, get the corresponding ViewObject and update value.
	if (msg_type == UPDATE_POINTS) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		// Get object id and new points.
		int id;
		int pts;
		memcpy(&id, buffer + 2 * sizeof(int), sizeof(int));
		memcpy(&pts, buffer + 3 * sizeof(int), sizeof(int));
		Object *p_obj = world_manager.objectWithId(id);
		if (p_obj == NULL) {
			log_manager.writeLog("No such object found for delete.");
		}
		else {
			// Update points.
			dynamic_cast<df::ViewObject *>(p_obj)->setValue(pts);
		}
	}
	// If message is update nuke count, get the corresponding ViewObject and update value.
	if (msg_type == UPDATE_NUKE) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		int id;
		int nukeCnt;
		memcpy(&id, buffer + 2 * sizeof(int), sizeof(int));
		memcpy(&nukeCnt, buffer + 3 * sizeof(int), sizeof(int));
		Object *p_obj = world_manager.objectWithId(id);
		if (p_obj == NULL) {
			log_manager.writeLog("No such object found for delete.");
		}
		else {
			dynamic_cast<df::ViewObject *>(p_obj)->setValue(nukeCnt);
		}
		// Play nuke sound when nuke count is updated.
		df::Sound *p_sound = df::ResourceManager::getInstance().getSound("nuke");
		if (p_sound != NULL) {
			p_sound->play();
		}
	}
}
// Handle keyboard event.
void Client::handleKeyboard(const df::EventKeyboard *p_keyboard_event) {
	ClientMessageType msg_type = DETECT_KEY;
	if (p_keyboard_event->getKey() == df::Keyboard::Q ||
		p_keyboard_event->getKey() == df::Keyboard::W ||
		p_keyboard_event->getKey() == df::Keyboard::S ||
		p_keyboard_event->getKey() == df::Keyboard::SPACE) {
		// Send the key to server.
		std::string msg;
		msg.append(std::to_string(p_keyboard_event->getKey()));
		int sRtn = sendEvent(msg_type, msg);
	}
}
// Handle mouse event.
void Client::handleMouse(const df::EventMouse *p_mouse_event) {
	ClientMessageType msg_type = DETECT_MOUSE;
	// If it is a left click, send event.
	if ((p_mouse_event->getMouseAction() == df::CLICKED) &&
		(p_mouse_event->getMouseButton() == df::Mouse::LEFT)) {
		// Convert the position to string.
		std::string msg;
		msg.append("xpos:");
		msg.append(std::to_string(p_mouse_event->getMousePosition().getX()));
		msg.append(",ypos:");
		msg.append(std::to_string(p_mouse_event->getMousePosition().getY()));
		int sRtn = sendEvent(msg_type, msg);
	}
}
// Method used to send mouse and keyboard to server.
int Client::sendEvent(ClientMessageType msg_type, std::string msg) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	// msg size, type and actual message as a string.
	int msg_size = 2 * sizeof(int) + strlen(msg.c_str());
	char buffer[4096];
	char sizeArr[sizeof(int)];
	char typeArr[sizeof(int)];
	memcpy(sizeArr, &msg_size, sizeof(int));
	memcpy(typeArr, &msg_type, sizeof(int));
	// Assign size to buffer.
	for (int i = 0; i < sizeof(int); i++) {
		buffer[i] = sizeArr[i];
	}
	// Assign type to buffer.
	for (int i = sizeof(int); i < 2 * sizeof(int); i++) {
		buffer[i] = typeArr[i - sizeof(int)];
	}
	// Assign msg to buffer.
	for (int i = 2 * sizeof(int); i < msg_size; i++) {
		buffer[i] = msg.c_str()[i - 2 * sizeof(int)];
	}
	// Send message.
	int rtn = network_manager.send((void*)buffer, msg_size);
	return rtn;
}