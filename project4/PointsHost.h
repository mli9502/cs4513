#pragma once

#include "ViewObject.h"
#include "Event.h"
#include "EventStep.h"

#define HOST_POINTS_STRING "hostpoints"

class PointsHost : public df::ViewObject
{
public:
	PointsHost();
	int eventHandler(const df::Event *p_e);
	// Send points to client.
	int sendUpdatePoints(int pts);
};

