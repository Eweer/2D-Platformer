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
	// While doing hurt/dead animation, we don't move the character
	if(iFrames > 0)
	{		
		iFrames++;

		// If enemy is dead
		if(hp == 0)
		{
			if(texture->IsLastFrame()) texture->Pause();
			if(iFrames >= 100)
			{
				iFrames = 0;
				active = false;
			}
		}
		// If it's not dead and iFrame timer expired
		else if(iFrames >= 20)
		{
			iFrames = 0;
		}
	}
	// If there's a valid path and we haven't finished it, we have to move
	else if(!path.empty() && currentPathIndex < path.size() - 1)
	{
		b2Vec2 vel = pBody->body->GetLinearVelocity();
		auto currentCoords = app->map->WorldToCoordinates(position);

		// If we got to the tile, we need to go to the next one
		if(currentCoords == path[currentPathIndex]) currentPathIndex++;

		// Set speed and direction depending on quadrant
		if(currentCoords.x > path[currentPathIndex].x)
		{
			vel.x = -2.0f;
			dir = 1;
			texture->SetCurrentAnimation("walk");
		}
		else if(currentCoords.x < path[currentPathIndex].x)
		{
			vel.x = 2.0f;
			dir = 0;
			texture->SetCurrentAnimation("walk");
		}
		else
		{
			vel.x = 0;
			texture->SetCurrentAnimation("idle");
		}

		pBody->body->SetLinearVelocity(vel);
	}
	else texture->SetCurrentAnimation("idle");

	//Update Character position in pixels
	if(pBody)
	{
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

	// If the new path is valid and not empty, it's the new path
	if(std::vector<iPoint> retPath = app->pathfinding->AStarSearch(positionTile, destinationTile);
	   !retPath.empty())
	{
		path = retPath;
		return true;
	}

	return false;
}

void Enemy::OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	using enum CL::ColliderLayers;

	if(iFrames == 0 // iFrames are not enabled
	   && ((pBodyB->ctype & BULLET) == BULLET) // got hit by a bullet
	   && ((pBodyB->pListener->source & PLAYER) == PLAYER)) // the source of the bullet was the player
	{
		hp -= 1;
		iFrames = 1;
		if(hp <= 0)
		{
			texture->SetCurrentAnimation("death");

			// We stop all X momentum
			pBody->body->SetLinearVelocity(b2Vec2(0, pBody->body->GetLinearVelocity().y));

			Disable();
			active = true;
		}
		else texture->SetCurrentAnimation("hurt");
	}
}
