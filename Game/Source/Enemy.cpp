#include "Enemy.h"
#include "App.h"

#include "PugiXml/src/pugixml.hpp"
#include <string>

Enemy::Enemy()
{
	name = "enemy";
}

Enemy::Enemy(pugi::xml_node const &itemNode = pugi::xml_node(), int newId = 0) : Character(itemNode, newId)
{
	name = "enemy";
}

Enemy::~Enemy() = default;

bool Enemy::Awake()
{
	startingPosition = {
		parameters.find_child_by_attribute("spawn", "id", std::to_string(id).c_str()).attribute("x").as_int(),
		parameters.find_child_by_attribute("spawn", "id", std::to_string(id).c_str()).attribute("y").as_int()
	};

	return true;
}
