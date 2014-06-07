#include "stdafx.h"
#include "Passenger.h"
#include "WagonEnviroment.h"
#include <limits> 

void Passenger::wakeup()
{
	isInit = true;

	neighborDist = 30;
	timeCollisionAgent = 15;
	timeCollisionObst = 15;
	radius = 7;
	maxSpeed = 3;
	isAchievedGoal = true;

	timeStep = 0.15f;
	timeStepInMS = timeStep * 100;

	allowableDistSqToSubGoal = sqr(2.5f * radius);
	allowableDistSqToGoal = sqr(radius);

	currentGoal = NULL;
}

Passenger::Passenger(WagonEnviroment* _env, float _neighborDist, float _timeCollisionAgent, 
		float _timeCollisionObst, float _radius,
		float _maxSpeed, float _allowableDistSqToSubGoal, 
		float _allowableDistSqToGoal, float _timeStep = 0.20f, const Vector2D &_velocity = Vector2D()):
isStart(false), isInit(false)
{
	neighborDist = _neighborDist;
	timeCollisionAgent = _timeCollisionAgent;
	timeCollisionObst = _timeCollisionObst;
	radius = _radius;
	maxSpeed = _maxSpeed;
	isAchievedGoal = true;

	timeStep = _timeStep;
	timeStepInMS = timeStep * 100;

	env = _env;

	allowableDistSqToSubGoal = _allowableDistSqToSubGoal;
	allowableDistSqToGoal = _allowableDistSqToGoal;

	currentGoal = NULL;
}

void Passenger::recieveEvent(const Event& e)
{
	int label = e.getLabel();
	
	switch(label)
	{
	case MOVE:

		if (!currentGoal) 
		{
			isSubGoal = true;
			setGoal(env->getStartGoal(), isSubGoal);
			setPrefVelocity();
			velocity = prefVelocity;
		}

		if (!isAchievedGoal)
		{
			toNextGoal(isBoarding);

			setPrefVelocity();
			computeNeighbors();
			updateVelocity();
			updatePosition();
			sendEvent(MOVE, getId(), timeStepInMS);
		}

		break;
	}
}

void Passenger::toNextGoal(bool isBoard)
{
	if (checkIsAchievedGoal(isSubGoal))
	{
		if (isBoard)
		{
			isSubGoal = currentGoal->getNextLevelGoal() != NULL || currentGoal->getSeatGoalPr() != NULL;

			if (!isSubGoal) 
			{
				isAchievedGoal = true;
				currentGoal->setIsOcuupied(true);

				position = currentGoal->getSeatPoint();
			}

			if (currentGoal->getNextLevelGoal() != NULL)
			{
				setGoal(currentGoal->getNextLevelGoal(), isSubGoal);
			} 
			else
			{
				chooseOptimalGoal(currentGoal);
			}
		} else
		{
			isSubGoal = currentGoal->getPrevLevelGoal() != NULL;
			
			if (currentGoal->getNextLevelGoal() == NULL && currentGoal->getSeatGoalPr() == NULL)
			{
				currentGoal->setIsOcuupied(false);
				position = currentGoal->getPoint();
			}

			if (!isSubGoal) 
			{
				//isAchievedGoal = true;
				setGoal(new Goal(Vector2D(currentGoal->getPoint() + Vector2D(0,200))), false);
			} else 
			{
				setGoal(currentGoal->getPrevLevelGoal(), isSubGoal);
			}
		}
	} else if (currentGoal->getNextLevelGoal() != NULL && currentGoal->getNextLevelGoal()->getIsOccupied())
	{
		chooseOptimalGoal(currentGoal->getPrevLevelGoal());
	} else if (currentGoal->getIsOccupied())
	{
		chooseOptimalGoal(currentGoal->getPrevLevelGoal()->getPrevLevelGoal());
	}

}

bool Passenger::chooseOptimalGoal(Goal* goal)
{
	if (!goal) return true;

	if (goal->getSeatGoalPr() != NULL)
	{

		std::vector<Goal*>* goals = goal->getSeatGoalPr();

		float minDist = std::numeric_limits<float>::infinity();
		float dist = 0;
		size_t optIndex = -1;

		for (size_t  i = 0; i < goals->size(); ++i)
		{
			if ((*goals)[i]->getNextLevelGoal()->getIsOccupied()) 
				continue;

			dist = std::fabs((*goals)[i]->getPoint().getX() - position.getX());

			if (dist < radius) 
			{
				optIndex = i;
				break;
			}

			if (dist < minDist) 
			{
				minDist = dist;
				optIndex = i;
			} 
		}

		if (optIndex == -1) return false;

		setGoal((*goals)[optIndex], isSubGoal);

		return true;
	}

	return true;
}

