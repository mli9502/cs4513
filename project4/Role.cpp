#include "Role.h"

Role::Role(){}

Role& Role::getInstance() {
	static Role role;
	return role;
}
// Set whether is host or not.
void Role::setHost(bool is_host) {
	this->is_host = is_host;
	return;
}
// Check whether is host or not.
bool Role::isHost() const {
	return this->is_host;
}
