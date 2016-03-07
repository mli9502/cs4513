#include "Explosion.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"

Explosion::Explosion() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite* p_temp_sprite = resource_manager.getSprite("explosion");
	if (!p_temp_sprite) {
		log_manager.writeLog("Explosion::Explosion(): Warning! Sprite '%s' not found", "explosion");
	}
	else {
		setSprite(p_temp_sprite);
		this->time_to_live = getSprite()->getFrameCount();
	}
	setType("Explosion");
	setSolidness(df::SPECTRAL);
	registerInterest(df::STEP_EVENT);
	// Play explode sound when an explosion is constructed.
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("explode");
	p_sound->play();
}

void Explosion::step() {
	this->time_to_live --;
	if (this->time_to_live <= 0) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		world_manager.markForDelete(this);
	}
}

int Explosion::eventHandler(const df::Event *p_e) {
	if (p_e->getType() == df::STEP_EVENT) {
		this->step();
		return 1;
	}
	return 0;
}
