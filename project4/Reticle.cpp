#include "Reticle.h"
#include "GraphicsManager.h" // Used to draw the reticle.
#include "LogManager.h"
#include "WorldManager.h"
#include "Reticle.h"

Reticle::Reticle() {
	df::LogManager &log_manager = df::LogManager::getInstance();
	setType("Reticle");
	// solidness is used for collision.
	// HARD: generate collision and impede movement.
	// SOFT: generate collision and not impede movement.
	// SPECTRAL: not collision and not impede movement.
	setSolidness(df::SPECTRAL);
	// Set layer of reticle.
	setAltitude(df::MAX_ALTITUDE);
	registerInterest(df::MOUSE_EVENT);
	df::WorldManager &world_manager = df::WorldManager::getInstance();
	df::Position pos(world_manager.getBoundary().getHorizontal() / 2, world_manager.getBoundary().getVertical() / 2);
	setPosition(pos);
}

int Reticle::eventHandler(const df::Event *p_e) {
	if (p_e->getType() == df::MOUSE_EVENT) {
		const df::EventMouse *p_mouse_event = dynamic_cast <const df::EventMouse *>(p_e);
		if (p_mouse_event->getMouseAction() == df::MOVED) {
			setPosition(p_mouse_event->getMousePosition());
		}
		return 1;
	}
	return 0;
}
// Reticle does not use sprite. It overrides draw method to draw it self.
void Reticle::draw() {
	df::GraphicsManager &graphics_manager = df::GraphicsManager::getInstance();
	graphics_manager.drawCh(getPosition(), RETICLE_CHAR, df::RED);
}