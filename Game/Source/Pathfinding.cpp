#include "Pathfinding.h"
#include "App.h"

#include "Map.h"

#include "Log.h"

#include <queue>

// ---------- SearchNode ----------
std::vector<NavLink> SearchNode::GetAdjacentGroundNodes(std::shared_ptr<SearchNode> searchNode) const
{
	std::vector<NavLink> list;

	iPoint currentPosition = searchNode.get()->position;
	// If we are not in a valid position we just return
	// TODO: Make this throw an exception.
	// Not done becuase I don't have enough time, so I don't
	// want to risk pathfinding malfunctioning and crashing the game.
	if(!app->pathfinding->IsValidPosition(currentPosition)) return list;
	
	using enum NavType;
	NavPoint currentNavPoint = app->pathfinding->GetNavPoint(searchNode.get()->position);
	NavType currentType = currentNavPoint.type;
	
	if(IsWalkable(currentPosition.Right()) && (currentType == LEFT || currentType == PLATFORM))
		list.emplace_back(currentPosition.Right(), 1);

	if(IsWalkable(currentPosition.Left()) && (currentType == RIGHT || currentType == PLATFORM))
		list.emplace_back(currentPosition.Left(), 1);

	for(auto const &elem : currentNavPoint.links)
	{
		list.emplace_back(elem);
	}

	return list;
}

std::vector<NavLink> SearchNode::GetAdjacentAirNodes(std::shared_ptr<SearchNode> searchNode) const
{
	std::vector<NavLink> list;
	// Check all directions, including diagonals
	iPoint pos = searchNode.get()->position;
	for(int x = pos.x - 1; x < pos.x + 1; x++)
	{
		for(int y = pos.y - 1; y < pos.y + 1; y++)
		{
			// If it's the tile we are at or if it's out of bounds
			// we continue the loop
			if(!app->pathfinding->IsValidPosition(iPoint(x, y))
			   || (x == 0 && y == 0))
				continue;

			if(app->pathfinding->GetNavPoint(iPoint(x, y)).type != NavType::NONE) continue;

			// Diagonal cost is 14 (if x == (1 or -1) and y == (1 or -1))
			// Can also be checked as (x && y), and another option would be ((x & y) == 1)
			list.emplace_back(iPoint(x, y), abs(x) == abs(y) ? 14 : 10);
		}
	}
	return list;
}

bool SearchNode::IsWalkable(iPoint p) const
{
	return app->pathfinding->IsValidPosition(p)
		&& app->pathfinding->GetNavPoint(p).type != NavType::NONE;
}

// ---------- PathFinding ---------
bool Pathfinding::SetWalkabilityMap()
{
	groundMap = app->map->CreateWalkabilityMap();
	if(!groundMap) return false;
	return true;
}

NavPoint &Pathfinding::GetNavPoint(iPoint position) const
{
	return groundMap->at(position.x)[position.y];
}

bool Pathfinding::IsValidPosition(iPoint position) const
{
	return (position.x >= 0 && position.x <= groundMap->size() &&
			position.y >= 0 && position.y <= groundMap->at(0).size());

}

int Pathfinding::HeuristicCost(iPoint origin, iPoint destination) const
{
	return origin.DistanceManhattan(destination);
}

std::vector<iPoint> Pathfinding::AStarSearch(iPoint origin, iPoint destination) const
{
	// Check if path is valid. If it isn't log it and return an empty path
	if(!IsValidPosition(origin) || !IsValidPosition(destination))
	{
		LOG("Path from %s, %s to %s, %s is not valid", origin.x, origin.y, destination.x, destination.y);
		return std::vector<iPoint>();
	}

	// Counter of iterations so we can log it
	int iterations = 0;

	// openList is ordered from smallest F to highest F
	// check overloaded operator in SearchNode
	// It stores the nodes that have not been fully evaluated
	std::priority_queue<SearchNode> openList;

	// The closed list will store those nodes that have been fully explored,
	// so we can skip them later if we find them again
	std::vector<iPoint> closedList;

	openList.emplace(origin, 0, HeuristicCost(origin, destination), nullptr);

	while(!openList.empty())
	{
		SearchNode currentNode = openList.top();
		openList.pop();

		// If we got to the goal
		if(currentNode.position == destination)
		{
			// Build the path
			std::vector<iPoint> path;
			auto node = std::make_shared<SearchNode>(currentNode);
			while(node)
			{
				path.emplace_back(node->position);
				node = node->parent;
			}
			// Reverse the path, as it's ordered from destination to origin
			std::ranges::reverse(path);

			// Log information and return the path
			LOG("Created path of %d steps in %d iterations", path.size(), iterations);
			return path;
		}

		// As we are visiting the node, we add it to the closed list to skip it later
		closedList.emplace_back(currentNode.position);
		
		// Get nodes that are adjacent or linked to the current node
		auto adjacentNodes = currentNode.GetAdjacentGroundNodes(
			std::make_shared<SearchNode>(currentNode)
		);

		// Loop through the adjacent nodes and modify or add them to the open list if required
		for(auto const &node : adjacentNodes)
		{
			// If node is on the closed List, we skip it
			if(std::ranges::find(closedList, node.destination) != closedList.end())
				continue;

			// Add the new score to the current g
			int cost = currentNode.g + node.score;

			// We'll need to know if the node is on the open list later
			bool bNodeInOpenList = false;

			// Check if the node is already on the open list
			std::priority_queue<SearchNode>tempOpen = openList;
			while(!tempOpen.empty())
			{
				SearchNode nodeToCheck = tempOpen.top();
				tempOpen.pop();

				// If this node is not the current node, we keep looping
				if(nodeToCheck.position != node.destination) continue;

				// We found the node, so we true a boolean.
				// This is required as we won't need to add the node to the 
				// openList when we end this loop
				bNodeInOpenList = true;

				// If the cost is lower than the current one, we keep looping
				if(nodeToCheck.g <= cost) continue;

				// If the cost is greater, we need to update the node
			
				// Remove it from the open list
				// aka, rebuild the list without adding the node
				std::priority_queue<SearchNode> newOpenList;
				while(!openList.empty())
				{
					SearchNode aux = openList.top();
					openList.pop();
					if(aux.position != node.destination)
						newOpenList.push(aux);
				}
				openList = newOpenList;

				// Update the g of the node and the parent
				nodeToCheck.g = cost;
				nodeToCheck.parent = std::make_shared<SearchNode>(currentNode);

				// Add the updated node to the list
				openList.push(nodeToCheck);

				// We already found our node, so we don't need to keep checking
				break;
			}

			// If the node was on the open list, we already updated it, so we keep looping
			if(bNodeInOpenList) continue;

			// If we got to this point, the node was not in the open list nor it is the destination,
			// so we create a new node and add it to the open list
			openList.emplace(
				node.destination,
				cost,
				HeuristicCost(node.destination, destination),
				std::make_shared<SearchNode>(currentNode)
			);
		}
		++iterations;
	}

	// If we ended last while, it means there is no path available.
	// We return an empty vector
	return std::vector<iPoint>();
}