void Passenger::setGoal(Goal* goal, bool isSubGoal)
{
	if (!goal) return;
	
	currentGoal = goal;

	checkIsAchievedGoal(isSubGoal);
}

void Passenger::computeNeighbors()
{
	neighborsObstVerts.clear();

	float maxDistSq = sqr(timeCollisionObst * maxSpeed + radius);
	
	if (env) 
	{
		env->getObstNeighbors(this, maxDistSq, neighborsObstVerts);
	} else throw NullPointerException("Env is null");

	neighborsAgents.clear();

	maxDistSq = sqr(neighborDist);
	
	if (env) 
	{
		env->getAgentNeighbors(this, maxDistSq, neighborsAgents);
	} else throw NullPointerException("Env is null");
}

bool Passenger::checkIsAchievedGoal(bool isSubGoal)
{
	if (!currentGoal) return true;

	const float distSq = absSq(currentGoal->getPoint() - position);

	bool res = isSubGoal ? (distSq < allowableDistSqToSubGoal) : (distSq < allowableDistSqToSubGoal);

	isAchievedGoal = res && !isSubGoal;

	return res;
}

void Passenger::updateVelocity()
{
	orcaLines.clear();
	
	const float invTimeCollisionObst = 1.0f / timeCollisionObst;

	// Create obstacle ORCA lines

	for (size_t i = 0; i < neighborsObstVerts.size(); ++i) 
	{

		ObstacleVertex *vert1 = neighborsObstVerts[i];
		ObstacleVertex *vert2 = vert1->getNextObstVertex();
		
		const Vector2D relativePos1 = vert1->getPoint() - position;
		const Vector2D relativePos2 = vert2->getPoint() - position;


		bool alreadyCovered = false;

		for (size_t j = 0; j < orcaLines.size(); ++j) 
		{
			if (prVectMul(invTimeCollisionObst * relativePos1 - orcaLines[j].getPoint(), orcaLines[j].getDir()) - 
				invTimeCollisionObst * radius >= -EPSILON && prVectMul(invTimeCollisionObst * relativePos2 - orcaLines[j].getPoint(), orcaLines[j].getDir()) - 
				invTimeCollisionObst * radius >=  -EPSILON) 
			{
				alreadyCovered = true;
				break;
			}
		}

		if (alreadyCovered) continue;

		// Check for collisions

		const float distSq1 = absSq(relativePos1);
		const float distSq2 = absSq(relativePos2);

		const float radiusSq = sqr(radius);

		const Vector2D edgeVector = vert2->getPoint() - vert1->getPoint();
		const float projectOnEdge = (-relativePos1 * edgeVector) / absSq(edgeVector);
		const float distSqLine = absSq(-relativePos1 - projectOnEdge * edgeVector);

		Line line;

		if (projectOnEdge < 0.0f && distSq1 <= radiusSq) 
		{
			// Collision with left vertex 

			line.setPoint(Vector2D(0.0f, 0.0f));
			line.setDir(getNormal(relativePos1));
			orcaLines.push_back(line);

			continue;
		}
		else if (projectOnEdge > 1.0f && distSq2 <= radiusSq) 
		{
			// Collision with right vertex. 

			if (isLeftOf(relativePos2, vert2->getDir()))
			{
				// if next obstacle edge don't process this collision

				line.setPoint(Vector2D(0.0f, 0.0f));
				line.setDir(getNormal(relativePos2));
				orcaLines.push_back(line);
			}
			continue;
		}
		else if (projectOnEdge >= 0.0f && projectOnEdge <= 1.0f && distSqLine <= radiusSq) 
		{
			// Collision with obstacle edge

			line.setPoint(Vector2D(0.0f, 0.0f));
			line.setDir(-vert1->getDir());
			orcaLines.push_back(line);

			continue;
		}

		// No collision, compute tangents' dirs.

		Vector2D leftTangDirection, rightTangDirection;

		if (projectOnEdge < 0.0f && distSqLine <= radiusSq) 
		{
			// Left vertex defines VO

			vert2 = vert1;

			const float tangent1 = std::sqrt(distSq1 - radiusSq);
			leftTangDirection = Vector2D(relativePos1.getX() * tangent1 - relativePos1.getY() * radius, 
				relativePos1.getX() * radius + relativePos1.getY() * tangent1) / distSq1;
			rightTangDirection = Vector2D(relativePos1.getX() * tangent1 + relativePos1.getY() * radius,
				-relativePos1.getX() * radius + relativePos1.getY() * tangent1) / distSq1;
		}
		else if (projectOnEdge > 1.0f && distSqLine <= radiusSq) 
		{
			// Right vertex defines VO

			vert1 = vert2;

			const float tangent2 = std::sqrt(distSq2 - radiusSq);
			leftTangDirection = Vector2D(relativePos2.getX() * tangent2 - relativePos2.getY() * radius, 
				relativePos2.getX() * radius + relativePos2.getY() * tangent2) / distSq2;
			rightTangDirection = Vector2D(relativePos2.getX() * tangent2 + relativePos2.getY() * radius,
				-relativePos2.getX() * radius + relativePos2.getY() * tangent2) / distSq2;
		}
		else 
		{
			// Edge defines VO

			const float tangent1 = std::sqrt(distSq1 - radiusSq);
			leftTangDirection = Vector2D(relativePos1.getX() * tangent1 - relativePos1.getY() * radius,
				relativePos1.getX() * radius + relativePos1.getY() * tangent1) / distSq1;

			const float tangent2 = std::sqrt(distSq2 - radiusSq);
			rightTangDirection = Vector2D(relativePos2.getX() * tangent2 + relativePos2.getY() * radius,
				-relativePos2.getX() * radius + relativePos2.getY() * tangent2) / distSq2;
		}


		ObstacleVertex* leftNeighbor = vert1->getPrevObstVertex();

		bool isLeftLegForeign = false;
		bool isRightLegForeign = false;

		// Tangents' dirs correction

		if (isLeftOf(leftTangDirection, -leftNeighbor->getDir())) 
		{
			
			// Left tangent crosses leftNeighbor edge

			leftTangDirection = -leftNeighbor->getDir();
			isLeftLegForeign = true;
		}

		if (!isLeftOf(rightTangDirection, vert2->getDir())) 
		{
			
			// Right tangent crosses rightNeighbor edge

			rightTangDirection = vert2->getDir();
			isRightLegForeign = true;
		}

		//Compute truncated centers

		const Vector2D leftTruncCenter = invTimeCollisionObst  * (vert1->getPoint() - position);
		const Vector2D rightTruncCenter = invTimeCollisionObst * (vert2->getPoint() - position);
		const Vector2D truncVec = rightTruncCenter - leftTruncCenter;

		// Project current velocity on VO

		// Check if current velocity is projected on truncated circles

		const float projectOnTruncEdgeNormByTruncVec = (vert1 == vert2 ? 0.5f : ((velocity - leftTruncCenter) * truncVec) / absSq(truncVec));
		const float projectOnLeftTangent = ((velocity - leftTruncCenter) * leftTangDirection);
		const float projectOnRightTangent = ((velocity - rightTruncCenter) * rightTangDirection);

		if ((projectOnTruncEdgeNormByTruncVec < 0.0f && projectOnLeftTangent < 0.0f) || 
			(vert1 == vert2 && projectOnLeftTangent < 0.0f && projectOnRightTangent < 0.0f)) 
		{

			// Project on left circle of truncated VO

			const Vector2D uDir = velocity - leftTruncCenter;

			line.setDir(-getNormal(uDir));
			line.setPoint(leftTruncCenter + (radius * invTimeCollisionObst) * uDir);
			orcaLines.push_back(line);
			continue;
		}
		else if (projectOnTruncEdgeNormByTruncVec > 1.0f && projectOnRightTangent < 0.0f) 
		{

			// Project on right circle of truncated VO

			const Vector2D uDir = velocity - rightTruncCenter;

			line.setDir(-getNormal(uDir));
			line.setPoint(rightTruncCenter + (radius * invTimeCollisionObst) * uDir);
			orcaLines.push_back(line);
			continue;
		}


		const float distSqTruncEdge = ((projectOnTruncEdgeNormByTruncVec < 0.0f || projectOnTruncEdgeNormByTruncVec > 1.0f || vert1 == vert2) ? 
			std::numeric_limits<float>::infinity() : 
			absSq(velocity - (leftTruncCenter + projectOnTruncEdgeNormByTruncVec * truncVec)));

		const float distSqLeftTangent = ((projectOnLeftTangent < 0.0f) ? 
			std::numeric_limits<float>::infinity() : 
			absSq(velocity - (leftTruncCenter + projectOnLeftTangent * leftTangDirection)));

		const float distSqRightTangent = ((projectOnRightTangent < 0.0f) ? 
			std::numeric_limits<float>::infinity() : 
			absSq(velocity - (rightTruncCenter + projectOnRightTangent * rightTangDirection)));

		if (distSqTruncEdge <= distSqLeftTangent && distSqTruncEdge <= distSqRightTangent) 
		{
			// Project on edge of truncated VO
			
			line.setDir(-vert1->getDir());
			line.setPoint(leftTruncCenter + radius * invTimeCollisionObst * getNormal(line.getDir()));
			orcaLines.push_back(line);
			continue;
		}
		else if (distSqLeftTangent <= distSqRightTangent) 
		{
			
			// Project on left tangent
			
			if (isLeftLegForeign) continue;

			line.setDir(leftTangDirection);
			line.setPoint(leftTruncCenter + (radius * invTimeCollisionObst) * getNormal(line.getDir()));
			orcaLines.push_back(line);
			continue;
		}
		else 
		{
			// Project on right tangent

			if (isRightLegForeign) continue;

			line.setDir(-rightTangDirection);
			line.setPoint(rightTruncCenter + (radius * invTimeCollisionObst) * getNormal(line.getDir()));
			orcaLines.push_back(line);
			
			continue;
		}
	}

	const size_t numObstLines = orcaLines.size();

	const float invTimeCollisionAgent = 1.0f / timeCollisionAgent;

	// Create agent ORCA lines

	for (size_t i = 0; i < neighborsAgents.size(); ++i) 
	{
		const Agent* agent = neighborsAgents[i];

		const Vector2D relativePos = agent->getPosition() - position;
		const Vector2D relativeVelocity = velocity - agent->getVelocity();

		const float distSq = absSq(relativePos);
		const float sumRadius = radius + agent->getRadius();
		const float sumRadiusSq = sqr(sumRadius);

		Line line;
		Vector2D u;

		if (distSq > sumRadiusSq) 
		{
			// No collision

			// v - vector from truncated center to relative velocity
			
			const Vector2D v = relativeVelocity - (invTimeCollisionAgent * relativePos);

			const float absVSq = absSq(v);

			const float projectOnRelPos = v * relativePos;

			if (projectOnRelPos < 0.0f && sqr(projectOnRelPos) > sumRadiusSq * absVSq) 
			{
				// Project on truncated circle
				
				const float absV = std::sqrt(absVSq);
				const Vector2D unitW = v / absV;

				line.setDir(-getNormal(unitW));
				u = (sumRadius * invTimeCollisionAgent - absV) * unitW;
			}
			else 
			{
				// Project on tangents

				const float tangent = std::sqrt(distSq - sumRadiusSq);

				if (isLeftOf(relativePos, v)) 
				{
					// Project on left tangent

					line.setDir(Vector2D(relativePos.getX() * tangent - relativePos.getY() * sumRadius, 
						relativePos.getX() * sumRadius + relativePos.getY() * tangent) / distSq);
				}
				else 
				{
					// Project on right tangent

					line.setDir(-Vector2D(relativePos.getX() * tangent + relativePos.getY() * sumRadius, 
						-relativePos.getX() * sumRadius + relativePos.getY() * tangent) / distSq);
				}

				const float projectRelVelOnLine = relativeVelocity * line.getDir();

				u = projectRelVelOnLine * line.getDir() - relativeVelocity;
			}
		}
		else 
		{
			// Collision
			
			const float invTimeStep = 1.0f / timeStep;

			// Vector from trancated center to relative velocity

			const Vector2D v = relativeVelocity - invTimeStep * relativePos;

			const float absV = abs(v);
			const Vector2D normV = v / absV;

			line.setDir(-getNormal(normV));
			u = (sumRadius * invTimeStep - absV) * normV;
		}

		line.setPoint(velocity + 0.5f * u);
		orcaLines.push_back(line);
	}

	Vector2D newVelocity;
	size_t lineFail = linearProgram2(orcaLines, maxSpeed, prefVelocity, false, newVelocity);

	if (lineFail < orcaLines.size()) 
	{
		linearProgram3(orcaLines, numObstLines, lineFail, maxSpeed, newVelocity);
	}

	velocity = newVelocity;
}

