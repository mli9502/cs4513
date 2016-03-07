#pragma once

#include "ViewObject.h"
#include "Event.h"
#include "EventStep.h"

#define CLIENT_POINTS_STRING "clientpoints"

class PointsClient : public df::ViewObject
{
public:
	PointsClient();
	int eventHandler(const df::Event *p_e);
	// Send update points to client.
	int sendUpdatePoints(int pts);
};

