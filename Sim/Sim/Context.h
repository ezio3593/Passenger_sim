#pragma once

#include <queue>
#include <map>
#include <vector>
#include "Objects.h"
#include "DrawingContext.h"

class Timer
{
	LARGE_INTEGER frequency;
	bool isSupported;
	LONG64 startTime;
public:
	Timer();
	bool isSupportPerf() { return isSupported; }
	void start() { startTime = getSysUptime(); }
	LONG64 getSysUptime();
	LONG64 getTimeFromStart() { return getSysUptime() - startTime; }
	~Timer() {}
};

class ContextException: public std::exception
{
	public:
		ContextException() : exception() {}
		ContextException(const char* const& msg) : exception(msg) {}
};

class Context: public ContextInterface
{
	typedef std::priority_queue<Event, std::vector<Event>, EventComparator> EventPool;
	typedef std::map<id, KR_Object*> ObjectPool;

	EventPool eventPool;
	ObjectPool objPool;
	Timer timer;

	CriticalSection cs;
	CriticalSection objPoolCs;
	CriticalSection objCs;

	bool isStart;

	HANDLE threadHandle;
	
	int addEvent(Event& e);
	void processEvents();

	static unsigned int __stdcall threadProcess(void *p);

public:
	Context();
	int addObject(KR_Object* obj);
	int deleteObjectById(id objId);
	void addEventFromObj(int label, id source, id dest, LONG64 timeDiff);
	CriticalSection& getObjCriticalSection() { return cs; }
	bool isStarted() { return isStart; }
	void start();
	void stop();

	void clearObjPool();

	~Context();
};