#pragma once

#include "Event.h"
/*
Nuke event class.
*/
const std::string NUKE_EVENT = "nuke";

class EventNuke : public df::Event
{
public:
	EventNuke();
};

