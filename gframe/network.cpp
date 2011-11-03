#include "network.h"
#include "game.h"

extern ygo::Game* mainGame;

namespace ygo {

bool NetManager::CreateHost() {
	wchar_t* pstr;
	int wp;
	hInfo.identifier = NETWORK_SERVER_ID;
	hInfo.address = mainGame->netManager.local_addr;
	for(wp = 0, pstr = (wchar_t*)mainGame->ebServerName->getText(); wp < 19 && pstr[wp]; ++wp)
		hInfo.name[wp] = pstr[wp];
	hInfo.name[wp] = 0;
	hInfo.no_check_deck = mainGame->chkNoCheckDeck->isChecked();
	hInfo.no_shuffle_deck = mainGame->chkNoShuffleDeck->isChecked();
	hInfo.no_shuffle_player = mainGame->chkNoShufflePlayer->isChecked();
	hInfo.attack_ft = mainGame->chkAttackFT->isChecked();
	hInfo.no_chain_hint = mainGame->chkNoChainHint->isChecked();
	hInfo.start_lp = _wtoi(mainGame->ebStartLP->getText());
	hInfo.start_hand = _wtoi(mainGame->ebStartHand->getText());
	hInfo.draw_count = _wtoi(mainGame->ebDrawCount->getText());
	hInfo.is_match = mainGame->cbMatchMode->getSelected() == 0 ? false : true;
	hInfo.lfindex = mainGame->cbLFlist->getSelected();
	hInfo.time_limit = mainGame->cbTurnTime->getSelected();
	for(wp = 0, pstr = (wchar_t*)mainGame->cbLFlist->getItem(mainGame->cbLFlist->getSelected()); wp < 19 && pstr[wp]; ++wp)
		hInfo.lflist[wp] = pstr[wp];
	hInfo.lflist[wp] = 0;
	sBHost = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sBHost == INVALID_SOCKET)
		return false;
	BOOL opt = TRUE;
	setsockopt(sBHost, SOL_SOCKET, SO_BROADCAST, (const char*)&opt, sizeof(BOOL));
	SOCKADDR_IN local;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(7913);
	if(bind(sBHost, (sockaddr*)&local, sizeof(sockaddr)) == SOCKET_ERROR) {
		closesocket(sBHost);
		return false;
	}
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sListen == INVALID_SOCKET) {
		closesocket(sBHost);
		return false;
	}
	local.sin_port = htons(7911);
	if(bind(sListen, (sockaddr*)&local, sizeof(sockaddr)) == SOCKET_ERROR) {
		closesocket(sBHost);
		closesocket(sListen);
		return false;
	}
	is_creating_host = true;
	Thread::NewThread(BroadcastServer, this);
	Thread::NewThread(ListenThread, this);
	return true;
}
bool NetManager::CancelHost() {
	if(!is_creating_host)
		return false;
	closesocket(sBHost);
	closesocket(sListen);
	is_creating_host = false;
}
bool NetManager::RefreshHost() {
	sBClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sBClient == INVALID_SOCKET)
		return false;
	BOOL opt = TRUE;
	setsockopt(sBClient, SOL_SOCKET, SO_BROADCAST, (const char*)&opt, sizeof(BOOL));
	SOCKADDR_IN local;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(7912);
	hReq.identifier = NETWORK_CLIENT_ID;
	if(bind(sBClient, (sockaddr*)&local, sizeof(sockaddr)) == SOCKET_ERROR) {
		closesocket(sBClient);
		return false;
	}
	Thread::NewThread(BroadcastClient, this);
	return true;
}
bool NetManager::JoinHost() {
	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sRemote == INVALID_SOCKET) {
		closesocket(sRemote);
		return false;
	}
	if(mainGame->lstServerList->getSelected() >= 0)
		Thread::NewThread(JoinThread, (void*)hosts[mainGame->lstServerList->getSelected()].address);
	else {
		char ip[20];
		int i = 0;
		wchar_t* pstr = (wchar_t *)mainGame->ebJionIP->getText();
		while(*pstr && i < 16) {
			ip[i++] = *pstr;
			*pstr++;
		}
		ip[i] = 0;
		int adr = inet_addr(ip);
		Thread::NewThread(JoinThread, (void*)adr);
	}
	return true;
}
bool NetManager::SendtoRemote(char* buf, int len) {
	int result = send(sRemote, buf, len, 0);
	if( result == SOCKET_ERROR)
		return false;
	return true;
}
bool NetManager::WaitClientResponse() {
	bool retry = false;
	do {
		int result = recv(sRemote, recv_buf, 2048, 0);
		if(result == 0) {
			if(!mainGame->is_closing) {
				mainGame->gMutex.Lock();
				mainGame->stACMessage->setText(L"与对方的连接中断");
				mainGame->dInfo.netError = true;
				mainGame->gMutex.Unlock();
			}
			return false;
		}
		if(result == SOCKET_ERROR) {
			if(!mainGame->is_closing) {
				mainGame->gMutex.Lock();
				mainGame->stACMessage->setText(L"网络连接发生错误");
				mainGame->dInfo.netError = true;
				mainGame->gMutex.Unlock();
				return false;
			}
		}
		char* pbuf = recv_buf;
		int type = NetManager::ReadInt8(pbuf);
		mainGame->dInfo.is_responsed = false;
		if(type == 1) {
			mainGame->dInfo.responseI = NetManager::ReadInt32(pbuf);
			mainGame->SetResponseI();
		} else if(type == 2) {
			int len = NetManager::ReadInt8(pbuf);
			for(int i = 0; i < len; ++i)
				mainGame->dInfo.responseB[i] = NetManager::ReadInt8(pbuf);
			mainGame->SetResponseB(len);
		} else {
			mainGame->SendByte(mainGame->dInfo.resPlayer, MSG_RETRY);
			retry = true;
		}
	} while(retry);
	return true;
}
int NetManager::GetLocalAddress() {
	hostent* host = gethostbyname(0);
	if(!host)
		return 0;
	return *(int*)host->h_addr_list[0];
}
int NetManager::BroadcastServer(void* np) {
	NetManager* net = (NetManager*)np;
	SOCKADDR_IN sockTo;
	sockTo.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	sockTo.sin_family = AF_INET;
	sockTo.sin_port = htons(7912);
	int recvLen = recvfrom(net->sBHost, (char*)&net->hReq, sizeof(HostRequest), 0, 0, 0);
	while(recvLen != 0 && recvLen != SOCKET_ERROR) {
		if(recvLen == sizeof(HostRequest) && net->hReq.identifier != NETWORK_CLIENT_ID)
			sendto(net->sBHost, (const char*)&net->hInfo, sizeof(HostInfo), 0, (sockaddr*)&sockTo, sizeof(sockaddr));
		recvLen = recvfrom(net->sBHost, (char*)&net->hReq, sizeof(HostRequest), 0, 0, 0);
	}
	net->is_creating_host = false;
	shutdown(net->sBHost, SD_BOTH);
	closesocket(net->sBHost);
	if(!mainGame->is_closing) {
		mainGame->gMutex.Lock();
		mainGame->btnLanStartServer->setEnabled(true);
		mainGame->btnLanCancelServer->setEnabled(false);
		mainGame->btnLanConnect->setEnabled(true);
		mainGame->btnRefreshList->setEnabled(true);
		mainGame->btnLoadReplay->setEnabled(true);
		mainGame->btnDeckEdit->setEnabled(true);
		mainGame->gMutex.Unlock();
	}
	return 0;
}
int NetManager::BroadcastClient(void* np) {
	NetManager* net = (NetManager*)np;
	SOCKADDR_IN sockTo;
	sockTo.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	sockTo.sin_family = AF_INET;
	sockTo.sin_port = htons(7913);
	fd_set fds;
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	FD_ZERO(&fds);
	FD_SET(net->sBClient, &fds);
	sendto(net->sBClient, (const char*)&net->hReq, sizeof(HostRequest), 0, (sockaddr*)&sockTo, sizeof(sockaddr));
	int result = select(0, &fds, 0, 0, &tv);
	std::set<int> addrset;
	net->hosts.clear();
	while(result != 0 && result != SOCKET_ERROR) {
		int recvLen = recvfrom(net->sBClient, (char*)&net->hInfo, sizeof(HostInfo), 0, 0, 0);
		if(recvLen == sizeof(HostInfo) && net->hInfo.identifier == NETWORK_SERVER_ID && addrset.find(net->hInfo.address) == addrset.end()) {
			net->hosts.push_back(net->hInfo);
		}
		result = select(0, &fds, 0, 0, &tv);
	}
	if(mainGame->is_closing)
		return 0;
	wchar_t tbuf[256];
	const wchar_t* mode;
	std::vector<HostInfo>::iterator it;
	mainGame->gMutex.Lock();
	mainGame->lstServerList->clear();
	for(it = net->hosts.begin(); it != net->hosts.end(); ++it) {
		if(!it->no_check_deck && !it->no_shuffle_deck && !it->no_shuffle_deck && !it->attack_ft && !it->no_chain_hint
		        && it->start_lp == 8000 && it->start_hand == 5 && it->draw_count == 1)
			mode = L"标准设定";
		else mode = L"自定义设定";
		swprintf(tbuf, L"[ % s][ % s] % s", mode, it->lflist, it->name);
		mainGame->lstServerList->addItem(tbuf);
	}
	mainGame->btnLanStartServer->setEnabled(true);
	mainGame->btnLanConnect->setEnabled(true);
	mainGame->btnRefreshList->setEnabled(true);
	mainGame->btnLoadReplay->setEnabled(true);
	mainGame->btnDeckEdit->setEnabled(true);
	mainGame->gMutex.Unlock();
	closesocket(net->sBClient);
	return 0;
}
int NetManager::ListenThread(void* np) {
	NetManager* net = (NetManager*)np;
	if(listen(net->sListen, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(net->sListen);
		return 0;
	}
	int recvlen, index;
	char* pbuf, *psbuf;
	net->sRemote = accept(net->sListen, 0, 0);
	while(net->sRemote != SOCKET_ERROR) {
		recvlen = recv(net->sRemote, net->recv_buf, 4096, 0);
		if(recvlen == SOCKET_ERROR) {
			closesocket(net->sRemote);
			net->sRemote = accept(net->sListen, 0, 0);
			continue;
		}
		//check deck
		pbuf = net->recv_buf;
		int maincount = ReadInt16(pbuf);
		int sidecount = ReadInt16(pbuf);
		mainGame->deckManager.LoadDeck(mainGame->deckManager.deckclient, (int*)pbuf, maincount, sidecount);
		if(!net->hInfo.no_check_deck && !mainGame->deckManager.CheckLFList(mainGame->deckManager.deckclient, net->hInfo.lfindex)) {
			psbuf = net->send_buf;
			WriteInt16(psbuf, 3);
			WriteInt8(psbuf, MSG_RETRY);
			send(net->sRemote, net->send_buf, 1, 0);
			closesocket(net->sRemote);
			net->sRemote = accept(net->sListen, 0, 0);
			continue;
		}
		psbuf = net->send_buf;
		WriteInt16(psbuf, 1);
		WriteInt8(psbuf, MSG_WAITING);
		send(net->sRemote, net->send_buf, 3, 0);
		mainGame->gMutex.Lock();
		mainGame->imgCard->setImage(mainGame->imageManager.tCover);
		mainGame->wCardImg->setVisible(true);
		mainGame->wInfos->setVisible(true);
		mainGame->stModeStatus->setText(L"");
		mainGame->dInfo.engLen = 0;
		mainGame->dInfo.msgLen = 0;
		mainGame->dField.Clear();
		mainGame->HideElement(mainGame->wModeSelection);
		mainGame->gMutex.Unlock();
		mainGame->WaitFrameSignal(10);
		closesocket(net->sBHost);
		net->is_creating_host = false;
		Thread::NewThread(Game::EngineThread, &mainGame->dInfo);
		Thread::NewThread(Game::GameThread, &mainGame->dInfo);
		break;
	}
	return 0;
}
int NetManager::JoinThread(void* adr) {
	SOCKADDR_IN server;
	server.sin_addr.s_addr = (long)adr;
	server.sin_family = AF_INET;
	server.sin_port = htons(7911);
	if(connect(mainGame->netManager.sRemote, (sockaddr*)&server, sizeof(sockaddr)) == SOCKET_ERROR) {
		closesocket(mainGame->netManager.sRemote);
		if(!mainGame->is_closing) {
			mainGame->btnLanStartServer->setEnabled(true);
			mainGame->btnLanConnect->setEnabled(true);
			mainGame->btnRefreshList->setEnabled(true);
			mainGame->btnLoadReplay->setEnabled(true);
			mainGame->btnDeckEdit->setEnabled(true);
			mainGame->stModeStatus->setText(L"无法连接至主机");
		}
		return 0;
	}
	char* pbuf = mainGame->netManager.send_buf;
	NetManager::WriteInt16(pbuf, mainGame->deckManager.deckhost.maincount + mainGame->deckManager.deckhost.extracount);
	NetManager::WriteInt16(pbuf, mainGame->deckManager.deckhost.sidecount);
	for(int i = 0; i < mainGame->deckManager.deckhost.maincount; ++i)
		NetManager::WriteInt32(pbuf, mainGame->deckManager.deckhost.main[i]->first);
	for(int i = 0; i < mainGame->deckManager.deckhost.extracount; ++i)
		NetManager::WriteInt32(pbuf, mainGame->deckManager.deckhost.extra[i]->first);
	for(int i = 0; i < mainGame->deckManager.deckhost.sidecount; ++i)
		NetManager::WriteInt32(pbuf, mainGame->deckManager.deckhost.side[i]->first);
	mainGame->netManager.SendtoRemote(mainGame->netManager.send_buf, pbuf - mainGame->netManager.send_buf);
	int result = recv(mainGame->netManager.sRemote, mainGame->netManager.recv_buf, 4096, 0);
	if(result == SOCKET_ERROR || mainGame->netManager.recv_buf[2] != MSG_WAITING) {
		closesocket(mainGame->netManager.sRemote);
		if(!mainGame->is_closing) {
			mainGame->btnLanStartServer->setEnabled(true);
			mainGame->btnLanConnect->setEnabled(true);
			mainGame->btnRefreshList->setEnabled(true);
			mainGame->btnLoadReplay->setEnabled(true);
			mainGame->btnDeckEdit->setEnabled(true);
			if(result == SOCKET_ERROR)
				mainGame->stModeStatus->setText(L"网络连接发生错误");
			else
				mainGame->stModeStatus->setText(L"无效卡组或者卡组不符合禁卡表规范");
		}
		return 0;
	}
	mainGame->gMutex.Lock();
	mainGame->imgCard->setImage(mainGame->imageManager.tCover);
	mainGame->wCardImg->setVisible(true);
	mainGame->wInfos->setVisible(true);
	mainGame->lstServerList->clear();
	mainGame->stModeStatus->setText(L"");
	mainGame->dInfo.engLen = 0;
	mainGame->dInfo.msgLen = 0;
	mainGame->dInfo.is_local_host = false;
	mainGame->dField.Clear();
	mainGame->btnLanStartServer->setEnabled(true);
	mainGame->btnLanConnect->setEnabled(true);
	mainGame->btnRefreshList->setEnabled(true);
	mainGame->btnLoadReplay->setEnabled(true);
	mainGame->btnDeckEdit->setEnabled(true);
	mainGame->HideElement(mainGame->wModeSelection);
	mainGame->gMutex.Unlock();
	mainGame->WaitFrameSignal(10);
	Thread::NewThread(Game::RecvThread, &mainGame->dInfo);
	Thread::NewThread(Game::GameThread, &mainGame->dInfo);
}
}
