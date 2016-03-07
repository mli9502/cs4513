/*
	Author: Mengwen Li (mli2)
	Reference: Dragonfly tutorial: http://dragonfly.wpi.edu/tutorial/
				Dragonflg Networking: http://web.cs.wpi.edu/~cs4513/c16/projects/dragonfly-networking/
*/
#include "HeroHost.h"
#include "Saucer.h"
#include "PointsHost.h"
#include "PointsClient.h"
#include "Star.h"
#include "GameStart.h"
#include "GameManager.h"
#include "Pause.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "NetworkManager.h"
#include "Role.h"
#include "iostream"
#include "string.h"
#include "Sentry.h"
#include "Host.h"
#include "Client.h"
#include <sstream>

void loadResources(void);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		exit(-1);
	}
	df::LogManager &log_manager = df::LogManager::getInstance();
	df::GameManager &game_manager = df::GameManager::getInstance();
	Role &role = Role::getInstance();
	if (game_manager.startUp()) {
		log_manager.writeLog("Error starting game manager!");
		game_manager.shutDown();
		return 0;
	}
	log_manager.setFlush(true);
	df::splash();
	df::NetworkManager &network_manager = df::NetworkManager::getInstance();
	network_manager.startUp();
	new df::Sentry;
	loadResources();
	if (strcmp(argv[1], "h") == 0) {
		role.setHost(true);
		new Host(argv[2]);
		game_manager.run();
	}
	if (strcmp(argv[1], "c") == 0) {
		role.setHost(false);
		new Client(argv[3], argv[2]);
		game_manager.run();
	}
	return 0;
}
// Used to load resources into resource manager in order to be used by objects.
void loadResources(void) {
	df::ResourceManager &resource_manager = df::ResourceManager::getInstance();
	resource_manager.loadSprite("sprites/saucer-spr.txt", "saucer");
	resource_manager.loadSprite("sprites/host-ship-spr.txt", "hostship");
	resource_manager.loadSprite("sprites/client-ship-spr.txt", "clientship");
	resource_manager.loadSprite("sprites/host-bullet-spr.txt", "hostbullet");
	resource_manager.loadSprite("sprites/client-bullet-spr.txt", "clientbullet");
	resource_manager.loadSprite("sprites/explosion-spr.txt", "explosion");
	resource_manager.loadSprite("sprites/gameover-spr.txt", "gameover");
	resource_manager.loadSprite("sprites/gamestart-spr.txt", "gamestart");
	resource_manager.loadSound("sounds/fire.wav", "fire");
	resource_manager.loadSound("sounds/explode.wav", "explode");
	resource_manager.loadSound("sounds/nuke.wav", "nuke");
	resource_manager.loadSound("sounds/game-over.wav", "game over");
	resource_manager.loadMusic("sounds/start-music.wav", "start music");
}