bool linearProgram1(const std::vector<Line> &lines, size_t lineNo, float radius, const Vector2D &optVelocity, bool directionOpt, Vector2D &result)
{
	const float projectLinePointVectOnLine = lines[lineNo].getPoint() * lines[lineNo].getDir();

	const float projectRadiusOnLineSq = sqr(radius) - (absSq(lines[lineNo].getPoint()) - sqr(projectLinePointVectOnLine));

	if (projectRadiusOnLineSq < 0.0f) 
	{
		// Max speed circle fully invalidates line lineNo

		return false;
	}

	const float projectRadiusOnLine = std::sqrt(projectRadiusOnLineSq);
	float crossPointLeft = -projectLinePointVectOnLine - projectRadiusOnLine;
	float crossPointRight = -projectLinePointVectOnLine + projectRadiusOnLine;

	for (size_t i = 0; i < lineNo; ++i) 
	{
		const float denominator = prVectMul(lines[lineNo].getDir(), lines[i].getDir());
		const float numerator = prVectMul(lines[i].getDir(), lines[lineNo].getPoint() - lines[i].getPoint());

		if (std::fabs(denominator) <= EPSILON) 
		{
			// Lines lineNo and i are parallel
			
			if (numerator < 0.0f) 
			{
				// Restrictions of line No and line i are inconsistent

				return false;
			}
			else 
			{
				continue;
			}
		}

		const float crossPoint = numerator / denominator;

		if (denominator >= 0.0f) 
		{
			// Line i crosses line lineNo on the right
			
			crossPointRight = std::min(crossPointRight, crossPoint);
		}
		else 
		{
			// Line i crosses line lineNo on the left.

			crossPointLeft = std::max(crossPointLeft, crossPoint);
		}

		if (crossPointLeft > crossPointRight) 
		{
			return false;
		}
	}


	if (directionOpt) 
	{
		// Optimize direction

		if (optVelocity * lines[lineNo].getDir() > 0.0f) 
		{
			// Take right extreme
			
			result = lines[lineNo].getPoint() + crossPointRight * lines[lineNo].getDir();
		}
		else 
		{
			// Take left extreme
			
			result = lines[lineNo].getPoint() + crossPointLeft * lines[lineNo].getDir();
		}

		if (absSq(result) > (radius * radius)) result = normalize(result) * radius;
	}
	else 
	{
		const float optVelProject = lines[lineNo].getDir() * (optVelocity - lines[lineNo].getPoint());

		// Optimize closest point

		if (optVelProject < crossPointLeft) 
		{
			result = lines[lineNo].getPoint() + crossPointLeft * lines[lineNo].getDir();
		}
		else if (optVelProject > crossPointRight) 
		{
			result = lines[lineNo].getPoint() + crossPointRight * lines[lineNo].getDir();
		}
		else 
		{
			result = lines[lineNo].getPoint() + optVelProject * lines[lineNo].getDir();
		}
	}

	return true;
}

