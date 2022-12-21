#include "Pathfinding.h"
#include "App.h"

#include "Map.h"
#include "Render.h"

#include "Log.h"

#include "BitMaskNavType.h"

#include <numeric>
#include <utility>
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
	
	using enum CL::NavType;
	NavPoint currentNavPoint = app->pathfinding->GetNavPoint(searchNode.get()->position);
	CL::NavType currentType = currentNavPoint.type;
	
	if(IsWalkable(currentPosition.Right()) && (currentType == LEFT || currentType == PLATFORM))
		list.emplace_back(currentPosition.Right(), 10, NavLinkType::WALK);

	if(IsWalkable(currentPosition.Left()) && (currentType == RIGHT || currentType == PLATFORM))
		list.emplace_back(currentPosition.Left(), 10, NavLinkType::WALK);

	for(auto const &elem : currentNavPoint.links)
	{
		list.emplace_back(elem);
	}

	return list;
}

std::vector<NavLink> SearchNode::GetAdjacentAirNodes(std::shared_ptr<SearchNode> searchNode, iPoint destination) const
{
	std::vector<NavLink> list;
	// Check all directions, including diagonals
	iPoint pos = searchNode.get()->position;
	for(int x = pos.x - 1; x <= pos.x + 1; x++)
	{
		for(int y = pos.y - 1; y <= pos.y + 1; y++)
		{
			// If it's the tile we are at or if it's out of bounds
			// we continue the loop
			if(!app->pathfinding->IsValidPosition(iPoint(x, y))
			   || (x == pos.x && y == pos.y))
				continue;

			if(app->pathfinding->GetNavPoint(iPoint(x, y)).type != CL::NavType::NONE
			   && destination != iPoint(x, y)) continue;

			// Diagonal cost is 14 (if x == (1 or -1) and y == (1 or -1))
			// Can also be checked as (x && y), and another option would be ((x & y) == 1)
			int score = abs(x) == abs(y) ? 14 : 10;
			list.emplace_back(iPoint(x, y), score, NavLinkType::WALK);
		}
	}
	return list;
}

bool SearchNode::IsWalkable(iPoint p) const
{
	return app->pathfinding->IsValidPosition(p)
		&& app->pathfinding->GetNavPoint(p).type != CL::NavType::NONE;
}

// ---------- PathFinding ---------
bool Pathfinding::SetWalkabilityMap()
{
	auto mapPtr = app->map->CreateWalkabilityMap();

	// Overwrite if there is already a groundMap
	if(groundMap) groundMap.reset(mapPtr.release());
	// Otherwise create a new groundMap
	else groundMap = std::move(mapPtr);

	if(!groundMap) return false;

	CreateWalkabilityLinks();

	return true;
}

NavPoint &Pathfinding::GetNavPoint(iPoint position) const
{
	return groundMap->at(position.x)[position.y];
}

bool Pathfinding::IsValidPosition(iPoint position) const
{
	return (position.x >= 0 && position.x < groundMap->size() &&
			position.y >= 0 && position.y < groundMap->at(0).size());

}

int Pathfinding::HeuristicCost(iPoint origin, iPoint destination) const
{
	return origin.DistanceManhattan(destination);
}

std::unique_ptr<std::vector<iPoint>> Pathfinding::AStarSearch(iPoint origin, iPoint destination, PathfindTerrain pTerrain) const
{
	// Check if path is valid. If it isn't log it and return an empty path
	if(!IsValidPosition(origin) || !IsValidPosition(destination))
	{
		//LOG("Path from %s, %s to %s, %s is not valid", origin.x, origin.y, destination.x, destination.y);
		return nullptr;
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
			auto path = std::make_unique<std::vector<iPoint>>();
			auto node = std::make_shared<SearchNode>(currentNode);
			while(node)
			{
				path->emplace_back(node->position);
				node = node->parent;
			}
			// Reverse the path, as it's ordered from destination to origin
			std::ranges::reverse(path->begin(), path->end());

			// Log information and return the path
			LOG("Created path of %d steps in %d iterations", path->size(), iterations);
			return path;
		}

		// As we are visiting the node, we add it to the closed list to skip it later
		closedList.emplace_back(currentNode.position);
		
		// Get nodes that are adjacent or linked to the current node
		std::vector<NavLink> adjacentNodes;
		if(pTerrain == PathfindTerrain::GROUND)
		{
			adjacentNodes = currentNode.GetAdjacentGroundNodes(
				std::make_shared<SearchNode>(currentNode)
			);
		}
		else
		{
			adjacentNodes = currentNode.GetAdjacentAirNodes(
				std::make_shared<SearchNode>(currentNode), destination
			);
		}

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
				HeuristicCost(node.destination, destination) * 10,
				std::make_shared<SearchNode>(currentNode)
			);
		}
		++iterations;
	}

	// If we ended last while, it means there is no path available.
	// We return an empty vector
	return nullptr;
}

