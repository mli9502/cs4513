#pragma once

#include "Object.h"
#include "EventStep.h"

namespace df {
	class Sentry : public df::Object
	{
	private:
		void doStep();
	public:
		Sentry();
		int eventHandler(const Event *p_e);
	};
}

