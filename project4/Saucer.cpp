#include <stdlib.h>
#include "Saucer.h"
#include "Explosion.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "NetworkManager.h"
#include "EventView.h" 
#include "PointsHost.h" // Points class used to update score.
#include "PointsClient.h"
#include "Role.h"
#include <sstream>

Saucer::Saucer() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite* p_temp_sprite = resource_manager.getSprite("saucer");
	if (!p_temp_sprite) {
		log_manager.writeLog("Saucer::Saucer(): Warning! Sprite '%s' not found", "saucer");
	} 
	else {
		// This method is in Object class.
		setSprite(p_temp_sprite);
		setSpriteSlowdown(4);
		setType("Saucer");
		setXVelocity(-0.3);
		this->moveToStart();
	}
	registerInterest(NUKE_EVENT);
}

Saucer::~Saucer() {
	// Create explotion on saucer destructor.
	Explosion *p_explosion = new Explosion;
	p_explosion->setPosition(this->getPosition());
}

int Saucer::eventHandler(const df::Event *p_e) {
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
	if (p_e->getType() == NUKE_EVENT) {
		// Create explosion event, mark for deletion and spawn new saucer.
		Explosion *p_explosion = new Explosion;
		p_explosion->setPosition(this->getPosition());
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		// Send deletion of this saucer to client.
		sendObjDel(this->getId());
		world_manager.markForDelete(this);
		// Create a new saucer.
		new Saucer;
	}
	return 0;
}
// Handle hit.
void Saucer::hit(const df::EventCollision *p_collision_event) {
	if ((p_collision_event->getObject1()->getType() == "Saucer") &&
		(p_collision_event->getObject2()->getType() == "Saucer")) {
		return;
	}
	if ((p_collision_event->getObject1()->getType() == "Bullet-Host") ||
		(p_collision_event->getObject1()->getType() == "Bullet-Host")) {
		new Saucer;
		// Update host points if host bullet hits a saucer.
		df::EventView ev(HOST_POINTS_STRING, 10, true);
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		world_manager.onEvent(&ev);
	}
	if ((p_collision_event->getObject1()->getType() == "Bullet-Client") ||
		(p_collision_event->getObject1()->getType() == "Bullet-Client")) {
		new Saucer;
		// Update client points if client bullet hits a saucer.
		df::EventView ev(CLIENT_POINTS_STRING, 10, true);
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		world_manager.onEvent(&ev);
	}
	if (((p_collision_event->getObject1()->getType() == "Hero-Host")) ||
		((p_collision_event->getObject2()->getType() == "Hero-Host"))) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		// Send delete of host hero to client.
		sendObjDel(p_collision_event->getObject1()->getId());
		sendObjDel(p_collision_event->getObject2()->getId());
		world_manager.markForDelete(p_collision_event->getObject1());
		world_manager.markForDelete(p_collision_event->getObject2());
	}
	if (((p_collision_event->getObject1()->getType() == "Hero-Client")) ||
		((p_collision_event->getObject2()->getType() == "Hero-Client"))) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		// Send delete of client hero to client.
		sendObjDel(p_collision_event->getObject1()->getId());
		sendObjDel(p_collision_event->getObject2()->getId());
		world_manager.markForDelete(p_collision_event->getObject1());
		world_manager.markForDelete(p_collision_event->getObject2());
	}
}

void Saucer::out() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	Role &role = Role::getInstance();
	if (getPosition().getX() >= 0) {
		return;
	}
	else {
		// Send the new location to client if it is host.
		if (role.isHost()) {
			this->moveToStart();
			sendLocUpdate(this);
			new Saucer;
		}
		return;
	}
}

void Saucer::moveToStart() {
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Position temp_pos;

	int world_horiz = world_manager.getBoundary().getHorizontal();
	int world_vert = world_manager.getBoundary().getVertical();

	temp_pos.setX(world_horiz + rand() % world_horiz + 3);
	temp_pos.setY(rand() % (world_vert - 8) + 4);

	// Check for collision when spawning.
	df::ObjectList collision_list = world_manager.isCollision(this, temp_pos);
	while (!collision_list.isEmpty()) {
		temp_pos.setX(temp_pos.getX() + 1);
		collision_list = world_manager.isCollision(this, temp_pos);
	}
	world_manager.moveObject(this, temp_pos);
}
// Send location update when saucer is out.
int Saucer::sendLocUpdate(Saucer *p_s) {
	std::string ser_saucer = p_s->serialize();
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	int msg_size = 3 * sizeof(int) + strlen(ser_saucer.c_str());
	HostMessageType msg_type = UPDATE_OBJECT;
	int msg_id = p_s->getId();
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
		buffer[i] = ser_saucer.c_str()[i - 3 * sizeof(int)];
	}
	// Send msg through network manager.
	int rtn = network_manager.send((void*)buffer, msg_size);
	return rtn;
}
// Send delete object.
int Saucer::sendObjDel(int obj_id) {
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


