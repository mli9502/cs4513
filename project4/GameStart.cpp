#include "Saucer.h"
#include "HeroHost.h"
#include "PointsHost.h"
#include "PointsClient.h"
#include "GameStart.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "ResourceManager.h"
#include "GameManager.h"

GameStart::GameStart() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite *p_temp_sprite;
	p_temp_sprite = resource_manager.getSprite("gamestart");
	if (!p_temp_sprite) {
		log_manager.writeLog("GameStart::GameStart(): Warning! Sprite '%s' not found", "gamestart");
	}
	else {
		setSprite(p_temp_sprite);
		setSpriteSlowdown(15);
	}
	setType("GameStart");
	setLocation(df::CENTER_CENTER);
	registerInterest(df::KEYBOARD_EVENT);
	// Get music from resource manager.
	p_music = df::ResourceManager::getInstance().getMusic("start music");
	playMusic();
}

int GameStart::eventHandler(const df::Event *p_e) {
	df::GameManager &game_manager = df::GameManager::getInstance();
	if (p_e->getType() == df::KEYBOARD_EVENT) {
		df::EventKeyboard *p_keyboard_event = (df::EventKeyboard *)p_e;
		switch (p_keyboard_event->getKey()) {
		case df::Keyboard::P:
			start();
			break;
		case df::Keyboard::Q:
			game_manager.setGameOver();
			break;
		default:
			break;
		}
		return 1;
	}
	return 0;
}

void GameStart::start() {
	for (int i = 0; i < 16; i++) {
		new Saucer;
	}
	// new Hero;
	// new Points;
	// Keep track of nukes left.
	df::ViewObject *p_vo = new df::ViewObject;
	p_vo->setLocation(df::TOP_LEFT);
	p_vo->setViewString("Nukes");
	p_vo->setValue(10);
	p_vo->setColor(df::YELLOW);
	// Set GameStart object inactive when game starts.
	setActive(false);
	p_music->pause();
}

void GameStart::draw() {
	df::Object::draw();
}

void GameStart::playMusic() {
	p_music->play();
}