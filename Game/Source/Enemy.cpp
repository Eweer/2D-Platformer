#include "Enemy.h"
#include "App.h"
#include "Player.h"
#include "Physics.h"
#include "Point.h"
#include "Map.h"
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
	if(StrEquals(parameters.attribute("type").as_string(), "Air"))
			pTerrain = PathfindTerrain::AIR;

	enemyClass = parameters.attribute("class").as_string();

	aggroRadius = parameters.attribute("aggro").as_int();
	if(aggroRadius == 0) aggroRadius = 8;
	patrolRadius = parameters.attribute("patrol").as_int();
	if(patrolRadius == 0) patrolRadius = 5;

	return true;
}

bool Enemy::Update()
{
	if(attackTimer >= attackCD) attackTimer = 0;
	else if(attackTimer > 0) attackTimer++;
	// While doing hurt/dead animation, we don't move the character
	if(iFrames > 0)
	{		
		iFrames++;

		// If enemy is dead
		if(hp == 0)
		{
			if(pTerrain == PathfindTerrain::AIR && position.y < tileYOnDeath) position.y += 5;

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
	else if(StrEquals(texture->GetCurrentAnimName(), "attack"))
	{
		if(texture->IsAnimFinished()) texture->SetCurrentAnimation("idle");
	}
	// If there's a valid path and we haven't finished it, we have to move
	else if(path && !path->empty())
	{
		auto currentCoords = app->map->WorldToCoordinates(position);
		if(pTerrain == PathfindTerrain::AIR)
		{
			if(currentPathIndex < path->size() - 1
			   && currentCoords == path->at(currentPathIndex))
				currentPathIndex++;
			// As we already got to the last element of path, that is our destination
			else  bRequestPath = true;
		}

		// Set direction and animation
		// This function returns the velocity as b2Vec2
		pBody->body->SetLinearVelocity(SetPathMovementParameters(currentCoords));
	}
	else texture->SetCurrentAnimation("idle");

	//Update Character position in pixels
	if(pBody)
	{
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
	}
	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame().get(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	return true;
}

void Enemy::BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
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
			behaviour = BehaviourState::DEAD;
			// We stop all X momentum
			pBody->body->SetLinearVelocity(b2Vec2(0, pBody->body->GetLinearVelocity().y));
			tileYOnDeath = app->map->GetTileHeight()/2 + app->map->MapToWorld(app->pathfinding->GetDestinationCoordinates(position, PathfindTerrain::GROUND)).y;
			Disable();
			active = true;
		}
		else texture->SetCurrentAnimation("hurt");
	}
	if((pBodyB->ctype & ENEMIES) == ENEMIES)
	{
		if((pBody->body->GetLinearVelocity().x * pBodyB->body->GetLinearVelocity().x > 0)
		   && (pBody->body->GetPosition().x > pBodyB->body->GetPosition().x))
		{
			dir = dir ? 0 : 1;
		}
		bRequestPath = true;
		
	}
	if(((pBodyB->ctype & PLAYER) == PLAYER) && attackTimer == 0)
	{
		if(pBodyB->GetPosition().x > position.x)
			dir = 0;
		else
			dir = 1;

		texture->SetCurrentAnimation("attack");
		attackTimer++;
	}
}

