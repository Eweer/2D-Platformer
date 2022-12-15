#include "Player.h"
#include "App.h"

#include "Input.h"
#include "Render.h"
#include "Map.h"
#include "Projectile.h"
#include "Log.h"

#include <string>
#include <unordered_map>
#include <regex>
#include <list>

Player::Player()
{
	name = "player";
}

Player::Player(pugi::xml_node const &itemNode = pugi::xml_node(), int newId) : Character(itemNode, newId)
{ 
	name = "player";
}

Player::~Player() = default;

bool Player::LoadProjectileData()
{
	for(auto const &elem : parameters.children("projectile"))
	{
		std::vector<b2Vec2> tempData;
		std::string shapeType = elem.attribute("shape").as_string();

		if(StrEquals(shapeType, "chain") || StrEquals(shapeType, "polygon"))
		{
			const std::string xyStr = elem.attribute("points").as_string();
			static const std::regex r(R"((-?\d{1,3})(?:\.\d+)*,(-?\d{1,3})(?:\.\d+)*)");
			auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
			auto xyStrEnd = std::sregex_iterator();

			for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
			{
				std::smatch match = *i;
				tempData.push_back(
					{
						PIXEL_TO_METERS(stoi(match[1].str())),
						PIXEL_TO_METERS(stoi(match[2].str()))
					}
				);
			}
		}
		else if(StrEquals(shapeType, "rectangle"))
		{
			tempData.push_back(
				{
					PIXEL_TO_METERS(elem.attribute("width").as_int()),
					PIXEL_TO_METERS(elem.attribute("height").as_int())
				}
			);
		}
		else if(StrEquals(shapeType, "circle"))
		{
			tempData.push_back(
				{
					elem.attribute("radius").as_float(),
					0
				}
			);
		}

		using enum CL::ColliderLayers;
		CL::ColliderLayers maskAux = (ENEMIES | PLATFORMS);
		ProjectileData projAux(
			shapeType,
			tempData,
			iPoint(
				elem.attribute("x").as_int(),
				elem.attribute("y").as_int()
			),
			iPoint(
				elem.attribute("width").as_int(),
				elem.attribute("height").as_int()
			),
			maskAux,
			elem.attribute("speed").as_int()
		);

		std::string pName = elem.attribute("name").as_string();
		projectileMap[pName] = projAux;
	}
	return true;
}

bool Player::Awake()
{
	jump = {
		.bOnAir = true,
		.currentJumps = 0,
		.maxJumps = parameters.attribute("maxjumps").as_int(),
		.jumpImpulse = parameters.attribute("jumpimpulse").as_float(),
	};

	startingPosition = {
		parameters.attribute("x").as_int(),
		parameters.attribute("y").as_int()
	};

	LoadProjectileData();

	return true;
}

std::string Player::ChooseAnim()
{
	if(bAttack1 || bAttack2 || bHurt || bDead || bFalling >= 100)
		bLockAnim = true;

	if(bDead) return "dead";
	if(bHurt) return "hurt";
	if(bFalling >= 100) return "falling";
	if(bFalling >= 10) return "jump";
	if(bClimbing) return "climb";
	if(bPushing) return "push";
	if(bNormalJump) return "jump";
	if(bHighJump) return "high_Jump";
	if(bRunning)
	{
		if(bAttack1) return "run_Attack";
		return "run";
	}
	if(bWalking)
	{
		if(bAttack1) return "walk_Attack";
		return "walk";
	}
	if(bAttack2) return "attack_Extra";
	if(bAttack1) return "attack";
	if(bIdle) return "idle";
	return "unknown";
}

bool Player::StopProjectiles()
{
	// Update Projectiles
	for(auto it = projectiles.begin(); it < projectiles.end(); ++it)
	{
		if(!it->get()) continue;
		if(it->get()->Update()) continue;
		projectiles.erase(it);
	}
	return true;
}

