#include "stdafx.h"
#include "KdTree.h"

void KdObstacleTree::buildObstacleTree(const std::vector<ObstacleVertex*> &verts)
{
	deleteObstacleTree(obstacleTree);

	obstacleTree = buildObstacleTreeRecursive(verts);
}

KdObstacleTree::ObstacleTreeNode* KdObstacleTree::buildObstacleTreeRecursive(const std::vector<ObstacleVertex *> &verts)
{
	if (verts.empty()) 
	{
		return NULL;
	}
	else 
	{
		ObstacleTreeNode* node = new ObstacleTreeNode();

		size_t optSplitIndex = 0;
		size_t numLeft = 0;
		size_t numRight = 0;

		int diff = verts.size();

		for (size_t i = 0; i < verts.size(); ++i) 
		{
			size_t leftSize = 0;
			size_t rightSize = 0;

			ObstacleVertex* splitVert1 = verts[i];
			ObstacleVertex* splitVert2 = splitVert1->getNextObstVertex();

			// Find optimal split obstacle edge

			for (size_t j = 0; j < verts.size(); ++j) 
			{
				if (i == j) continue;

				ObstacleVertex* vert1 = verts[j];
				ObstacleVertex* vert2 = vert1->getNextObstVertex();

				if (!vert2) continue;

				bool isLeftOfSplitEdge1 = isLeftOf(splitVert1->getPoint(), splitVert2->getPoint(), vert1->getPoint());
				bool isLeftOfSplitEdge2 = isLeftOf(splitVert1->getPoint(), splitVert2->getPoint(), vert2->getPoint());

				if (isLeftOfSplitEdge1 && isLeftOfSplitEdge2) 
				{
					++leftSize;
				}
				else if (!(isLeftOfSplitEdge1 && isLeftOfSplitEdge2)) 
				{
					++rightSize;
				}
				else 
				{
					++leftSize;
					++rightSize;
				}
			}

			int currentDiff = std::abs(int(rightSize - leftSize));
				if (currentDiff == 0 || diff > currentDiff)
				{
					diff = currentDiff;
					numRight = rightSize;
					numLeft = leftSize;
					optSplitIndex = i;
					if (currentDiff == 0)
						break;
				}
		}

		//Split all obstacles' edges 

		std::vector<ObstacleVertex*> leftObstVerts(numLeft);
		std::vector<ObstacleVertex*> rightObstVerts(numRight);

		size_t leftCounter = 0;
		size_t rightCounter = 0;

		ObstacleVertex* splitVert1 = verts[optSplitIndex];
		ObstacleVertex* splitVert2 = splitVert1->getNextObstVertex();

		for (size_t i = 0; i < verts.size(); ++i) 
		{
			if (i == optSplitIndex) continue;

			ObstacleVertex* vert1 = verts[i];
			ObstacleVertex* vert2 = vert1->getNextObstVertex();

			if (!vert2) continue;

			bool leftOfSplitEdge1 = isLeftOf(splitVert1->getPoint(), splitVert2->getPoint(), vert1->getPoint());
			bool leftOfSplitEdge2 = isLeftOf(splitVert1->getPoint(), splitVert2->getPoint(), vert2->getPoint());

			if (leftOfSplitEdge1 && leftOfSplitEdge2) 
			{
				leftObstVerts[leftCounter++] = verts[i];
			}
			else if (!(leftOfSplitEdge1 && leftOfSplitEdge2)) 
			{
				rightObstVerts[rightCounter++] = verts[i];
			}
			else 
			{
				// Split edge

				Vector2D splitPoint;
				getPointOfLineIntersection2(
					Line(splitVert1->getPoint(), splitVert1->getDir()), 
					Line(vert1->getPoint(), vert1->getDir()), splitPoint);

				ObstacleVertex *newObstVert = new ObstacleVertex();
				newObstVert->setPoint(splitPoint);
				newObstVert->setPrevObstVertex(vert1);
				newObstVert->setNextObstVertex(vert2);
				newObstVert->setDirection(vert1->getDir());

				vert1->setNextObstVertex(newObstVert);
				vert2->setPrevObstVertex(newObstVert);

				if (leftOfSplitEdge1) {
					leftObstVerts[leftCounter++] = vert1;
					rightObstVerts[rightCounter++] = newObstVert;
				}
				else {
					rightObstVerts[rightCounter++] = vert1;
					leftObstVerts[leftCounter++] = newObstVert;
				}
			}
		}

		node->setObstacleVertex(splitVert1);
		node->setLeftNode(buildObstacleTreeRecursive(leftObstVerts));
		node->setRightNode(buildObstacleTreeRecursive(rightObstVerts));

		return node;
	}
}

void KdObstacleTree::computeObstacleNeighbors(const Agent *agent, float maxDistSq, std::vector<ObstacleVertex*> &neighborsObstVerts) const
{
	computeObstacleNeighborsRecursive(agent, maxDistSq, obstacleTree, neighborsObstVerts);
}

void KdObstacleTree::computeObstacleNeighborsRecursive(const Agent *agent, float maxDistSq, const ObstacleTreeNode *node, std::vector<ObstacleVertex*> &neighborsObstVerts) const
{
	if (node == NULL) 
	{
		return;
	}
	else 
	{
		ObstacleVertex* vert1 = node->getObstacleVertex();
		ObstacleVertex* vert2 = vert1->getNextObstVertex();

		float signAgentDistToLine = 0;
		bool isAgentLeftOfEdge = isLeftOf(vert1->getPoint(), vert2->getPoint(), agent->getPosition(), signAgentDistToLine);

		computeObstacleNeighborsRecursive(agent, maxDistSq, (isAgentLeftOfEdge ? node->getLeftNode() : node->getRightNode()), neighborsObstVerts);

		float distSqLine = 0;
		
		distSqLine = sqr(isAgentLeftOfEdge) / absSq(vert1->getDir());
		
		if (!isPointProjInSegment(vert1->getPoint(), vert2->getPoint(), vert1->getDir(), agent->getPosition(), distSqLine))
		{
			distSqLine = std::min(absSq(agent->getPosition() - vert1->getPoint()),
				absSq(agent->getPosition() - vert2->getPoint()));
		}

		if (distSqLine < maxDistSq) 
		{
			if (!isAgentLeftOfEdge && isLeftOf(vert1->getDir(), agent->getVelocity()))
			{
				// Add obstacle only if agent is on right side
				neighborsObstVerts.push_back(node->getObstacleVertex());
			}
			
			computeObstacleNeighborsRecursive(agent, maxDistSq, (isAgentLeftOfEdge ? node->getRightNode() : node->getLeftNode()), neighborsObstVerts);
		}
	}
}

void KdObstacleTree::deleteObstacleTree(ObstacleTreeNode *node)
{
	if (node != NULL) {
		deleteObstacleTree(node->getLeftNode());
		deleteObstacleTree(node->getRightNode());
		delete node;
	}
}

KdObstacleTree::~KdObstacleTree()
{
	deleteObstacleTree(obstacleTree);
}