bool Enemy::SetPath(iPoint destinationCoords)
{
	bRequestPath = false;

	// Get the coordinates of origin and destination
	auto positionTile = app->map->WorldToCoordinates(position);

	auto pathPtr = app->pathfinding->AStarSearch(positionTile, destinationCoords, pTerrain);

	// If it's nullptr we don't update the path
	if(!pathPtr.get()) return false;
	
	// If the new path is valid and not empty, it's the new path
	if(path) path.reset(pathPtr.release());
	else path = std::move(pathPtr);
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

b2Vec2 Enemy::SetGroundPathMovement()
{
	float sign = 0;
	auto prevCoords = coordinates;
	coordinates = app->map->WorldToCoordinates(position);
	std::string direction = "none";
	if(path->size() > 1 && path->size() - 1 != currentPathIndex)
	{
		if(prevCoords.x != coordinates.x && !prevCoords.IsZero())
		{
			if(currentPathIndex < path->size() - 1) currentPathIndex++;
			else bRequestPath = true;
		}

		if(currentPathIndex > 0)
			direction = (path->at(currentPathIndex - 1).x < path->at(currentPathIndex).x) ? "right" : "left";
		else 
			direction = (prevCoords.x < path->at(currentPathIndex).x) ? "right" : "left";
	}
	else if(path->size() == 1 || path->size() - 1 == currentPathIndex)
	{
		bool facingPlayer = false;
		for(auto elem = pBody->body->GetContactList(); elem; elem = elem->next)
		{
			if(auto const &otherPBody = static_cast<PhysBody *>(elem->other->GetUserData());
			   otherPBody && ((otherPBody->ctype & CL::ColliderLayers::PLAYER) == CL::ColliderLayers::PLAYER))
			{
				if(!elem->contact->IsTouching()) continue;
				direction = "none";
				dir = (otherPBody->GetPosition().x > position.x) ? 0 : 1;

				if(attackTimer == 0)
				{
					auto const &player = static_cast<Player *>(otherPBody->listener);
					texture->SetCurrentAnimation("attack");
					attackTimer++;
					player->HitByEnemy(this);
				}

				facingPlayer = true;
				break;
			}
		}

		if(!facingPlayer)
		{
			int nextX = position.x + (position.x - METERS_TO_PIXELS(pBody->body->GetTransform().p.x));
			iPoint tileAfterMoving = app->map->WorldToCoordinates(iPoint(nextX, position.y));

			// If current or next coordinate is not a walkable tile we stop the movement and request a new path
			if (!app->pathfinding->IsGroundNode(tileAfterMoving) 
				|| !app->pathfinding->IsGroundNode(coordinates))
			{
				bRequestPath = true;
				lastDirection = "none";
			}
			
			direction = lastDirection;
		}
	}
	if(direction != "none") lastDirection = direction;

	if(attackTimer == 0)
	{
		if(direction == "none") texture->SetCurrentAnimation("idle");
		else
		{
			sign = (direction == "right") ? 1 : -1;
			dir = (sign == -1) ? 1 : 0;
			texture->SetCurrentAnimation("walk");
		}
		return b2Vec2(2.0f * sign, pBody->body->GetLinearVelocity().y);
	}
	else return b2Vec2(0.0f, pBody->body->GetLinearVelocity().y);
}

BehaviourState Enemy::SetBehaviour(iPoint playerPosition, iPoint screenSize)
{
	using enum BehaviourState;
	BehaviourState prevBehaviour = behaviour;
	switch(behaviour)
	{
		case IDLE:
			// If the x distance is less than the width of the screen we activate patrol
			if(abs(playerPosition.x - position.x) <= screenSize.x + 20)
				behaviour = PATROL;
			break;
		case PATROL:
			// If player is in aggro radius we activate aggro behaviour
			if(app->map->WorldToCoordinates(playerPosition).DistanceManhattan(app->map->WorldToCoordinates(position)) <= aggroRadius)
				behaviour = AGGRO;
			break;
		case AGGRO:
		{
			// If the player left the screen, we put the enemy in idle
			if(abs(playerPosition.x - position.x) > screenSize.x + 20)
				behaviour = IDLE;
			break;
		}
		default:
			return DEAD;
	}

	if(behaviour != prevBehaviour)
		bRequestPath = true;

	return behaviour;
}

bool Enemy::HasSaveData() const
{
	return true;
}

bool Enemy::LoadState(pugi::xml_node const &data)
{
	return false;
}

b2Vec2 Enemy::SetAirPathMovement(iPoint currentCoords)
{
	b2Vec2 sign = {0, 0};

	if(currentPathIndex > path->size() - 1) currentPathIndex--;
	if(currentPathIndex < 0) currentPathIndex = 0;

	if(currentCoords.x < path->at(currentPathIndex).x)
		sign.x = 1;
	else if(currentCoords.x > path->at(currentPathIndex).x)
		sign.x = -1;

	if(currentCoords.y < path->at(currentPathIndex).y)
		sign.y = 1;
	else if(currentCoords.y > path->at(currentPathIndex).y)
		sign.y = -1;
		

	if(sign == b2Vec2{0,0})
	{
		texture->SetCurrentAnimation("idle");
	}
	else
	{
		dir = (sign.x == -1) ? 1 : 0;
		texture->SetCurrentAnimation("walk");
	}

	return b2Vec2(2.0f * sign.x, 2.0f * sign.y);
}

b2Vec2 Enemy::SetPathMovementParameters(iPoint currentCoords)
{
	if(pTerrain == PathfindTerrain::GROUND) return SetGroundPathMovement();
	else return SetAirPathMovement(currentCoords);
}

pugi::xml_node Enemy::SaveState(pugi::xml_node const &data)
{
	std::string saveData2 = "<{} {}=\"{}\"/>\n";
	std::string saveOpenData2 = "<{} {}=\"{}\">\n";
	std::string saveData4 = "<{} {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveOpenData4 = "<{} {}=\"{}\" {}=\"{}\">\n";
	std::string saveData6 = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveFloatData = "<{} {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string dataToSave = "<enemy>\n";
	dataToSave += AddSaveData(saveData4, "enemy", "id", id, "class", enemyClass);
	dataToSave += AddSaveData(saveData4, "position", "x", position.x, "y", position.y);
	dataToSave += AddSaveData(saveFloatData, "velocity", "x", pBody->body->GetLinearVelocity().x, "y", pBody->body->GetLinearVelocity().y);
	dataToSave += AddSaveData(saveData4, "entity", "active", active, "disablenextupdate", disableOnNextUpdate);
	dataToSave += AddSaveData(saveData4, "character", "hp", hp, "iframes", iFrames);
	dataToSave += AddSaveData(saveData6, "jump", "onair", jump.bOnAir, "currentjumps", jump.currentJumps, "keepmomentum", bKeepMomentum);
	if(path && !path->empty())
	{
		dataToSave += AddSaveData(saveOpenData2, "pathfind", "behaviour", static_cast<int>(behaviour));
		dataToSave += AddSaveData(saveData4, "destination", "x", path->back().x, "y", path->back().y);
		dataToSave += "</pathfind>";
	}
	else dataToSave += AddSaveData(saveData2, "pathfind", "behaviour", static_cast<int>(behaviour));
	dataToSave += "</enemy>";

	app->AppendFragment(data, dataToSave.c_str());

	return data;
}

void Enemy::SpecificRestart()
{
	behaviour = BehaviourState::IDLE;
	
	path.reset();
	
	currentPathIndex = 0;
	bRequestPath = false;
	pTerrain = PathfindTerrain::GROUND;
	tileYOnDeath = 0;
	bAttack = false;
	bDeath = false;
	bHurt = false;
	bIdle = false;
	bWalk = false;
}
