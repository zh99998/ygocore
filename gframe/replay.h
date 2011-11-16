#ifndef REPLAY_H
#define REPLAY_H

#include "config.h"
#include <sys/time.h>

namespace ygo {

struct ReplayHeader {
	unsigned int id;
	unsigned int version;
	wchar_t player1[20];
	wchar_t player2[20];
	int flag;
	int seed;
	int startlp;
	int starthand;
	int drawcount;
	int option;
	int datasize;
	int rawdatasize;
	int hash;
	int reserved;
};

class Replay {
public:
	Replay(): is_recording(false), is_replaying(false) {}
	void BeginRecord(time_t seed);
	void WriteData(const void* data, unsigned int length, bool flush = true);
	void WriteInt32(int data, bool flush = true);
	void WriteInt16(short data, bool flush = true);
	void WriteInt8(char data, bool flush = true);
	void Flush();
	void EndRecord();
	bool BeginReplay(const wchar_t* name);
	bool ReadNextResponse();
	void ReadData(void* data, unsigned int length);
	int ReadInt32();
	short ReadInt16();
	char ReadInt8();
	void EndReplay();
protected:
	FILE* fp;
	bool is_recording;
	bool is_replaying;
};

}

#endif
