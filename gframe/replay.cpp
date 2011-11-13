#include "game.h"
#include "replay.h"
#include "../ocgcore/ocgapi.h"
#include "../ocgcore/card.h"
#include <sys/time.h>
#include <algorithm>

extern ygo::Game* mainGame;

namespace ygo {

void Replay::Open() {
	time_t now;
	tm* ptime;
	time(&now);
	ptime = localtime(&now);
	char fname[32];
	sprintf(fname, "./replay/%s.yrp", asctime(ptime));
	fp = fopen("fname", "wb+");
	if(!fp)
		return;
	is_opened = true;
}
void Replay::WriteData(char* data, unsigned int length) {
	if(!is_opened)
		return;
	fwrite(data, length, 1, fp);
	fflush(fp);
}
void Replay::Close() {
	if(!is_opened)
		return;
	fclose(fp);
}
void Replay::ReplayThread(void* pd) {

}

}
