#pragma once
/*
Role class for recording the role of host or client.
Singleton.
*/
class Role
{
private:
	Role();
	Role(Role const&);
	void operator=(Role const&);
	bool is_host;
public:
	// Get the one and only instance of the Role.
	static Role &getInstance();
	// Set host.
	void setHost(bool is_host = true);
	// Return true if host.
	bool isHost() const;
};

