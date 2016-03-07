#include "EventNetwork.h"

df::EventNetwork::EventNetwork() {
	setType(df::NETWORK_EVENT);
}
// Constructor with initial bytes.
df::EventNetwork::EventNetwork(int initial_bytes) {
	setType(df::NETWORK_EVENT);
	this->bytes = initial_bytes;
}
// Set number of bytes avaliable.
void df::EventNetwork::setBytes(int new_bytes) {
	this->bytes = new_bytes;
	return;
}
// Get number of bytes avaliable.
int df::EventNetwork::getBytes() const {
	return this->bytes;
}

