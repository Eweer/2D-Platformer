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

bool Enemy::Update()
{
	if(iFrames > 0)
	{
		iFrames++;
		if(hp == 0)
		{
			if(texture->IsLastFrame()) texture->Pause();
			if(iFrames >= 100)
			{
				iFrames = 0;
				active = false;
			}
		}
		else
		{
			if(iFrames >= 20)
			{
				texture->SetCurrentAnimation("idle");
				iFrames = 0;
			}
		}
	}
	else
	{
		//Update Character position in pixels
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
	}

	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);
	return true;
}

bool Enemy::SetPath(iPoint destination)
{
	// Get the coordinates of origin and destination
	auto positionTile = app->map->WorldToCoordinates(position);
	auto destinationTile = app->map->WorldToCoordinates(destination);
	// Set the path
	path = app->pathfinding->AStarSearch(positionTile, destinationTile);
	// If it's empty, no path could be formed
	if(path.empty()) return false;
	return true;
}

void Enemy::OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	using enum CL::ColliderLayers;
	if(iFrames == 0)
	{
		if(((pBodyB->ctype & BULLET) == BULLET) && ((pBodyB->pListener->source & PLAYER) == PLAYER))
		{
			hp -= 1;
			iFrames = 1;
			if(hp <= 0)
			{
				texture->SetCurrentAnimation("death");
				Disable();
				active = true;
			}
			else texture->SetCurrentAnimation("hurt");
		}
	}
}
