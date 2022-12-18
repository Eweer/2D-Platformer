#include "Player.h"
#include "App.h"

#include "Input.h"
#include "Render.h"
#include "Map.h"
#include "Projectile.h"
#include "Log.h"
#include "Physics.h"
#include "Scene.h"

#include "Defs.h"

#include "Box2D/Box2D/Box2D.h"
#include <unordered_map>
#include <regex>
#include <list>

#include <format>
#include <string>
#include <string_view>


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
		std::string pName = elem.attribute("name").as_string();
		projectileMap[pName] = std::make_unique<ProjectileData>(
			ShapeData(shapeType, tempData),
			elem.attribute("gothrough").as_bool(),
			iPoint(
				elem.attribute("x").as_int(),
				elem.attribute("y").as_int()
			),
			iPoint(
				elem.attribute("width").as_int(),
				elem.attribute("height").as_int()
			),
			elem.attribute("speed").as_int(),
			static_cast<ProjectileFreedom>(elem.attribute("freedom").as_uint())
		);
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

	skillCD = parameters.attribute("skillcd").as_int() * 60;

	LoadProjectileData();

	return true;
}

std::string Player::ChooseAnim()
{
	if(bAttack1 || bAttack2 || bHurt || bDead || bFalling >= 100)
		bLockAnim = true;

	if(bDead) return "death";
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

bool Player::UpdateProjectiles()
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
	if(skillCDTimer >= skillCD) skillCDTimer = 0;
	if(skillCDTimer > 0) skillCDTimer++;

	if(iFrames > 0)
	{
		iFrames++;

		// If player is dead
		if(hp == 0)
		{
			if(texture->IsLastFrame()) texture->Pause();
			if(iFrames >= 100)
			{
				iFrames = 0;
				active = false;
				Start();
				active = true;
				bDead = false;
				bAbleToMove = true;
				bLockAnim = false;
				hp = 3;
			}
		}
		// If it's not dead and iFrame timer expired
		else if(iFrames >= 40)
		{
			iFrames = 0;
			bHurt = false;
		}
		else if(iFrames >= 20)
			bAbleToMove = true;
	}

	// If landing
	if(bKeepMomentum)
	{
		pBody->body->SetLinearVelocity(b2Vec2(velocityToKeep.x, 0));
		bKeepMomentum = false;
	}
	
	b2Vec2 vel;
	b2Vec2 impulse;
	float maxVel;
	if(pBody)
	{
		// Set movement variables
		vel = pBody->body->GetLinearVelocity();
		impulse = {0, 0};
		maxVel = app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT ? 7.5f : 5.0f;
	}

	// If it's able to move
	if(bAbleToMove)
	{
		// Move player if pressing A/D
		if(app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			if(vel.y != 0) impulse.x = b2Max(vel.x - 0.15f, maxVel * -1);
			else impulse.x = b2Max(vel.x - 0.25f, maxVel * -1);
			dir = 1;
		}

		if(app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			if(vel.y != 0) impulse.x = b2Min(vel.x + 0.15f, maxVel);
			else impulse.x = b2Min(vel.x + 0.25f, maxVel);
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
				skillCDTimer++;
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

		// If the texture finished playing, we give the control back to the player
		if(texture->GetAnimFinished())
		{
			bLockAnim = false;
			bAbleToMove = true;
		}
	}
	// Jump / Attacks
	else 
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
		if(skillCDTimer == 0 && !jump.bOnAir && app->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
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

	if(pBody)
	{
		// Set player speed
		pBody->body->SetLinearVelocity(b2Vec2(impulse.x, pBody->body->GetLinearVelocity().y));
	

		// Set boolean actions based on current movement
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
		
		app->scene->IncreaseBGScrollSpeed(impulse.x);
		// Set image position and draw character
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
		
	}

	// If it's not locked, we set the texture based on priority
	if(!bLockAnim) texture->SetCurrentAnimation(ChooseAnim());

	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	if(auto currentCoords = app->map->WorldToCoordinates(position);
	   currentCoords != coordinates)
	{
		changedTile = true;
		coordinates = currentCoords;
	}
	else
		changedTile = false;

	// Move camera
	if(bMoveCamera)
		app->render->AdjustCamera(position);

	if(app->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
		bMoveCamera = !bMoveCamera;

	if(app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
		bGodMode = !bGodMode;

	return true;
}

void Player::BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
{
	switch (pBodyB->ctype)
	{
		using enum CL::ColliderLayers;
		case ITEMS:
			break;
		case PLATFORMS:
		{
			if(pBody->ground->ptr == fixtureA && pBody->body->GetLinearVelocity().y > 0)
			{
				if (bNormalJump) bLockAnim = false;

				bNormalJump = false;
				bHighJump = false;

				bKeepMomentum = true;
				velocityToKeep = pBody->body->GetLinearVelocity();
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
			if(iFrames == 0)
			{
				if(!bGodMode) hp -= 1;
				iFrames = 1;
				if(hp <= 0)
				{
					position.x -= 20;
					position.y -= 10;
					bDead = true;
					bAbleToMove = false;
					bLockAnim = false;
					pBody->body->SetLinearVelocity(b2Vec2(0, pBody->body->GetLinearVelocity().y));
					Disable();
					active = true;
				}
				else
				{
					position.x -= 20;
					position.y -= 10;
					bHurt = true;
					bAbleToMove = false;
				}
			}
			break;
		}
		case TRIGGERS:
		{
			if(bGodMode) break;
			hp = 0;
			iFrames = 1;
			bDead = true;
			bAbleToMove = false;
			bLockAnim = false;
			pBody->body->SetLinearVelocity(b2Vec2(0, pBody->body->GetLinearVelocity().y));
			Disable();
			active = true;
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

bool Player::IsOnAir() const
{
	return jump.bOnAir;
}

bool Player::Pause() const
{
	app->render->DrawCharacterTexture(
		texture->GetCurrentTexture(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	for(auto it = projectiles.begin(); it < projectiles.end(); ++it)
	{
		if(!it->get()) continue;
		if(!it->get()->Pause()) return false;
	}
	return true;
}

bool Player::DidChangeTile() const
{
	return changedTile;
}

void Player::SpecificRestart()
{
	for(auto &elem: projectiles)
		elem.reset();

	projectiles.clear();

	changedTile = true;
	bMoveCamera = true;
	skillCDTimer = 0;
	bFalling = 0;
	bDead = false;
	bHurt = false;
	bAttack2 = false;
	bClimbing = false;
	bPushing = false;
	bHighJump = false;
	bNormalJump = false;
	bRunning = false;
	bWalking = false;
	bIdle = false;
	bAttack1 = false;
	bLockAnim = false;
	bAbleToMove = true;
	bGodMode = false;
	bAttackQueue = false;
	attackDir = {0, 0};
}

bool Player::HasSaveData() const
{
	return true;
}

bool Player::LoadState(pugi::xml_node const &data)
{
	return false;
}

pugi::xml_node Player::SaveState(pugi::xml_node const &data)
{
	std::string saveData2 = "<{} {}=\"{}\"/>\n";
	std::string saveData4 = "<{} {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveOpenData4 = "<{} {}=\"{}\" {}=\"{}\">\n";
	std::string saveData6 = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveFloatData = "<{} {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string dataToSave = "<player>\n";
	dataToSave += AddSaveData(saveData4, "position", "x", position.x, "y", position.y);
	dataToSave += AddSaveData(saveFloatData, "velocity", "x", pBody->body->GetLinearVelocity().x, "y", pBody->body->GetLinearVelocity().y);
	dataToSave += AddSaveData(saveData4, "entity", "active", active, "disablenextupdate", disableOnNextUpdate);
	dataToSave += AddSaveData(saveData4, "character", "hp", hp, "iframes", iFrames);
	dataToSave += AddSaveData(saveData6, "jump", "onair", jump.bOnAir, "currentjumps", jump.currentJumps, "keepmomentum", bKeepMomentum);
	dataToSave += AddSaveData(saveData2, "anim", "falling", bFalling);
	dataToSave += AddSaveData(saveData4, "misc", "move", bMoveCamera, "skillcdtimer", skillCDTimer);
	if(bAttackQueue)
	{
		dataToSave += AddSaveData(saveOpenData4, "attack", "queue", bAttackQueue, "abletomove", bAbleToMove);
		dataToSave += AddSaveData(saveData6, "queue", "attack1", bAttack1, "attack2", bAttack2, "frame", texture->GetCurrentIndex());
		dataToSave += AddSaveData(saveFloatData, "dir", "x", attackDir.x, "y", attackDir.y);
		dataToSave += "</attack>";
	}
	else
	{
		dataToSave += AddSaveData(saveData4, "attack", "queue", bAttackQueue, "abletomove", bAbleToMove);
	}
	dataToSave += "</player>";

	app->AppendFragment(data, dataToSave.c_str());

	return data;
}