size_t linearProgram2(const std::vector<Line> &lines, float radius, const Vector2D &optVelocity, bool directionOpt, Vector2D &result)
{
	if (directionOpt) 
	{
		result = optVelocity * radius;
	}
	else if (absSq(optVelocity) > sqr(radius)) 
	{
		result = normalize(optVelocity) * radius;
	}
	else 
	{
		result = optVelocity;
	}

	for (size_t i = 0; i < lines.size(); ++i) 
	{
		if (!isLeftOf(lines[i].getDir(), result - lines[i].getPoint())) 
		{
			// Result velocity is not in the allowable region

			if (!linearProgram1(lines, i, radius, optVelocity, directionOpt, result))  return i;
		}
	}

	return lines.size();
}

void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine, float radius, Vector2D &result)
{
	float distance = 0.0f;

	std::vector<Line> obstacleLines(lines.begin(), lines.begin() + static_cast<ptrdiff_t>(numObstLines));

	for (size_t i = beginLine; i < lines.size(); ++i) 
	{
		if (prVectMul(lines[i].getDir(), lines[i].getPoint() - result) > distance) 
		{
			// Result velocity is not in the allowable region

			obstacleLines.erase(obstacleLines.begin() + static_cast<ptrdiff_t>(numObstLines), obstacleLines.end());

			for (size_t j = numObstLines; j < i; ++j) 
			{
				Line line;

				const float denominator = prVectMul(lines[i].getDir(), lines[j].getDir());

				if (std::fabs(denominator) <= EPSILON) 
				{
					// Line i and line j are parallel

					if (lines[i].getDir() * lines[j].getDir() > 0.0f) 
					{
						// Line i and line j have the same direction
						
						continue;
					}
					else 
					{
						// Line i and line j have opposite direction
						
						line.setPoint(0.5f * (lines[i].getPoint() + lines[j].getPoint()));
					}
				}
				else 
				{
					const float numerator = prVectMul(lines[j].getDir(), lines[i].getPoint() - lines[j].getPoint());
					line.setPoint(lines[i].getPoint() + (numerator / denominator) * lines[i].getDir());
				}

				line.setDir(normalize(lines[j].getDir() - lines[i].getDir()));
				obstacleLines.push_back(line);
			}

			linearProgram2(obstacleLines, radius, getNormal(lines[i].getDir()), true, result);

			distance = prVectMul(lines[i].getDir(), lines[i].getPoint() - result);
		}
	}
}