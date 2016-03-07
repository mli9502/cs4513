#pragma once

#include "Object.h"
#include "EventOut.h"
#include "EventCollision.h"
/*
Client bullet object.
*/
class BulletClient : public df::Object
{
private:
	void out();
	void hit(const df::EventCollision *p_collision_event);
public:
	BulletClient();
	BulletClient(df::Position hero_pos);
	int eventHandler(const df::Event *p_e);
	// Send a message to client to delete this bullet.
	int sendObjDel(int obj_id);
};