#ifndef TRACKING_H
#define TRACKING_H

#include "network.h"

#include <time.h>
#include <map>

namespace ygo
{

class Tracking
{
public:
	Tracking();
	void init();
	void createGame(HostInfo *);
	void startGame();
	void cancelGame();
	void queryList();
	void loopCheck();
	
	HostInfo * host(unsigned int idx)
	{
		auto it = hosts.find(idx);
		if(it == hosts.end()) return NULL;
		return &it->second;
	}
	void clearHost() { hosts.clear(); }
private:
	void send(const void *, size_t);
private:
	int udpSock;
	struct sockaddr_in toAddr;
	time_t nextSync;
	std::map<unsigned int, HostInfo> hosts;
};

extern Tracking tracking;

}

#endif
