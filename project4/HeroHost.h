#pragma once
#include "Object.h"
#include "EventKeyboard.h"
#include "EventMouse.h"
#include "EventStep.h" // Decrease the countdown variable every game step.
#include "EventNuke.h"
#include "Reticle.h"
/*
Class for host hero.
*/
class HeroHost : public df::Object {
private:
	void kbd(const df::EventKeyboard *p_keyboard_event);
	void mouse(const df::EventMouse *p_mouse_event);
	void move(int dy);
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
	HeroHost();
	~HeroHost();
	int eventHandler(const df::Event *p_e);
	void step();
	void fire(df::Position target);
	void nuke();
	int sendUpdateNuke(int obj_id, int nukeCnt);
	int sendObjDel(int obj_id);
};

