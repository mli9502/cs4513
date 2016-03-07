#include "Object.h"
#include "EventOut.h"
#include "EventCollision.h"
#include "EventNuke.h"
#include "Host.h"

class Saucer : public df::Object {

public:
	Saucer();
	~Saucer();
	int eventHandler(const df::Event *p_e);
	void out();
	void moveToStart();
	void hit(const df::EventCollision *p_collision_event);
	// Send location update to client. Called when Saucer is out.
	int sendLocUpdate(Saucer *p_s);
	// Send deleting an object.
	int sendObjDel(int obj_id);
};

