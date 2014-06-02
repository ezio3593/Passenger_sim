#pragma once

#include <vector>
#include "Vector2D.h"

class Goal
{
	Vector2D point;

	Goal* nextLevelGoal;
	Goal* prevLevelGoal;
	
	std::vector<Goal*>* seatsPr;

	bool isOccupied;

	Vector2D seatPoint;

public:
	Goal(const Vector2D& _point): point(_point), isOccupied(false),
		nextLevelGoal(NULL), prevLevelGoal(NULL), seatsPr(NULL) {}

	Goal* getNextLevelGoal() const { return nextLevelGoal; }
	Goal* getPrevLevelGoal() const { return prevLevelGoal; }
	std::vector<Goal*>* getSeatGoalPr() const { return seatsPr; }

	Vector2D getPoint() const { return point; }
	Vector2D getSeatPoint() const { return seatPoint; }

	void setPoint(const Vector2D& _point) { point = _point; }
	void setSeatPoint(const Vector2D& _seatPoint) { seatPoint = _seatPoint; }
	void setNextLevelGoal(Goal* goals) { nextLevelGoal = goals; }
	void setPrevLevelGoal(Goal* goals) { prevLevelGoal = goals; }
	void setSeatGoalPr(std::vector<Goal*>* goals) { seatsPr = goals; }
	void setIsOcuupied(bool _isOccupied) { isOccupied = _isOccupied; }

	bool getIsOccupied() const { return isOccupied; }

	~Goal() {}
};