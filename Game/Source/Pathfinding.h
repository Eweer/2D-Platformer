#ifndef __PATHFINDING_H__
#define __PATHFINDING_H__

#include "Module.h"
#include "Map.h"

#include "Point.h"
#include <memory>
#include <vector>
#include <queue>


enum class PathfindTerrain
{
	NONE = 0x0000,
	GROUND = 0x0001,
	AIR = 0x0002,
	WATER = 0x0004,
	LAVA = 0x0008
};

struct SearchNode
{
	SearchNode() = default;
	SearchNode(iPoint p, int score, int dist, std::shared_ptr<SearchNode>node)
		: position(p), g(score), h(dist), parent(node) {}

	iPoint position;
	// Cost to reach node
	int g;
	// Estimated cost to reach the goal
	int h;

	// Pointer to parent node
	std::shared_ptr<SearchNode> parent;

	std::vector<NavLink> GetAdjacentGroundNodes(std::shared_ptr<SearchNode> searchNode) const;
	std::vector<NavLink> GetAdjacentAirNodes(std::shared_ptr<SearchNode> searchNode, iPoint destination) const;

	bool IsWalkable(iPoint p) const;
	
	bool operator<(const SearchNode &right) const
	{
		return (g + h) > (right.g + right.h);
	}

};

class Pathfinding : public Module
{
public:
	// ------ Algorithms
	std::unique_ptr<std::vector<iPoint>> AStarSearch(iPoint origin, iPoint destination, PathfindTerrain pTerrain = PathfindTerrain::GROUND) const;

	iPoint GetDestinationCoordinates(iPoint position, PathfindTerrain pTerrain) const;

	iPoint GetPatrolCoordinates(iPoint position, int dir, PathfindTerrain pTerrain = PathfindTerrain::GROUND, int patrolRadius = 10) const;

	iPoint GetPatrolMaxX(iPoint position, CL::NavType check, int patrolRadius = 10) const;


	bool PostUpdate() override;
	
	// ------ Utils
	// --- Set maps
	bool SetWalkabilityMap();
	// --- Get information
	bool IsValidPosition(iPoint position) const;
	NavPoint &GetNavPoint(iPoint position) const;

private:
	iPoint GetTerrainUnder(iPoint position) const;
	void DrawNodeDebug() const;
	bool CreateWalkabilityLinks();
	void AddFallLinks(iPoint position, iPoint limit);
	int HeuristicCost(iPoint origin, iPoint destination) const;

	int maxJump = 1;
	int minJump = 1;
	std::unique_ptr<navPointMatrix> groundMap;
};

#endif //__PATHFINDING_H_