iPoint Pathfinding::GetDestinationCoordinates(iPoint position, PathfindTerrain pTerrain) const
{
	position = app->map->WorldToCoordinates(position);
	if(!IsValidPosition(position))
	{
		position.x = (position.x < 0) ? 2 : groundMap->size() - 2;
		position.y = (position.y < 0) ? 2 : groundMap->at(0).size() - 2;
	}

	// If it's a ground enemy, we pathfind to the node
	// that is under the position, even if he is on the air
	if(pTerrain == PathfindTerrain::GROUND)
		return GetTerrainUnder(position);
	
	return position;
}

iPoint Pathfinding::GetPatrolCoordinates(iPoint position, int dir, PathfindTerrain pTerrain, int patrolRadius) const
{
	using enum CL::NavType;
	position = app->map->WorldToCoordinates(position);

	if(!IsValidPosition(position)) return position;
	
	auto type = groundMap->at(position.x)[position.y].type & (LEFT | PLATFORM | RIGHT);
	auto check = NONE;
	if(pTerrain == PathfindTerrain::GROUND)
	{
  		check = (type & PLATFORM) != PLATFORM
			? (TERRAIN | LEFT | RIGHT | PLATFORM) ^ type
			: (TERRAIN | LEFT | RIGHT) ^ type;
	}
	iPoint left(position);
	iPoint right(position);

	CL::NavType leftCheck = (pTerrain == PathfindTerrain::GROUND) ? (RIGHT | PLATFORM) : NONE;
	CL::NavType rightCheck = (pTerrain == PathfindTerrain::GROUND) ? (LEFT | PLATFORM) : NONE;

	if((type | leftCheck) == leftCheck)
		left = GetPatrolMaxX(position, check, patrolRadius * -1);
	if((type | rightCheck) == rightCheck)
		right = GetPatrolMaxX(position, check, patrolRadius);

	// If there is a path in the other direction, take it
	if(position.x - right.x != 0 && dir == 1)
		return right;

	// If there wasn't, because either direction was wrong or right.x == position.x
	if(position.x - left.x != 0)
		return left;
	
	// If position.x is the same as left.x return right
	// Right can be either currentPos or a differentPos
	return right;
}

iPoint Pathfinding::GetPatrolMaxX(iPoint position, CL::NavType check, int patrolRadius) const
{
	int sign = patrolRadius > 0 ? 1 : -1;
	for(int i = sign; abs(i) < abs(patrolRadius); i += sign)
	{
		// Check if position is valid and 
		// if the tile at position is an ending one
		if(std::cmp_less(position.x + i, groundMap->size()) && position.x + i >= 0 &&
		   (groundMap->at(position.x + i)[position.y].type | check) != check)
			return iPoint(position.x + i - sign, position.y);
	}
	return iPoint(position.x + patrolRadius, position.y);
}

iPoint Pathfinding::GetTerrainUnder(iPoint position) const
{
	for(int y = position.y; y < groundMap->at(position.x).size(); y++)
		if(GetNavPoint({position.x, y}).type !=  CL::NavType::NONE)
			return {position.x, y};

	return position;
}

bool Pathfinding::PostUpdate()
{
	if(app->physics->IsDebugActive()) DrawNodeDebug();
	return true;
}

