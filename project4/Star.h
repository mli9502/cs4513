#pragma once

#include "Object.h"
#include "EventOut.h"

#define STAR_CHAR '.'

class Star : public df::Object
{
private:
	void out();
public:
	Star();
	void draw(void);
	int eventHandler(const df::Event *p_e);
};

