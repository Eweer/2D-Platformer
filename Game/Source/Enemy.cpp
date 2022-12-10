#include "Enemy.h"

Enemy::Enemy()
{
	name = "enemy";
}

Enemy::Enemy(pugi::xml_node const &itemNode = pugi::xml_node(), int newId = 0) : Character(itemNode, newId)
{
	name = "enemy";
}

Enemy::~Enemy() = default;
