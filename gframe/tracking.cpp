#include "config.h"
#include "tracking.h"
#include "game.h"

extern ygo::Game* mainGame;

namespace ygo
{
	
#define TRACKER_REQUEST 0xdead
#define TRACKER_START 0xf00d
#define TRACKER_CANCEL 0xc001
#define TRACKER_PUSH 0x1eaf
#define TRACKER_SYNC 0xbeef

Tracking tracking;

unsigned int ResolveAddress( const char * domain )
{
	struct hostent * remoteHost;
	remoteHost = gethostbyname(domain);
	if(remoteHost == NULL)
		return (unsigned int)ntohl(inet_addr(domain));
	return (unsigned int)ntohl(*(unsigned int *)remoteHost->h_addr_list[0]);
}

Tracking::Tracking(): nextSync(0)
{
}

void Tracking::init()
{
	unsigned int trackerIP = ResolveAddress("wgdev.net");
	unsigned short trackerPort = 8914;
	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = htonl(trackerIP);
	toAddr.sin_port = htons(trackerPort);

	udpSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(udpSock == (int)INVALID_SOCKET)
		return;
	int opt = TRUE;
	setsockopt(udpSock, SOL_SOCKET, SO_BROADCAST, (const char*)&opt, sizeof(int));
	unsigned short lport = 7914;
	struct sockaddr_in local;
	while(lport < 32768)
	{
		local.sin_addr.s_addr = htonl(INADDR_ANY);
		local.sin_family = AF_INET;
		local.sin_port = htons(lport);
		if(::bind(udpSock, (sockaddr*)&local, sizeof(sockaddr)) != SOCKET_ERROR)
			break;
		++ lport;
	}
}

void Tracking::cancelGame()
{
	if(udpSock < 0)
		return;
	unsigned short op = TRACKER_CANCEL;
	send((const char *)&op, 2);
}

void Tracking::createGame(HostInfo * hi)
{
	if(udpSock < 0)
		return;
	char buf[sizeof(HostInfo)];
	memcpy(buf, hi, sizeof(HostInfo));
	*(unsigned short *)buf = TRACKER_PUSH;
	send(buf, sizeof(HostInfo));
	nextSync = time(NULL) + 30;
}

void Tracking::loopCheck()
{
	if(udpSock < 0)
		return;
	
	time_t now = time(NULL);
	if(nextSync > 0 && nextSync <= now)
	{
		unsigned short op = TRACKER_SYNC;
		send((const char *)&op, 2);
		nextSync += 30;
	}

	fd_set fds;
	struct timeval tv = {0, 0};
	FD_ZERO(&fds);
	FD_SET(udpSock, &fds);
	int result = select(udpSock + 1, &fds, 0, 0, &tv);
	while(result > 0)
	{
		struct sockaddr_in from = {0};
		socklen_t len = sizeof(struct sockaddr_in);
		char buf[4096];
		int l = recvfrom(udpSock, buf, 4096, 0, (struct sockaddr *)&from, &len);
		if(l <= 0)
			return;
		if(l == sizeof(HostInfo))
		{
			HostInfo * hi = (HostInfo *)&buf[0];
			if(hi->identifier == TRACKER_PUSH)
			{
				bool found = false;
				for(auto it = hosts.begin(); it != hosts.end(); ++ it)
				{
					if(it->second.address == hi->address && it->second.port == hi->port)
					{
						found = true; break;
					}
				}
				if(!found)
				{
					const wchar_t * mode;
					if(!hi->no_check_deck && !hi->no_shuffle_deck && !hi->no_shuffle_deck && !hi->attack_ft && !hi->no_chain_hint
							&& hi->start_lp == 8000 && hi->start_hand == 5 && hi->draw_count == 1)
						mode = L"标准设定";
					else mode = L"自定义设定";
					wchar_t tbuf[128];
					myswprintf(tbuf, L"[T] [%ls] [%ls] %ls", mode, hi->lflist, hi->name);
					mainGame->gMutex.Lock();
					unsigned int n = mainGame->lstServerList->addItem(tbuf);
					hosts[n] = *hi;
					mainGame->gMutex.Unlock();
				}
			}
		}
		result = select(udpSock + 1, &fds, 0, 0, &tv);
	}
}

void Tracking::queryList()
{
	if(udpSock < 0)
		return;
	unsigned short op = TRACKER_REQUEST;
	send((const char *)&op, 2);
}

void Tracking::startGame()
{
	if(udpSock < 0)
		return;
	unsigned short op = TRACKER_START;
	send((const char *)&op, 2);
}

void Tracking::send(const void* buf, size_t len)
{
	sendto(udpSock, (const char *)buf, len, 0, (const struct sockaddr *)&toAddr, sizeof(struct sockaddr_in));
}
	
}
