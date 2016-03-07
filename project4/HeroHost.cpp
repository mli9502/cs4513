#include "HeroHost.h"
#include "BulletHost.h"
#include "Explosion.h"
#include "GameOver.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "GameManager.h" // Used to exit game.
#include "EventView.h"
#include "Role.h"
#include "GraphicsManager.h"
#include "Config.h"
#include "PointsHost.h"
#include "NetworkManager.h"
#include "Host.h"

#undef MOUSE_EVENT

HeroHost::HeroHost() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite *p_temp_sprite;
	// Construct new reticle.
	this->p_reticle = new Reticle();
	p_reticle->draw();
	this->move_slowdown = 3;
	this->move_countdown = move_slowdown;
	this->fire_slowdown = 15;
	this->fire_countdown = fire_slowdown;
	this->nuke_slowdown = 15;
	this->nuke_countdown = nuke_slowdown;
	// Set nuke count.
	this->nuke_count = 10;
	p_temp_sprite = resource_manager.getSprite("hostship");
	if (!p_temp_sprite) {
		log_manager.writeLog("Hero::Hero(): Warning! Sprite '%s' not found", "host-ship");
	}
	else {
		setSprite(p_temp_sprite);
		setSpriteSlowdown(3);
		setTransparency();
	}
	// Register for keyboard event, step event and mouse event.
	registerInterest(df::KEYBOARD_EVENT);
	registerInterest(df::STEP_EVENT);
	registerInterest(df::MOUSE_EVENT);
	setType("Hero-Host");
	df::Position pos(7, world_manager.getBoundary().getVertical() / 3);
	setPosition(pos);
}
// End game when hero is destroied.
HeroHost::~HeroHost() {
	// Make big explosion.
	for (int i = -8; i <= 8; i += 5) {
		for (int j = -5; j <= 5; j += 3) {
			df::Position temp_pos = this->getPosition();
			temp_pos.setX(this->getPosition().getX() + i);
			temp_pos.setY(this->getPosition().getY() + j);
			Explosion *p_explosion = new Explosion;
			p_explosion->setPosition(temp_pos);
		}
	}
	GameOver *p_go = new GameOver;
	df::WorldManager::getInstance().markForDelete(this->p_reticle);
}

int HeroHost::eventHandler(const df::Event *p_e) {
	Role &role = Role::getInstance();
	df::LogManager &log_manager = df::LogManager::getInstance();
	// If in client, don't handle event.
	if (!role.isHost()) {
		return 0;
	}
	if (p_e->getType() == df::STEP_EVENT) {
		this->step();
		return 1;
	}
	// Check if mouse outside game window.
	sf::RenderWindow *p_win = df::GraphicsManager::getInstance().getWindow();
	sf::Vector2i lp = sf::Mouse::getPosition(*p_win);
	if (lp.x > df::Config::getInstance().getWindowHorizontalPixels() ||
		lp.x < 0 ||
		lp.y > df::Config::getInstance().getWindowVerticalPixels() ||
		lp.y < 0) {
		return 0;
	}
	else {
		if (p_e->getType() == df::KEYBOARD_EVENT) {
			const df::EventKeyboard *p_keyboard_event = dynamic_cast <const df::EventKeyboard *> (p_e);
			this->kbd(p_keyboard_event);
			return 1;
		}
		if (p_e->getType() == df::MOUSE_EVENT) {
			const df::EventMouse *p_mouse_event = dynamic_cast <const df::EventMouse *>(p_e);
			mouse(p_mouse_event);
			return 1;
		}
		return 0;
	}
	
}

void HeroHost::mouse(const df::EventMouse *p_mouse_event) {
	if ((p_mouse_event->getMouseAction() == df::CLICKED) &&
		(p_mouse_event->getMouseButton() == df::Mouse::LEFT)) {
		fire(p_mouse_event->getMousePosition());
	}
}

void HeroHost::kbd(const df::EventKeyboard *p_keyboard_event) {
	switch (p_keyboard_event->getKey()) {
	case df::Keyboard::Q:
		if (p_keyboard_event->getKeyboardAction() == df::KEY_PRESSED) {
			df::WorldManager &world_manager = df::WorldManager::getInstance();
			// Send delete msg to client.
			sendObjDel(this->getId());
			world_manager.markForDelete(this);
		}
		break;
	case df::Keyboard::W:
		if (p_keyboard_event->getKeyboardAction() == df::KEY_DOWN) {
			move(-1);
		}
		break;
	case df::Keyboard::S:
		if (p_keyboard_event->getKeyboardAction() == df::KEY_DOWN) {
			move(1);
		}
		break;
	case df::Keyboard::SPACE:
		if (p_keyboard_event->getKeyboardAction() == df::KEY_DOWN) {
			nuke();
		}
		break;
	}
}

