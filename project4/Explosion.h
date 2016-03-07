#pragma once

#include "Object.h"
#include "EventStep.h"
/*
Explosion class.
*/
class Explosion : public df::Object
{
private:
	int time_to_live;
	void step();
public:
	Explosion();
	int eventHandler(const df::Event *p_e);
};

