#include "Enemy.h"
#include "App.h"
#include "Projectile.h"
#include "BitMaskColliderLayers.h"
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

void Enemy::OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	using enum CL::ColliderLayers;
	if(iFrames == 0)
	{
		if(((pBodyB->ctype & BULLET) == BULLET) && (pBodyB->pListener->source == PLAYER))
		{
			hp -= 1;
			iFrames = 1;
			if(hp <= 0) Disable();
		}
	}
	
}
