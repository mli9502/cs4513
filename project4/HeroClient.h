#pragma once
#include "Object.h"
#include "EventKeyboard.h"
#include "EventMouse.h"
#include "EventStep.h" // Decrease the countdown variable every game step.
#include "EventNuke.h"
#include "Reticle.h"
/*
Class for client hero.
*/
class HeroClient : public df::Object
{
private:
	// Used to slow down hero movement.
	int move_slowdown;
	int move_countdown;
	// Used to slow down fire rate.
	int fire_slowdown;
	int fire_countdown;
	// Used to slow down nuke.
	int nuke_slowdown;
	int nuke_countdown;
	int nuke_count;
	// Hero draws reticle when it spawns.
	Reticle *p_reticle;
public:
	HeroClient();
	~HeroClient();
	// Handles step event only.
	int eventHandler(const df::Event *p_e);
	// Method to act on the received key from host.
	void kbd(int key);
	// Method to act to nuke.
	void nuke();
	void move(int dy);
	void step();
	// Method to act on the receive mouse from host.
	void fire(df::Position target);
	// Method to send nuke count update to client.
	int sendUpdateNuke(int obj_id, int nukeCnt);
	// Method to send remove message to client.
	int sendObjDel(int obj_id);
};

