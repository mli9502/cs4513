#include "HeroClient.h"
#include "BulletClient.h"
#include "Explosion.h"
#include "GameOver.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "GameManager.h" // Used to exit game.
#include "EventView.h"
#include "Role.h"
#include "PointsClient.h"
#include "NetworkManager.h"
#include "Host.h"

HeroClient::HeroClient() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite *p_temp_sprite;
	// Construct new reticle.
	this->p_reticle = new Reticle();
	p_reticle->draw();
	this->move_slowdown = 5;
	this->move_countdown = move_slowdown;
	this->fire_slowdown = 15;
	this->fire_countdown = fire_slowdown;
	this->nuke_slowdown = 15;
	this->nuke_countdown = nuke_slowdown;
	// Set nuke count.
	this->nuke_count = 10;
	// Use client ship sprite.
	p_temp_sprite = resource_manager.getSprite("clientship");
	if (!p_temp_sprite) {
		log_manager.writeLog("Hero::Hero(): Warning! Sprite '%s' not found", "client-ship");
	}
	else {
		setSprite(p_temp_sprite);
		setSpriteSlowdown(3);
		setTransparency();
	}
	// Register for step event.
	registerInterest(df::STEP_EVENT);
	setType("Hero-Client");
	df::Position pos(7, world_manager.getBoundary().getVertical() * 2 / 3);
	setPosition(pos);
}
// End game when hero is destroied.
HeroClient::~HeroClient() {
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
// Handles step event.
int HeroClient::eventHandler(const df::Event *p_e) {
	if (p_e->getType() == df::STEP_EVENT) {
		this->step();
		return 1;
	}
	return 0;
}
// Method to act on the received key from host.
void HeroClient::kbd(int key) {
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	Role &role = Role::getInstance();
	switch (key) {
	// If Q is presseed, send client hero delete msg to client.
	case df::Keyboard::Q:
		if (role.isHost()) {
			sendObjDel(this->getId());
		}
		world_manager.markForDelete(this);
		break;
	case df::Keyboard::W:
		move(-1);
		break;
	case df::Keyboard::S:
		move(1);
		break;
	case df::Keyboard::SPACE:
		nuke();
		break;
	}
}

void HeroClient::nuke() {
	if (this->nuke_count == 0) {
		return;
	}
	if (this->nuke_countdown > 0) {
		return;
	}
	nuke_countdown = nuke_slowdown;
	this->nuke_count--;
	// Create nuke event and send to interested Objects.
	// Update score.
	int totalPts = 0;
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::ObjectList object_list = world_manager.getAllObjects(true);
	df::ObjectListIterator i(&object_list);
	// Get the total points to update by counting the number of Saucers in window.
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
	// Update the corresponding points with total points.
	df::EventView pv(CLIENT_POINTS_STRING, totalPts, true);
	world_manager.onEvent(&pv);
	// Create a nuke event and decrease nuke count.
	EventNuke nuke;
	world_manager.onEvent(&nuke);
	df::EventView ev("clientnuke", -1, true);
	world_manager.onEvent(&ev);
	// Send nuke count update to client.
	Role &role = Role::getInstance();
	if (role.isHost()) {
		int obj_id = 0;
		df::ObjectList object_list = world_manager.getAllObjects(true);
		df::ObjectListIterator i(&object_list);
		for (i.first(); !i.isDone(); i.next()) {
			df::Object *p_o = i.currentObject();
			if (strcmp((p_o->getType()).c_str(), "Nuke-Client") == 0) {
				obj_id = p_o->getId();
				break;
			}
		}
		df::Object *p_o = world_manager.objectWithId(obj_id);
		df::ViewObject *p_vo = dynamic_cast<df::ViewObject *>(p_o);
		sendUpdateNuke(obj_id, p_vo->getValue());
	}
	// Play nuke sound at server side.
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("nuke");
	if (p_sound != NULL) {
		p_sound->play();
	}
}

void HeroClient::move(int dy) {
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

void HeroClient::step() {
	// Step countdown.
	move_countdown--;
	fire_countdown--;
	nuke_countdown--;
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
// Method to act on the receive mouse from host.
void HeroClient::fire(df::Position target) {
	if (fire_countdown > 0) {
		return;
	}
	fire_countdown = fire_slowdown;
	BulletClient *p = new BulletClient(this->getPosition());
	p->setYVelocity((float)(target.getY() - getPosition().getY()) /
		(float)(target.getX() - getPosition().getX()));
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("fire");
	if (p_sound != NULL) {
		p_sound->play();
	}
}
// Send updated nuke count for client nuke.
int HeroClient::sendUpdateNuke(int obj_id, int nukeCnt) {
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
// Send delete object msg to client.
int HeroClient::sendObjDel(int obj_id) {
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

