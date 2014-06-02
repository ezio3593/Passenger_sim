#include <Windows.h>
#include <string>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <vector>
#include "Objects.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "glu32.lib")

class DrawingContextException: public std::exception
{
	public:
		DrawingContextException() : exception() {}
		DrawingContextException(const char* const& msg) : exception(msg) {}
};

class DrawingContext: public DrawingContextInterface
{
	class Texture
	{
		int height;
		int width;
		char *colorBytes;

	public:
		Texture(): width(0), height(0), colorBytes(NULL) {}
		int initFromFile(std::string filename);
		char *getColorBytes() const { return colorBytes; }
		int getHeight() const { return height; }
		int getWidth() const { return width; }
		~Texture() { if (colorBytes) delete colorBytes; }

	};

	HDC deviceContext;
	HGLRC renderContext;

	CStatic* drawElem;
	std::vector<GL_DiskObj*> diskObjPool;
	
	GL_WagonObj* wagonObj;

	CriticalSection cs;
	CriticalSection &objCs;
	
	HANDLE threadHandle;

	LONG height;
	LONG width;
	int redrawInterval;

	bool isStart;

	static unsigned int __stdcall threadFunction(void* p);

	void setPixelFormat();

public:
	DrawingContext(CriticalSection &_objCs): isStart(false), threadHandle(NULL), redrawInterval(20), objCs(_objCs), wagonObj(NULL) {}
	LONG getHeight() const { return height; } 
	LONG getWidth() const { return width; }
	int  init(CStatic *pc);
	void initScene(GL_Color color = GL_Color::white);
	int  loadRectTexture(std::string filename);
	void enableTexture() { glEnable(GL_TEXTURE_2D); }
	void disableTexture() { glDisable(GL_TEXTURE_2D); }
	void makeCurrent() { wglMakeCurrent(deviceContext, renderContext); }
	void unmakeCurrent() { wglMakeCurrent(NULL, NULL); }
	void drawRect(GL_Rect rect);
	void drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	void drawDisk(GLfloat x, GLfloat y, GLdouble r, GLint slices = 20, GLint loops = 1, GL_Color = GL_Color::black);
	void redrawScene();
	void startRedrawingThread();
	void stopRedrawingThread();
	void addObject(GL_DiskObj* obj);
	void refreshScreen() { SwapBuffers(deviceContext); }
	
	void clearObjPool() { diskObjPool.erase(diskObjPool.begin(), diskObjPool.end()); }

	void addWagonEnv(GL_WagonObj* wagon) { wagonObj = wagon; }

	~DrawingContext();
};