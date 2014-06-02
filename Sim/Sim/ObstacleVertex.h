#pragma once

#include "Vector2D.h"
#include <list>

class ObstacleVertex
{
	Vector2D point;
	Vector2D dir;

	ObstacleVertex* nextVert;
	ObstacleVertex* prevVert;
public:
	ObstacleVertex() {}
	ObstacleVertex(const Vector2D &pointVector, const Vector2D &direction) { point = pointVector; dir = direction; }

	void setPoint(const Vector2D &pointVector) { point = pointVector; }
	void setDirection(const Vector2D &direction) { dir = direction; }

	Vector2D getPoint() const { return point; }
	Vector2D getDir() const { return dir; }

	void setNextObstVertex(ObstacleVertex* nextVertex) { nextVert = nextVertex; }
	void setPrevObstVertex(ObstacleVertex* prevVertex) { prevVert = prevVertex; }

	ObstacleVertex* getNextObstVertex() { return nextVert; }
	ObstacleVertex* getPrevObstVertex() { return prevVert; }
};