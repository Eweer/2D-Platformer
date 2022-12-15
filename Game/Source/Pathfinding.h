#ifndef __PATHFINDING_H__
#define __PATHFINDING_H__

#include "Module.h"

#include "Point.h"
#include <memory>
#include <vector>

struct SearchNode
{
	iPoint position;
	// Cost to reach node
	int g;
	// Estimated cost to reach the goal
	int h;

	// Pointer to parent node
	std::shared_ptr<SearchNode> parent;

	std::vector<iPoint> GetAdjacentNodes(std::shared_ptr<SearchNode> searchNode) const;

	std::strong_ordering operator<=>(const SearchNode &right) const
	{
		return (g + h) <=> (right.g + right.h);
	}

};

class Pathfinding : public Module
{
public:

	bool CreateWalkabilityMap;

	int width;
	int height;
	std::vector<bool>* walkability;
};

#endif //__PATHFINDING_H_