#include "stdafx.h"
#include "DrawingContext.h"
#include <fstream>


int DrawingContext::Texture::initFromFile(std::string filename)
{
	int BITMAP_ID = 0x4d42;

	std::ifstream ifs;
	ifs.open(filename.c_str(), std::ifstream::binary);

    BITMAPINFOHEADER bitmapInfoHeader;
    BITMAPFILEHEADER header;

	ifs.read((char *)&header, sizeof(BITMAPFILEHEADER));

	if (ifs.fail()) 
	{
		ifs.close();
		return -1;
	}

    if(header.bfType != BITMAP_ID)
    {
		ifs.close();
		return -1;
    }
 
	ifs.read((char *)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));

	if (ifs.fail()) 
	{
		ifs.close();
		return -1;
	}

	width = bitmapInfoHeader.biWidth;
	height = bitmapInfoHeader.biHeight;

	ifs.seekg(header.bfOffBits, ifs.beg);

	colorBytes = new char[bitmapInfoHeader.biSizeImage];

	ifs.read(colorBytes, bitmapInfoHeader.biSizeImage);

	if (ifs.fail()) 
	{
		ifs.close();
		return -1;
	}

    ifs.close();
	return 0;
}

unsigned int DrawingContext::threadFunction(void *p)
{
	DrawingContext *dc = reinterpret_cast<DrawingContext*>(p);

	bool flag = false;
	{
		Lock l(dc->cs);
		flag = dc->isStart;
	}

	dc->makeCurrent();
	
	while (flag)
	{
		
		dc->redrawScene();

		Sleep(dc->redrawInterval);

		{
			Lock l(dc->cs);
			flag = dc->isStart;
		}
	}

	dc->unmakeCurrent();

	return 0;
}

void DrawingContext::startRedrawingThread()
{
	if (!isStart)
	{
		isStart = true;
		
		threadHandle = (HANDLE)_beginthreadex(0, 0, threadFunction, reinterpret_cast<void*>(this), 0, NULL);

		if (!threadHandle) throw DrawingContextException("Cannot create drawing thread");

	}
}

void DrawingContext::stopRedrawingThread()
{
	if (isStart)
	{
		{
			Lock l(cs);
			isStart = false;
		}

		if (threadHandle)
		{
			WaitForSingleObject(threadHandle, INFINITE);
			threadHandle = NULL;
		}
	}
}


int DrawingContext::init(CStatic *elem)
{
	Lock l(cs);

	if (!elem) return -1;

	deviceContext = elem->GetDC()->m_hDC;
	drawElem = elem;

	LPRECT lpr = new RECT();
	elem->GetWindowRect(lpr);

	width = lpr->right - lpr->left;
	height = - (lpr->top - lpr->bottom);

	delete lpr;

	setPixelFormat();

	renderContext = wglCreateContext(deviceContext);

	if (!renderContext) return -1;

	makeCurrent();

	return 0;
}



void DrawingContext::initScene(GL_Color color)
{	
	glViewport(0,0, width, height);
	
	glShadeModel(GL_SMOOTH);
	glClearColor(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0, width, 0.0, height);

}

void DrawingContext::drawRect(GL_Rect rect)
{
	glPushMatrix();
		
		glTranslatef(rect.getRotateX(), rect.getRotateY(), 0.0f);
		glRotatef(rect.getAngle(), 0.0f, 0.0f, 1.0f);	
		glTranslatef(-rect.getRotateX(), -rect.getRotateY(), 0.0f);

		GL_Color color = rect.getColor();

		if (glIsEnabled(GL_TEXTURE_2D))
		{
			glBegin(GL_LINE_LOOP);
				glColor3f(color.getRed(), color.getGreen(), color.getBlue());
				glTexCoord2f(0.0,0.0);
				glVertex2f(rect.getX1(), rect.getY1());
				glTexCoord2f(1.0,0.0);
				glVertex2f(rect.getX2(), rect.getY1());
				glTexCoord2f(1.0,1.0); 
				glVertex2f(rect.getX2(), rect.getY2());
				glTexCoord2f(0.0,1.0);
				glVertex2f(rect.getX1(), rect.getY2());
			glEnd();

			return;
		}

		glBegin(GL_LINE_LOOP);
			glColor3f(color.getRed(), color.getGreen(), color.getBlue());
			glVertex2f(rect.getX1(), rect.getY1());
			glVertex2f(rect.getX2(), rect.getY1()); 
			glVertex2f(rect.getX2(), rect.getY2());
			glVertex2f(rect.getX1(), rect.getY2());
		glEnd();

	glPopMatrix();
}

