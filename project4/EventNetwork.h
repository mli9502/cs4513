#pragma once

#include "Event.h"
/*
Network event class.
*/
namespace df {

	const std::string NETWORK_EVENT = "df::network";

	class EventNetwork : public df::Event
	{
	private:
		// Number of bytes avaliable.
		int bytes;
	public:
		EventNetwork();
		// Constructor with initial bytes.
		EventNetwork(int initial_bytes);
		// Set number of bytes avaliable.
		void setBytes(int new_bytes);
		// Get number of bytes avaliable.
		int getBytes() const;
	};
}




