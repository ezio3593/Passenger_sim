#pragma once

#include <vector>
#include "ObstacleVertex.h"
#include "Passenger.h"

class KdObstacleTree
{
	class ObstacleTreeNode 
	{
		ObstacleTreeNode *left;
		ObstacleTreeNode *right;

		ObstacleVertex* obstacleVertex;
	public:
		ObstacleTreeNode(): left(NULL), right(NULL), obstacleVertex(NULL) {}
		
		ObstacleTreeNode* getLeftNode() const { return left; }
		ObstacleTreeNode* getRightNode() const { return right; }
		ObstacleVertex* getObstacleVertex() const { return obstacleVertex; } 

		void setLeftNode(ObstacleTreeNode* leftNode) {left = leftNode;}
		void setRightNode(ObstacleTreeNode* rightNode) {right = rightNode;}
		void setObstacleVertex(ObstacleVertex *obstVert) { obstacleVertex = obstVert; }

		~ObstacleTreeNode() {}
	};

	ObstacleTreeNode *obstacleTree;

	void computeObstacleNeighborsRecursive(const Agent *agent, float maxDistSq, const ObstacleTreeNode *node, std::vector<ObstacleVertex*> &neighborsObstVerts) const;
	ObstacleTreeNode *buildObstacleTreeRecursive(const std::vector<ObstacleVertex*> &verts);
public:
	KdObstacleTree(): obstacleTree(NULL) {}

	void buildObstacleTree(const std::vector<ObstacleVertex*> &verts);
	void computeObstacleNeighbors(const Agent *agent, float maxDistSq, std::vector<ObstacleVertex*> &neighborsObstVerts) const;
	void deleteObstacleTree(ObstacleTreeNode *node);

	~KdObstacleTree();
};