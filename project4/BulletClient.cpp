#include "BulletClient.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "NetworkManager.h"
#include "Role.h"
#include "Host.h"

BulletClient::BulletClient() {}
// Constructor with hero position.
BulletClient::BulletClient(df::Position hero_pos) {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite* p_temp_sprite = resource_manager.getSprite("clientbullet");
	if (!p_temp_sprite) {
		log_manager.writeLog("Bullet::Bullet(): Warning! Sprite '%s' not found", "bullet");
	}
	else {
		setSprite(p_temp_sprite);
		setSpriteSlowdown(5);
		setType("Bullet-Client");
		setXVelocity(1);
		df::Position pos(hero_pos.getX() + 3, hero_pos.getY());
		setPosition(pos);
	}
}
// Handles out event and collision event.
int BulletClient::eventHandler(const df::Event *p_e) {
	Role &role = Role::getInstance();
	if (!role.isHost()) {
		return 0;
	}
	if (p_e->getType() == df::OUT_EVENT) {
		out();
		return 1;
	}
	if (p_e->getType() == df::COLLISION_EVENT) {
		const df::EventCollision *p_collision_event = dynamic_cast <const df::EventCollision *>(p_e);
		hit(p_collision_event);
		return 1;
	}
	return 0;
}
// Remove the bullet when out.
// Send remove message to client if in host.
void BulletClient::out() {
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	Role &role = Role::getInstance();
	if (role.isHost()) {
		sendObjDel(this->getId());
		world_manager.markForDelete(this);
	}
}
// Remove both objects. 
// Send remove messages to client if in host.
void BulletClient::hit(const df::EventCollision *p_collision_event) {
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	Role &role = Role::getInstance();
	if (role.isHost()) {
		sendObjDel(p_collision_event->getObject1()->getId());
		sendObjDel(p_collision_event->getObject2()->getId());
		world_manager.markForDelete(p_collision_event->getObject1());
		world_manager.markForDelete(p_collision_event->getObject2());
	}
}
// Send a delete type message to client.
int BulletClient::sendObjDel(int obj_id) {
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	int msg_size = 3 * sizeof(int);
	HostMessageType msg_type = DELETE_OBJECT;
	char buffer[4096];
	// Set size, type and id as headers.
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