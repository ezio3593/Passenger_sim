#pragma once

#include <vector>
#include "KdTree.h"
#include "Goal.h"

class WagonEnviroment: public GL_WagonObj
{
	std::vector<ObstacleVertex*> obstVerts;
	std::vector<Agent*> agents;
	
	KdObstacleTree kdObstacleTree;

	std::vector<Goal*> goalsLevel3;
	std::vector<Goal*> goalsLevel2;
	std::vector<Goal*> goalsLevel1;

	GL_Rect seatRect;
	float intX;
	float intY;
	float numSeatsOnSide;
	
	float middleY;

public:
	WagonEnviroment() {}

	// Add seats in wagon and compute goals distrubution

	bool addSeats(const std::vector<Vector2D> &seat, int numOnSide, int intervalX, int intervalY, float passRadius);

	bool addObstacte(const std::vector<Vector2D> &verts); 

	void addAgent(Agent* agent) { if (agent) agents.push_back(agent); }

	void buildKdObstTree() { kdObstacleTree.buildObstacleTree(obstVerts); }

	void getAgentNeighbors(const Agent* agent, float maxDistSq, std::vector<Agent*>& res);
	
	void getObstNeighbors(const Agent* agent, float maxDistSq, std::vector<ObstacleVertex*>& res) 
	{ kdObstacleTree.computeObstacleNeighbors(agent, maxDistSq, res); }

	Goal* getStartGoal() { return goalsLevel1[std::rand()%goalsLevel1.size()]; }

	std::vector<ObstacleVertex*>& getObstaclesVertexs() { return obstVerts; }
	std::vector<Agent*> getAgent() { return agents; }

	std::vector<Goal*>& getSeatGoals() { return goalsLevel3; }

	void clearAgentPool() { agents.erase(agents.begin(), agents.end()); } 

	GL_Rect getSeatRect() const { return seatRect; }
	float getIntervalX() const  { return intX; }
	float getIntervalY() const { return intY; }

	float getMiddleY() const { return middleY; }

	int getNumOnSide() const  {return numSeatsOnSide; }

	~WagonEnviroment();
};