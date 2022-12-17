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
	else if(path && !path->empty())
	{
		b2Vec2 vel = pBody->body->GetLinearVelocity();
		auto currentCoords = app->map->WorldToCoordinates(position);

		// and the currentCoords are the same as the ones on the path[currentPathIndex]
		// and we are not falling nor jumping
		if(currentCoords == path->at(currentPathIndex) && vel.y == 0)
		{
			// We request a new path in case destination has moved
			bRequestPath = true;
			// We update our index unless it's the last tile, as that is our destination
			if(currentPathIndex < path->size() - 1) currentPathIndex++;
		}

		// Set direction and animation
		// This function returns the velocity as float32
		vel.x = SetPathMovementParameters(currentCoords);

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

bool Enemy::SetPath(iPoint destinationCoords)
{
	bRequestPath = false;

	// Get the coordinates of origin and destination
	auto positionTile = app->map->WorldToCoordinates(position);

	auto pathPtr = app->pathfinding->AStarSearch(positionTile, destinationCoords);

	// If it's nullptr we don't update the path
	if(!pathPtr.get()) return false;
	
	// If the new path is valid and not empty, it's the new path
	path = std::move(pathPtr);
	currentPathIndex = path->size() > 1 ? 1 : 0;
	return true;
}

void Enemy::DrawDebugPath() const
{
	iPoint origin = path->at(currentPathIndex);
	origin = app->map->MapToWorld(origin.x, origin.y);
	origin.x += app->map->GetTileWidth()/2;
	origin.y += app->map->GetTileHeight();

	for(int i = currentPathIndex; i < path->size() - 1; i++)
	{
		iPoint destination = path->at(i + 1);
		destination = app->map->MapToWorld(destination.x, destination.y);
		destination.x += app->map->GetTileWidth()/2;
		destination.y += app->map->GetTileHeight();
		app->render->DrawLine(origin, destination, SDL_Color(255, 0, 0, 255));
		origin = destination;
	}
}

void Enemy::DrawDebug() const
{
	if(path && currentPathIndex < path->size() - 1) DrawDebugPath();
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

float Enemy::SetPathMovementParameters(iPoint currentCoords)
{
	float vel = 2.0f;
	float sign = 0;

	if(currentCoords.y == path->at(currentPathIndex).y)
	{
		if(currentCoords.x < path->at(currentPathIndex).x)
			sign = 1;
		else if(currentCoords.x > path->at(currentPathIndex).x)
			sign = -1;
	}
	else
	{
		if(path->at(currentPathIndex - 1).x < path->at(currentPathIndex).x)
			sign = 1;
		if(path->at(currentPathIndex - 1).x > path->at(currentPathIndex).x)
			sign = -1;
	}
	vel *= sign;
	
	if(sign == 0) texture->SetCurrentAnimation("idle");
	else
	{
		dir = (sign == -1) ? 1 : 0;
		texture->SetCurrentAnimation("walk");
	}

	return vel;
}