bool Player::Update()
{
	// If landing
	if(bKeepMomentum)
	{
		pBody->body->SetLinearVelocity(b2Vec2(velocityToKeep.x, 0));
		bKeepMomentum = false;
	}
	
	// Set movement variables
	b2Vec2 vel = pBody->body->GetLinearVelocity();
	b2Vec2 impulse = {0, 0};
	float maxVel = app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT ? 7.5f : 5.0f;
	bool moveCamera = false;

	
	// If it's able to move
	if(bAbleToMove)
	{
		// Move player if pressing A/D
		if(app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			if(vel.y != 0) impulse.x = b2Max(vel.x - 0.15f, maxVel * -1);
			else impulse.x = b2Max(vel.x - 0.25f, maxVel * -1);
			moveCamera = true;
			dir = 1;
		}

		if(app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			if(vel.y != 0) impulse.x = b2Min(vel.x + 0.15f, maxVel);
			else impulse.x = b2Min(vel.x + 0.25f, maxVel);
			moveCamera = true;
			dir = 0;
		}
		if(impulse.x == 0) impulse.x = vel.x * 0.98f;
	}


	// If it's playing an animation lock
	if(bLockAnim)
	{
		// If it's attacking, wait for fifth texture before creating the projectile
		if(bAttackQueue && texture->GetCurrentFrame() >= 5)
		{
			using enum CL::ColliderLayers;
			bAttackQueue = false;
			std::unique_ptr<Projectile>projPtr;
			if(bAttack1)
			{
				bAttack1 = false;
				projPtr = std::make_unique<Projectile>(
					texture->GetAnim("fire"),
					iPoint(position.x + 30, position.y),
					projectileMap["fire"],
					attackDir
				);
			}
			else if(bAttack2)
			{
				bAttack2 = false;
				projPtr = std::make_unique<Projectile>(
					texture->GetAnim("fire_Extra"),
					iPoint(position.x + 30, position.y),
					projectileMap["fire_Extra"],
					attackDir
				);
			}
			projectiles.push_back(std::move(projPtr));
		}
		if(bHighJump) bHighJump = false;
		if(bHurt) bHurt = false;
		if(bDead) bDead = false;

		// If the texture finished playing, we give the control back to the player
		if(texture->GetAnimFinished())
		{
			bLockAnim = false;
			bAbleToMove = true;
		}
	}
	else // If it's not animation locked
	{
		// Check if player is able and wants to jump
		if(jump.currentJumps < jump.maxJumps && app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		{
			// Check if it is currently jumping, if it is restart the animation
			if(bNormalJump || bHighJump)
			{
				texture->SetCurrentFrame(0);
				bLockAnim = true;
			}

			if(app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
			{
				impulse.y = jump.jumpImpulse * -1.5f;
				impulse.x /= 1.5f;
				bHighJump = true;
			}
			else
			{
				impulse.y = jump.jumpImpulse * -1.0f;
				bNormalJump = true;
			}

			jump.bOnAir = true;
			jump.currentJumps++;

			pBody->body->SetLinearVelocity(b2Vec2(vel.x, 0));
			pBody->body->ApplyLinearImpulse(b2Vec2(0, impulse.y), pBody->body->GetWorldCenter(), true);
		}

		// Left click attack, only able to do it if on the floor
		if(!jump.bOnAir && app->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			// If it's idle, player can't move during the lock.
			if(impulse.x == 0) bAbleToMove = false;
			iPoint currentMousePos = {
				app->input->GetMousePosition().x - app->render->GetCamera().x,
				app->input->GetMousePosition().y - app->render->GetCamera().y
			};
			attackDir = PIXEL_TO_METERS(currentMousePos - position);
			bAttack1 = true;
			bAttackQueue = true;
			if(pBody->body->GetPosition().x > PIXEL_TO_METERS(currentMousePos.x))
				dir = 1;
			else
				dir = 0;
		}

		// Right click attack, only able to do it if on the floor
		if(!jump.bOnAir && app->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
		{
			// Stop all momentum, player can't move during lock
			impulse.x = 0;
			bAbleToMove = false;
			iPoint currentMousePos = {
				app->input->GetMousePosition().x - app->render->GetCamera().x,
				app->input->GetMousePosition().y - app->render->GetCamera().y
			};
			attackDir = PIXEL_TO_METERS(currentMousePos - position);
			bAttack2 = true;
			bAttackQueue = true;
			if(pBody->body->GetPosition().x > PIXEL_TO_METERS(currentMousePos.x))
				dir = 1;
			else
				dir = 0;
		}		
	}

	// Set player speed
	pBody->body->SetLinearVelocity(b2Vec2(impulse.x, pBody->body->GetLinearVelocity().y));

	// Set booleans based on current movement
	if(pBody->body->GetLinearVelocity().y <= 0)	bFalling = 0;
	else if(pBody->body->GetLinearVelocity().y > 1.0f) bFalling++;

	if(pBody->body->GetLinearVelocity().x == 0)
	{
		bWalking = false;
		bRunning = false;
		bIdle = true;
	}
	else
	{
		bIdle = false;
		bWalking = true;
		if(app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
			bRunning = true;
		else
			bRunning = false;
	}

	// If it's not locked, we set the texture based on priority
	if(!bLockAnim) texture->SetCurrentAnimation(ChooseAnim());

	// Move camera
	app->render->AdjustCamera(position);

	// Set image position and draw character
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	return true;
}

void Player::BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	switch (pBodyB->ctype)
	{
		using enum CL::ColliderLayers;
		case ITEMS:
			LOG("Collision ITEMS");
			break;
		case PLATFORMS:
		{
			LOG("Collision PLATFORMS");
			if(pBody->ground->ptr == fixtureA)
			{
				if(bNormalJump)
				{
					bLockAnim = false;
				}
				bNormalJump = false;
				bHighJump = false;
				if(jump.bOnAir)
				{
					bKeepMomentum = true;
					velocityToKeep = pBody->body->GetLinearVelocity();
				}
				jump = {
					.bOnAir = false,
					.currentJumps = 0,
					.maxJumps = jump.maxJumps,
					.jumpImpulse = jump.jumpImpulse,
				};
			}
 			
			break;
		}
		case PLAYER:
		{
			break;
		}
		case ENEMIES:
		{
			break;
		}
		case TRIGGERS:
		{
			break;
		}
		case CHECKPOINTS:
		{
			break;
		}
		case UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
	}
}
