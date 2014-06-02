#include "stdafx.h"
#include "WagonEnviroment.h"

bool WagonEnviroment::addObstacte(const std::vector<Vector2D> &verts)
{
	if (verts.size() < 2) return false;

	const size_t size = verts.size();
	const size_t firstVertsIndex = obstVerts.size();

	for (size_t i = 0; i < size; ++i) 
	{
		ObstacleVertex *vertex = new ObstacleVertex();
		vertex->setPoint(verts[i]);

		if (i != 0) 
		{
			vertex->setPrevObstVertex(obstVerts.back());
			vertex->getPrevObstVertex()->setNextObstVertex(vertex);
		}

		if (i == size - 1) 
		{	
			vertex->setNextObstVertex(obstVerts[firstVertsIndex]);
			vertex->getNextObstVertex()->setPrevObstVertex(vertex);
		}

		vertex->setDirection(normalize(verts[(i == size - 1 ? 0 : i + 1)] - verts[i]));

		bool isConvex = false;
		if (size == 2) 
		{
			isConvex = true;
		}
		else 
		{
			isConvex = isLeftOf(verts[(i == 0 ? size - 1 : i - 1)], verts[i], verts[(i == verts.size() - 1 ? 0 : i + 1)]);
		}

		if (!isConvex) return false;

		obstVerts.push_back(vertex);
	}

	return true;
}

bool WagonEnviroment::addSeats(const std::vector<Vector2D> &seat, int numOnSide, int intervalX, int intervalY, float passRadius)
{
	if (numOnSide <= 0 || seat.size() != 4) return false;
	
	float maxX = seat[0].getX();
	float minX = seat[0].getX();
	float maxY = seat[0].getY();
	float minY = seat[0].getY();

	for(size_t i = 1; i < seat.size(); ++i)
	{
		maxX = std::max(maxX, seat[i].getX());
		minX = std::min(minX, seat[i].getX());
		maxY = std::max(maxY, seat[i].getY());
		minY = std::min(minY, seat[i].getY());
	}

	float length = maxX - minX;
	float width = maxY - minY;

	Vector2D delta;

	std::vector<Vector2D> door;

	Vector2D vectIntX(intervalX, 0);
	Vector2D vectIntY(0, intervalY);


	std::vector<Vector2D> verts1;
	std::vector<Vector2D> verts2;

	for(int i = 0; i < numOnSide; ++i)
	{
		verts1.clear();
		verts2.clear();

		for (size_t j = 0; j < seat.size(); ++j)
		{
		
			verts1.push_back(seat[j] + delta);
			verts2.push_back(seat[j] + delta - vectIntY);
		}

		// Closed doors on the other side are obstacles

		door.clear();
		door.push_back(Vector2D(maxX + delta.getX(), minY - intervalY));
		door.push_back(Vector2D(minX + intervalX + delta.getX(), minY - intervalY));

		delta += vectIntX;

		if (i == numOnSide - 1) addObstacte(door);

		if (!addObstacte(verts1) || !addObstacte(verts2)) return false;
	}

	// Side doors are obstacles

	door.clear();

	door.push_back(Vector2D(minX, minY));
	door.push_back(Vector2D(minX, maxY - intervalY));

	addObstacte(door);

	door.clear();

	float x = maxX + delta.getX() - intervalX;

	door.push_back(Vector2D(x, minY));
	door.push_back(Vector2D(x, maxY - intervalY));

	addObstacte(door);

	door.clear();

	// Set params for draw

	seatRect = GL_Rect(minX, minY, maxX, maxY);
	intX = intervalX;
	intY = intervalY;
	numSeatsOnSide = numOnSide;

	// Create first and second level goals
	
	float middleWidthWagon = maxY - (intervalY + width) / 2.0f; 
	
	middleY = middleWidthWagon;

	x = maxX + (intervalX - length) / 2.0f;

	for (int i = 0; i < numOnSide-1; ++i)
	{
		Goal* g1 = new Goal(Vector2D(x, maxY + 6*passRadius));
		Goal* g2 = new Goal(Vector2D(x, middleWidthWagon));

		goalsLevel1.push_back(g1);
		goalsLevel2.push_back(g2);

		g1->setNextLevelGoal(g2);
		g2->setPrevLevelGoal(g1);

		x += intervalX;
	}

	// Create third level goals

	int numSeatParts = int(length / (2*passRadius));
	float partSeatSize = length / numSeatParts;

	int splitX = length / 2.0 + intervalX;
	int indexInLevel2 = 0;

	float startX = minX + partSeatSize / 2.0f;

	for (int i = 0; i < numOnSide; ++i)
	{
		for (int j = 0; j < numSeatParts; ++j)
		{
			x = startX + j * partSeatSize + i * intervalX;

			// Right side

			Goal* g3prRight = new Goal(Vector2D(x, middleWidthWagon));
			Goal* g3right = new Goal(Vector2D(x, minY - passRadius));

			g3prRight->setNextLevelGoal(g3right);
			g3right->setPrevLevelGoal(g3prRight);
			g3right->setSeatPoint(Vector2D(x, minY + width / 2.0f));

			goalsLevel3.push_back(g3prRight);
			
			// Left side

			Goal* g3prLeft = new Goal(Vector2D(x, middleWidthWagon));
			Goal* g3left = new Goal(Vector2D(x, maxY + passRadius - intervalY));

			g3prLeft->setNextLevelGoal(g3left);
			g3left->setPrevLevelGoal(g3prLeft);
			g3left->setSeatPoint(Vector2D(x, maxY - width / 2.0f - intervalY));

			if (x > splitX && indexInLevel2 < goalsLevel2.size()-1)
			{
				++indexInLevel2;
				splitX += intervalX;
			}

			g3prLeft->setPrevLevelGoal(goalsLevel2[indexInLevel2]);
			g3prRight->setPrevLevelGoal(goalsLevel2[indexInLevel2]);

			goalsLevel3.push_back(g3prLeft);
		}
	}

	for (size_t i = 0; i < goalsLevel2.size(); ++i)
	{
		goalsLevel2[i]->setSeatGoalPr(&goalsLevel3);
	}

	return true;
}

void WagonEnviroment::getAgentNeighbors(const Agent* agent, float maxDistSq, std::vector<Agent*>& res)
{
	for (size_t i = 0; i < agents.size(); ++i)
	{
		if (agent != agents[i] && absSq(agent->getPosition() - agents[i]->getPosition()) <= maxDistSq)
		{
			res.push_back(agents[i]);
		}
	}
}

WagonEnviroment::~WagonEnviroment()
{
	for(size_t i = 0; i < goalsLevel1.size(); ++i)
	{
		delete goalsLevel1[i];
	}

	for(size_t i = 0; i < goalsLevel2.size(); ++i)
	{
		delete goalsLevel2[i];
	}

	for(size_t i = 0; i < goalsLevel3.size(); ++i)
	{
		delete goalsLevel3[i];
	}

	for(size_t i = 0; i < obstVerts.size(); ++i)
	{
		delete obstVerts[i];
	}
}