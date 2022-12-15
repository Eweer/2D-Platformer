#include "Pathfinding.h"
#include "App.h"

std::vector<iPoint> SearchNode::GetAdjacentNodes(std::shared_ptr<SearchNode> searchNode) const
{
	std::vector<iPoint> list;
	int width = app->pathfinding->width;
	int height = app->pathfinding->height;

	for(int x = searchNode->position.x; x <= searchNode->position.x; x++)
	{
		for(int y = searchNode->position.y; y <= searchNode->position.y; y++)
		{
			//if(x >= 0 && x < width && y >= 0 && y < height);
		}
	}
	return list;
}
