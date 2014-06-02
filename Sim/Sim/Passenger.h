#pragma once

#include <vector>
#include "Objects.h"
#include "ObstacleVertex.h"
#include "Vector2D.h"
#include "Goal.h"

class Agent
{
protected:
	Vector2D position;
	Vector2D velocity;
	Vector2D prefVelocity;

	float radius;
	float personalSpace;

	float maxSpeed;
	float neighborDist;
	float timeStep;

	virtual void computeNeighbors() = 0;
	virtual void updateVelocity() = 0;

	void updatePosition() { position += velocity * timeStep; }
public:
	virtual Vector2D getPosition() const { return position;}
	virtual Vector2D getVelocity() const { return velocity;}
	virtual float getRadius() const { return radius; }
	virtual float getPersonalSpace() const { return personalSpace; }

	virtual ~Agent() {}
};

class Passenger: public Agent, public KR_Object, public GL_DiskObj
{
	enum eventLabel
	{
		MOVE
	};

	WagonEnviroment* env;
	Goal* currentGoal;

	std::vector<Agent*> neighborsAgents;
	std::vector<ObstacleVertex*> neighborsObstVerts;
	std::vector<Line> orcaLines;

	float timeCollisionAgent;
	float timeCollisionObst;
	
	float allowableDistSqToSubGoal;
	float allowableDistSqToGoal;

	LONG64 timeStepInMS;

	bool isInit;
	bool isStart;
	bool isAchievedGoal;

	bool isBoarding;
	bool isSubGoal;

	void computeNeighbors();
	void updateVelocity();

	bool checkIsAchievedGoal(bool isSubGoal);
	void toNextGoal(bool isBoarding); 
	void setPrefVelocity() { if (currentGoal) prefVelocity = normalize(currentGoal->getPoint() - position) * maxSpeed; }
	bool chooseOptimalGoal(Goal* goal);
public:
	Passenger(WagonEnviroment* _env) : env(_env), isStart(false), isInit(false) {};

	Passenger(WagonEnviroment* _env, float _neighborDist, 
		float _timeCollisionAgent, float _timeCollisionObst, float _radius,
		float _maxSpeed, float _personalSpace, float _allowableDistSqToSubGoal, 
		float _allowableDistSqToGoal, float _timeStep, const Vector2D &_velocity);

	void start() { if (!isStart) { isStart = true; sendEvent(MOVE, getId(), timeStepInMS);} }
	void wakeup();
	bool isInitObj() const {return isInit;}
	void recieveEvent(const Event& e);
	id getId() const {return this;}
	void stop() { isStart = false; }

	void setNeighborDist(float _neighborDist) { neighborDist = _neighborDist; }
	void setTimeCollisionAgent(float _timeCollisionAgent) { timeCollisionAgent = _timeCollisionAgent; }
	void setTimeCollisionObst(float _timeCollisionObst) { timeCollisionObst = _timeCollisionObst; }
	void setRadius(float _radius) { radius = _radius; }
	void setMaxSpeed(float _maxSpeed) { maxSpeed = _maxSpeed; }
	void setPersonalSpace(float _personalSpace) { personalSpace = _personalSpace; }
	void setVelocity(const Vector2D& _velocity) { velocity = _velocity; }
	void setTimeStep(float _timeStep) { timeStep = timeStep; }
	void setPosition(const Vector2D& pos) { position = pos; }
	void setIsBoarding(bool _isBoarding) { isBoarding = _isBoarding; }
	void setGoal(Goal* goal, bool isSubGoal);

	void setEnviroment(WagonEnviroment* _env) { env = _env; }

	float getNeighborDist() const { return neighborDist; }
	float getTimeCollisionAgent() const { return timeCollisionAgent; }
	float getTimeCollisionObst() const { return timeCollisionObst; }
	float getRadius() const { return radius; }
	float getMaxSpeed() const { return maxSpeed; }
	float getPersonalSpace() const { return personalSpace; }
	float getTimeStep() const { return timeStep; }
	Vector2D getVelocity() const { return velocity; }
	bool getIsBoarding() const { return isBoarding; }

	float getX() const { return position.getX(); }
	float getY() const { return position.getY(); }
	float getR() const { return radius; }
	GL_Color getColor() const { 
		return isAchievedGoal ? GL_Color::green : GL_Color::black; }

	~Passenger() {}
};

bool linearProgram1(const std::vector<Line> &lines, size_t lineNo,
					float radius, const Vector2D &optVelocity,
					bool directionOpt, Vector2D &result);
	
size_t linearProgram2(const std::vector<Line> &lines, float radius,
						const Vector2D &optVelocity, bool directionOpt,
						Vector2D &result);

void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine,
					float radius, Vector2D &result);