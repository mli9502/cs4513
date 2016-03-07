#include "GameOver.h"
#include "GameStart.h"
#include "ResourceManager.h"
#include "LogManager.h"
#include "WorldManager.h"
#include "GameManager.h"

GameOver::GameOver() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Sprite *p_temp_sprite;
	p_temp_sprite = resource_manager.getSprite("gameover");
	if (!p_temp_sprite) {
		log_manager.writeLog("GameOver::GameOver(): Warning! Sprite '%s' not found", "gameover");
	}
	else {
		setSprite(p_temp_sprite);
		setSpriteSlowdown(15);
		setTransparency('#');
		this->time_to_live = getSprite()->getFrameCount() * 15;
	}
	setLocation(df::CENTER_CENTER);
	registerInterest(df::STEP_EVENT);
	// Play the game over sound.
	df::Sound *p_sound = df::ResourceManager::getInstance().getSound("game over");
	p_sound->play();
}

GameOver::~GameOver() {
	df::GameManager &game_manager = df::GameManager::getInstance();
	game_manager.setGameOver();
}

void GameOver::draw() {
	df::Object::draw();
}

int GameOver::eventHandler(const df::Event *p_e) {
	
	if (p_e->getType() == df::STEP_EVENT) {
		this->step();
		return 1;
	}
	return 0;
}

void GameOver::step() {
	time_to_live--;
	if (time_to_live <= 0) {
		df::WorldManager &world_manager = df::WorldManager::getInstance();
		world_manager.markForDelete(this);
	}
}