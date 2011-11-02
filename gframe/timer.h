#ifndef TIMER_H
#define TIMER_H

#ifdef _WIN32

#include <windows.h>

class Timer {
public:
	Timer() {
		hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	}
	~Timer() {
		CloseHandle(hTimer);
	}
	void Reset() {
		QueryPerformanceCounter(&startTime);
	}
	float GetElapsedTime() {
		LARGE_INTEGER endTime;
		LARGE_INTEGER timerFreq;
		QueryPerformanceFrequency(&timerFreq);
		QueryPerformanceCounter(&endTime);
		return (endTime.QuadPart - startTime.QuadPart) * 1000000.0f / timerFreq.QuadPart;
	}
	void Wait(long long us) {
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = us * -10LL;
		SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
		WaitForSingleObject(hTimer, INFINITE);
	}
private:
	LARGE_INTEGER startTime;
	HANDLE hTimer;
};

#else // _WIN32

#include <sys/time.h>
#include <signal.h>

class Timer {
public:
	Timer() {
	}
	~Timer() {
	}
	void Reset() {
		gettimeofday(&startTime, 0);
	}
	float GetElapsedTime() {
		timeval endTime;
		gettimeofday(&endTime, 0);
		return (endTime.tv_sec - startTime.tv_sec) * 1000000.0f + endTime.tv_usec - startTime.tv_usec;
	}
	void Wait(long long us) {
		sigset_t sig;
		itimerval itimer;
		sigemptyset(&sig, SIGALRM);
		sigprocmask(SIG_BLOCK, &sig, 0);
		itimer.it_interval.tv_sec = 0;
		itimer.it_interval.tv_usec = 0;
		itimer.it_value.tv_sec = us / 1000000;
		itimer.it_value.tv_usec = us % 1000000;
		setitimer(ITIMER_REAL, &itimer, 0);
		sigwaitinfo(&sig, 0);
	}
private:
	timeval startTime;
};

#endif // _WIN32

#endif // TIMER_H
