#pragma once

#include <exception>
#include <string>
#include <gl\GL.h>
#include "threads.h"

typedef void const* id;

class WagonEnviroment;

class GL_Color
{
	GLfloat colorR;
	GLfloat colorG;
	GLfloat colorB;
	GLfloat colorA;

public:
	static GL_Color white;
	static GL_Color black;

	static GL_Color red;
	static GL_Color green;
	static GL_Color blue;

	GL_Color(GLfloat _colorR = 0.0f, GLfloat _colorG = 0.0f, GLfloat _colorB = 0.0f, GLfloat _colorA = 0.0f):
		colorR(_colorR), colorG(_colorG), colorB(_colorB), colorA(_colorA) {}
	GLfloat getRed() const { return colorR; }
	GLfloat getGreen() const { return colorG; }
	GLfloat getBlue() const { return colorB; }
	GLfloat getAlpha() const { return colorA; }
};

class GL_Rect
{
	GLfloat x1;
	GLfloat y1;
	GLfloat x2;
	GLfloat y2;
	GLfloat angle;
	GLfloat rotX;
	GLfloat rotY;
	GL_Color color;
public:
	GL_Rect() {}
	GL_Rect(GLfloat _x1, GLfloat _y1, GLfloat _x2, GLfloat _y2, GLfloat _angle = 0.0f, GLfloat _rotX = 0.0f, GLfloat _rotY = 0.0f, GL_Color _color = GL_Color::black):
		x1(_x1), y1(_y1), x2(_x2), y2(_y2), angle(_angle), rotX(_rotX), rotY(_rotY), color(_color) {}
	GLfloat getX1() const { return x1; }
	GLfloat getY1() const { return y1; }
	GLfloat getX2() const { return x2; }
	GLfloat getY2() const { return y2; }
	GLfloat getAngle() const { return angle; }
	GLfloat getRotateX() const { return rotX; }
	GLfloat getRotateY() const { return rotY; }
	GL_Color getColor() const { return color; }
};

class ContextInterface
{
public:
	virtual void addEventFromObj(int label, id source, id dest, LONG64 timeDiff) = 0;
	virtual CriticalSection& getObjCriticalSection() = 0;
};

class DrawingContextInterface
{
public:
	virtual void redrawScene() = 0;
	virtual LONG getHeight() const = 0;
	virtual LONG getWidth() const = 0;
	virtual void drawDisk(GLfloat x, GLfloat y, GLdouble r, GLint slices = 20, GLint loops = 1, GL_Color = GL_Color::black) = 0;
	virtual void drawRect(GL_Rect rect) = 0;
	virtual void initScene(GL_Color color = GL_Color::white) = 0;
	virtual void refreshScreen() = 0;
	virtual void makeCurrent() = 0;
	virtual void unmakeCurrent() = 0;
	virtual ~DrawingContextInterface() {}
};

class Event
{
	int label;
	id source;
	id dest;
	LONG64 time;
	LONG64 timestamp;

public:
	Event(int _label, id _source, id _dest, LONG64 _time, LONG64 _timestamp):
		label(_label), source(_source), dest(_dest), time(_time), timestamp(_timestamp) {}
	LONG64 getTimestamp() const {return timestamp;}
	int getLabel() const { return label; }
	id getDestId() const { return dest; }
	~Event() {}
};

class EventComparator
{
public:
	EventComparator() {}
	bool operator()(const Event& e1, const Event& e2) { return e1.getTimestamp() > e2.getTimestamp(); }
	bool operator()(const Event* e1, const Event* e2) { return e1->getTimestamp() > e2->getTimestamp(); }
	~EventComparator() {}
};

class KR_Object
{

	ContextInterface *context;

protected:
	void sendEvent(int label, id dest, LONG64 timeDiff) { 
		if (context) context->addEventFromObj(label, getId(), dest, timeDiff); }

public:

	virtual void wakeup() = 0;
	virtual bool isInitObj() const = 0;
	virtual void recieveEvent(const Event& e) = 0;
	virtual id getId() const = 0;
	void setContext(ContextInterface *_context) { context = _context; }
	virtual void stop() = 0;
};

class GL_DiskObj
{
public:
	virtual float getX() const = 0;
	virtual float getY() const = 0;
	virtual float getR() const = 0;
	virtual GL_Color getColor() const = 0;
	virtual bool isInitObj() const = 0; 
};

class GL_WagonObj
{
public:
	virtual GL_Rect getSeatRect() const = 0;
	virtual float getIntervalX() const = 0;
	virtual float getIntervalY() const = 0;
	virtual int getNumOnSide() const = 0;
};

class IncorrectDataException: public std::exception
{
	public:
		IncorrectDataException() : exception() {}
		IncorrectDataException(const char* const& msg) : exception(msg) {}
};

class NullPointerException: public std::exception
{
	public:
		NullPointerException() : exception() {}
		NullPointerException(const char* const& msg) : exception(msg) {}
};