void DrawingContext::drawDisk(GLfloat x, GLfloat y, GLdouble r, GLint slices, GLint loops, GL_Color color)
{
	glPushMatrix();
		GLUquadricObj* obj = gluNewQuadric();

		glTranslatef(x, y, 0);

		glColor3f(color.getRed(), color.getGreen(), color.getBlue());
		gluDisk(obj, 0, r, slices, loops);
	glPopMatrix();
}

void DrawingContext::drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glBegin(GL_LINES);
			glVertex2f(x1, y1);
			glVertex2f(x2, y2);
	glEnd();
}

void DrawingContext::setPixelFormat()
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof (PIXELFORMATDESCRIPTOR),	// strcut size 
		1,						// Version number
		PFD_DRAW_TO_WINDOW |    // Flags, draw to a window,
		PFD_SUPPORT_OPENGL |    // use OpenGL
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,          // RGBA pixel values
		32,                     // 32-bit color
		0, 0, 0,                // RGB bits & shift sizes.
		0, 0, 0,                // Don't care about them
		0, 0,                   // No alpha buffer info
		0, 0, 0, 0, 0,          // No accumulation buffer
		32,                     // 32-bit depth buffer
		0,                      // No stencil buffer
		0,                      // No auxiliary buffers
		PFD_MAIN_PLANE,         // Layer type
		0,                      // Reserved (must be 0)
		0,                      // No layer mask
		0,                      // No visible mask
		0                       // No damage mask
	};

    int nMyPixelFormatID = ChoosePixelFormat(deviceContext, &pfd );

    SetPixelFormat(deviceContext, nMyPixelFormatID, &pfd );
}

int DrawingContext::loadRectTexture(std::string filename)
{
	Texture texture;

	if (!texture.initFromFile(filename))
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, texture.getWidth(), texture.getHeight(), 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.getColorBytes());
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		return 0;
	} else return -1;
}

void DrawingContext::addObject(GL_DiskObj* obj)
{
	if (obj)
	{
		Lock l(cs);
		diskObjPool.push_back(obj);
	}
}

void DrawingContext::redrawScene()
{
	initScene();

	{
		Lock l(cs);

		int size = diskObjPool.size();
		
		Lock objL(objCs);
		for (int i = 0; i < size; ++i)
		{
			GL_DiskObj *obj = diskObjPool[i];
			drawDisk(obj->getX(), obj->getY(), obj->getR(), 20, 1, obj->getColor());
		}
	}
	
	// draw wagon
	
	if (wagonObj)
	{
		GLfloat x1 = wagonObj->getSeatRect().getX1();
		GLfloat x2 = wagonObj->getSeatRect().getX2();
		GLfloat y1 = wagonObj->getSeatRect().getY1();
		GLfloat y2 = wagonObj->getSeatRect().getY2();

		GLfloat intX = wagonObj->getIntervalX();
		GLfloat intY = wagonObj->getIntervalY();

		GLfloat delta = 0;
		for (int i = 0; i < wagonObj->getNumOnSide(); ++i)
		{
			drawRect(GL_Rect(x1 + delta, y1, x2 + delta, y2));
			drawRect(GL_Rect(x1 + delta, y1 - intY, x2 + delta, y2 - intY));

			if (i != wagonObj->getNumOnSide()-1) drawLine(x2 + delta, y1 - intY, x1 + intX + delta, y1 - intY);
			
			delta += intX;
		}
		
		drawLine(x1, y1, x1, y2 - intY);
		drawLine(x2 + delta - intX, y1, x2 + delta - intX, y2 - intY);

	}

	refreshScreen();
}

DrawingContext::~DrawingContext()
{
	stopRedrawingThread();
	
	if (threadHandle)
	{
		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
	}

	
	if (renderContext) 
		wglDeleteContext(renderContext);

	unmakeCurrent();
}