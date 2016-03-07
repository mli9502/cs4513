#pragma once

#include "Object.h"
#include "EventMouse.h"

#define RETICLE_CHAR '+'

class Reticle : public df::Object
{
public:
	Reticle();
	void draw(void);
	int eventHandler(const df::Event *p_e);
};