void HeroHost::nuke() {
	if (this->nuke_count == 0) {
		return;
	}
	if (this->nuke_countdown > 0) {
		return;
	}
	nuke_countdown = nuke_slowdown;
	this->nuke_count --;
	// Create nuke event and send to interested Objects.
	// Update score.
	int totalPts = 0;
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::ObjectList object_list = world_manager.getAllObjects(true);
	df::ObjectListIterator i(&object_list);
	for (i.first(); !i.isDone(); i.next()) {
		df::Object *p_o = i.currentObject();
		if (strcmp((p_o->getType()).c_str(), "Saucer") == 0) {
			// Check if saucer is in screen.
			int world_horiz = world_manager.getBoundary().getHorizontal();
			if (p_o->getPosition().getX() < world_horiz) {
				totalPts += 10;
			}	
		}
	}
	df::EventView pv(HOST_POINTS_STRING, totalPts, true);
	world_manager.onEvent(&pv);
	EventNuke nuke;
	world_manager.onEvent(&nuke);
	// Update nuke count.
	df::EventView ev("hostnuke", -1, true);
	world_manager.onEvent(&ev);
	Role &role = Role::getInstance();
	// Send nuke count to client.
	if (role.isHost()) {
		int obj_id = 0;
		df::ObjectList object_list = world_manager.getAllObjects(true);
		df::ObjectListIterator i(&object_list);
		for (i.first(); !i.isDone(); i.next()) {
			df::Object *p_o = i.currentObject();
			if (strcmp((p_o->getType()).c_str(), "Nuke-Host") == 0) {
				obj_id = p_o->getId();
				break;
			}
		}
		df::Object *p_o = world_manager.objectWithId(obj_id);
		df::ViewObject *p_vo = dynamic_cast<df::ViewObject *>(p_o);
		sendUpdateNuke(obj_id, p_vo->getValue());
	}
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("nuke");
	p_sound->play();
}

void HeroHost::move(int dy) {
	if (move_countdown > 0) {
		return;
	}
	move_countdown = move_slowdown;
	// Construct the new position.
	df::Position new_pos(getPosition().getX(), getPosition().getY() + dy);
	// Check whether it stays on window.
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	if ((new_pos.getY() > 3) &&
		(new_pos.getY() < world_manager.getBoundary().getVertical() - 4)) {
		world_manager.moveObject(this, new_pos);
	}
}

void HeroHost::step() {
	// Move countdown.
	move_countdown --;
	fire_countdown --;
	nuke_countdown --;
	if (move_countdown < 0) {
		move_countdown = 0;
	}
	if (fire_countdown < 0) {
		fire_countdown = 0;
	}
	if (nuke_countdown < 0) {
		nuke_countdown = 0;
	}
}

void HeroHost::fire(df::Position target) {
	if (fire_countdown > 0) {
		return;
	}
	fire_countdown = fire_slowdown;
	BulletHost *p = new BulletHost(this->getPosition());
	p->setYVelocity((float)(target.getY() - getPosition().getY()) /
		(float)(target.getX() - getPosition().getX()));
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("fire");
	if (p_sound != NULL) {
		p_sound->play();
	}
}
// Send updated nuke count for host nuke.
int HeroHost::sendUpdateNuke(int obj_id, int nukeCnt) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	int msg_size = 4 * sizeof(int);
	HostMessageType msg_type = UPDATE_NUKE;
	char buffer[4096];
	char sizeArr[sizeof(int)];
	char typeArr[sizeof(int)];
	char idArr[sizeof(int)];
	char ptsArr[sizeof(int)];
	memcpy(sizeArr, &msg_size, sizeof(int));
	memcpy(typeArr, &msg_type, sizeof(int));
	memcpy(idArr, &obj_id, sizeof(int));
	memcpy(ptsArr, &nukeCnt, sizeof(int));
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

int HeroHost::sendObjDel(int obj_id) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	int msg_size = 3 * sizeof(int);
	HostMessageType msg_type = DELETE_OBJECT;
	char buffer[4096];
	char sizeArr[sizeof(int)];
	char typeArr[sizeof(int)];
	char idArr[sizeof(int)];
	memcpy(sizeArr, &msg_size, sizeof(int));
	memcpy(typeArr, &msg_type, sizeof(int));
	memcpy(idArr, &obj_id, sizeof(int));
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
	int rtn = network_manager.send((void*)buffer, msg_size);
	return rtn;
}