void Pathfinding::DrawNodeDebug() const
{
	for(int i = 0; i < groundMap->size(); i++)
	{
		for(int j = 0; j < groundMap->at(0).size(); j++)
		{
			using enum CL::NavType;
			auto const &tile = groundMap->at(i).at(j);
			if(tile.type == NONE || tile.type == TERRAIN)
				continue;

			SDL_Color rgba = {0, 0, 0, 255};
			if(tile.type == LEFT) { rgba.r = 255; rgba.g = 218; }
			else if(tile.type == RIGHT) { rgba.r = 255; rgba.g = 143; }
			else if(tile.type == PLATFORM) rgba.b = 255;
			else if(tile.type == SOLO) rgba.g = 255;

			iPoint pos = app->map->MapToWorld(i, j);
			pos.x += app->map->GetTileWidth()/2;
			pos.y += app->map->GetTileHeight();
			app->render->DrawCircle(pos, 10, rgba);

			for(auto const &elem : tile.links)
			{
				using enum NavLinkType;
				iPoint elemPos = app->map->MapToWorld(elem.destination.x, elem.destination.y);
				elemPos.x += app->map->GetTileWidth()/2;
				elemPos.y += app->map->GetTileHeight();
				rgba = {0, 0, 0, 255};
				if(elem.movement == WALK) { rgba.r = 122; rgba.b = 122; }
				if(elem.movement == FALL) { rgba.b = 122; rgba.g = 122; }
				if(elem.movement == JUMP) { rgba.g = 122; rgba.r = 122; }
				app->render->DrawLine(pos, elemPos, rgba);
			}
		}
	}
}

bool Pathfinding::CreateWalkabilityLinks()
{
	for(int x = 0; x < app->map->GetWidth(); x++)
	{
		for(int y = 0; y < app->map->GetHeight(); y++)
		{
			using enum CL::NavType;
			CL::NavType maskFlag = NONE;
			maskFlag = RIGHT | LEFT | SOLO;
			if((groundMap->at(x).at(y).type & maskFlag) == NONE) continue;

			int leftFrontier = -1;
			int rightFrontier = 1;
			// Tile type is not right, left or solo 
			switch(groundMap->at(x).at(y).type)
			{
				case RIGHT:
					leftFrontier = 1;
					break;
				case LEFT:
					rightFrontier = -1;
					break;
				default:
					break;
			}

			AddFallLinks({x, y}, {leftFrontier, rightFrontier});
			
		}
	}
	return true;
}

void Pathfinding::AddFallLinks(iPoint position, iPoint limit)
{
	for(int xToCheck = position.x + limit.x; xToCheck <= position.x + limit.y; xToCheck++)
	{
		if(xToCheck == position.x) continue;
		bool found = false;
		for(int yToCheck = position.y; !found && yToCheck < app->map->GetHeight(); yToCheck++)
		{
			using enum CL::NavType;
			CL::NavType nodeFlag = RIGHT | PLATFORM | SOLO | LEFT;

			// If it's a terrain tile without a linkable node or the cell is not valid.
			// As it is not walkable, we go to next X as there will be no link in this column
			if(!IsValidPosition({xToCheck, yToCheck}) ||
			   (groundMap->at(xToCheck).at(yToCheck).type & TERRAIN) == TERRAIN) break;

			// If it's not a flag we are looking for we go to next tile in column
			if((groundMap->at(xToCheck).at(yToCheck).type & nodeFlag) == NONE) continue;

			// Create and push the new link
			NavLink tempNav = {
				iPoint(xToCheck, yToCheck),
				HeuristicCost(position, {xToCheck, yToCheck}) * 10,
				NavLinkType::FALL
			};
			groundMap->at(position.x).at(position.y).links.push_back(tempNav);

			found = true;
		}
	}
}

bool Pathfinding::IsRightNode(iPoint position) const
{
	auto coords = app->map->WorldToCoordinates(position);
	coords.y -= 1;
	if(!IsValidPosition(coords)) return false;
	return groundMap->at(coords.x)[coords.y].type == CL::NavType::RIGHT
		|| groundMap->at(coords.x)[coords.y].type == CL::NavType::SOLO;
}

bool Pathfinding::IsLeftNode(iPoint position) const
{
	auto coords = app->map->WorldToCoordinates(position) - 1;
	if(!IsValidPosition(coords)) return false;
	return groundMap->at(coords.x)[coords.y].type == CL::NavType::LEFT
		|| groundMap->at(coords.x)[coords.y].type == CL::NavType::SOLO;
}

bool Pathfinding::IsBorderNode(iPoint position) const
{
	using enum CL::NavType;
	auto coords = app->map->WorldToCoordinates(position);
	if(!IsValidPosition(coords)) return false;
	return groundMap->at(coords.x)[coords.y].type == LEFT
		|| groundMap->at(coords.x)[coords.y].type == SOLO
		|| groundMap->at(coords.x)[coords.y].type == RIGHT;
}