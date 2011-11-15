#include "game.h"
#include "replay.h"
#include "../ocgcore/ocgapi.h"
#include "../ocgcore/card.h"
#include <algorithm>

extern ygo::Game* mainGame;

namespace ygo {

void Replay::BeginRecord(time_t seed) {
	tm* ptime = localtime(&seed);
	char fname[32];
	sprintf(fname, "./replay/%d-%02d-%02d %d.yrp", ptime->tm_year + 1900, ptime->tm_mon, ptime->tm_mday, seed);
	fp = fopen(fname, "wb+");
	if(!fp)
		return;
	is_recording = true;
}
void Replay::WriteData(const void* data, unsigned int length, bool flush) {
	if(!is_recording)
		return;
	fwrite(data, length, 1, fp);
	if(flush)
		fflush(fp);
}
void Replay::WriteInt32(int data, bool flush) {
	if(!is_recording)
		return;
	fwrite(&data, sizeof(int), 1, fp);
	if(flush)
		fflush(fp);
}
void Replay::WriteInt16(short data, bool flush) {
	if(!is_recording)
		return;
	fwrite(&data, sizeof(short), 1, fp);
	if(flush)
		fflush(fp);
}
void Replay::WriteInt8(char data, bool flush) {
	if(!is_recording)
		return;
	fwrite(&data, sizeof(char), 1, fp);
	if(flush)
		fflush(fp);
}
void Replay::Flush() {
	if(!is_recording)
		return;
	fflush(fp);
}
void Replay::EndRecord() {
	if(!is_recording)
		return;
	fclose(fp);
	is_recording = false;
}
bool Replay::BeginReplay(const wchar_t* name) {
	wchar_t fname[64];
	myswprintf(fname, L"./replay/%ls", name);
	char fname2[64];
	DataManager::EncodeUTF8(fname, fname2);
	fp = fopen(fname2, "r");
	if(!fp)
		return false;
	is_replaying = true;
	return true;
}
bool Replay::ReadNextResponse() {
	char resType = ReadInt8();
	if(resType == 1) {
		fread(&mainGame->dInfo.responseI, 4, 1, fp);
		set_responsei(mainGame->dInfo.pDuel, mainGame->dInfo.responseI);
	} else if(resType = 2) {
		int len = ReadInt8();
		fread(mainGame->dInfo.responseB, len, 1, fp);
		set_responseb(mainGame->dInfo.pDuel, (byte*)mainGame->dInfo.responseB);
	} else
		return false;
	if(feof(fp))
		return false;
	return true;
}
void Replay::ReadData(void* data, unsigned int length) {
	if(!is_replaying)
		return;
	fread(data, length, 1, fp);
}
int Replay::ReadInt32() {
	if(!is_replaying)
		return -1;
	int ret;
	fread(&ret, sizeof(int), 1, fp);
	return ret;
}
short Replay::ReadInt16() {
	if(!is_replaying)
		return -1;
	short ret;
	fread(&ret, sizeof(short), 1, fp);
	return ret;
}
char Replay::ReadInt8() {
	if(!is_replaying)
		return -1;
	char ret;
	fread(&ret, sizeof(char), 1, fp);
	return ret;
}
void Replay::EndReplay() {
	if(!is_replaying)
		return;
	fclose(fp);
	is_replaying = false;
}

}
