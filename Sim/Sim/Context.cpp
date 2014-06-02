#include "stdafx.h"
#include "Context.h"

Timer::Timer()
{
	isSupported = QueryPerformanceFrequency(&frequency);
	startTime = 0;
}

LONG64 Timer::getSysUptime()
{
	if (isSupported)
	{
		LARGE_INTEGER time;
        QueryPerformanceCounter(&time);
        return (1000LL * time.QuadPart) / frequency.QuadPart;
	} else return -1;
}

unsigned int Context::threadProcess(void *p)
{
	Context* c = reinterpret_cast<Context*>(p);
	
	c->processEvents();

	return 0;
}

Context::Context()
{
	isStart = false;
	threadHandle = NULL;
}

int Context::addObject(KR_Object* obj)
{
	Lock l(objPoolCs);

	if (obj && objPool.find(obj->getId()) == objPool.end())
	{
		objPool.insert(std::pair<id, KR_Object*>(obj->getId(), obj));
		if (!obj->isInitObj()) 
		{
			obj->setContext(this);
			try
			{
				obj->wakeup();
			} catch (const IncorrectDataException&)
			{
				return -1;
			}
		}
		return 0;
	} else return -1;
}

int Context::deleteObjectById(id objId)
{
	Lock l(objPoolCs);
	
	KR_Object* obj = objPool[objId];
	if (obj) 
	{
		objPool.erase(objId);
		return 0;
	} else return -1;
}

void Context::processEvents()
{
	bool flag;

	{
		Lock l(cs);

		flag = isStart;
	}


	while (flag)
	{

		{
			Lock l(cs);

			if (eventPool.size())
			{
				Event e = eventPool.top();

				if (timer.getTimeFromStart() >= e.getTimestamp())
				{
					id idNum = e.getDestId();
					
					KR_Object* obj = NULL;

					{
						Lock objPoolL(objPoolCs);
						obj = objPool[idNum];
					}
					
					{
						Lock objL(objCs);
						obj->recieveEvent(e);
					}

					eventPool.pop();

				}
			}
		}

		{
			Lock l(cs);
			flag = isStart;
		}
	}
}

int Context::addEvent(Event& e)
{
	eventPool.push(e);
	return 0;
}

void Context::addEventFromObj(int label, id source, id dest, LONG64 timeDiff)
{
	Lock l(cs);

	time_t t = timer.getTimeFromStart();
	addEvent(Event(label, source, dest, t, t + timeDiff));
}

void Context::start()
{

	if (!isStart)
	{
		isStart = true;

		if (!timer.isSupportPerf()) throw ContextException("Perfomance counter isn't supported");

		timer.start();
		
		threadHandle = (HANDLE)_beginthreadex(0, 0, threadProcess, reinterpret_cast<void*>(this), 0, NULL);

		if (!threadHandle) throw ContextException("Cannot create thread");
	}
}

void Context::stop()
{
	if (isStart)
	{
		{
			Lock l(cs);
			
			isStart = false;

			for (ObjectPool::iterator i = objPool.begin(); i != objPool.end(); ++i)
				i->second->stop();
		}

		if (threadHandle)
		{
			WaitForSingleObject(threadHandle, INFINITE);
			threadHandle = NULL;
		}

		while (eventPool.size())
		{
			eventPool.pop();
		}
	}
}

void Context::clearObjPool()
{
	objPool.erase(objPool.begin(), objPool.end());
}

Context::~Context()
{
	stop();
	
	if (threadHandle)
	{
		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
	}

	while (eventPool.size())
	{
		eventPool.pop();
	}
}