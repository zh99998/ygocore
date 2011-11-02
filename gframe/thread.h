#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32

#include <windows.h>

class Thread {
public:
	static void NewThread(int (*thread_func)(void*), void* param) {
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_func, param, 0, 0);
	}
};

#else // _WIN32

#endif // _WIN32

#endif // THREAD_H
