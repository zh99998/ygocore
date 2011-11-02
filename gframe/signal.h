#ifndef SIGNAL_H
#define SIGNAL_H

#ifdef _WIN32

#include <windows.h>

class Signal {
public:
	Signal() {
		_event = CreateEvent(0, FALSE, FALSE, 0);
	}
	~Signal() {
		CloseHandle(_event);
	}
	void Set() {
		SetEvent(_event);
	}
	void Reset() {
		ResetEvent(_event);
	}
	void Wait() {
		WaitForSingleObject(_event, INFINITE);
	}
private:
	HANDLE _event;
};

#else // _WIN32

#endif // _WIN32

#endif // SIGNAL_H
