#include "Player.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"
#include "BitMaskColliderLayers.h"
#include "Map.h"
#include <iostream>

Player::Player() : Character(ColliderLayers::PLAYER)
{
	name = "player";
}

Player::Player(pugi::xml_node const &itemNode = pugi::xml_node()) : Character(itemNode)
{ 
	name = "player";
}

Player::~Player() = default;

bool Player::Awake() 
{
	currentCharacter = parameters.attribute("class").as_string();
	scoreList.first = parameters.attribute("highscore").as_uint();
	scoreList.second = 0;
	jump = { 
		.bJumping = false, 
		.currentJumps = 0, 
		.maxJumps = parameters.attribute("maxjumps").as_int(), 
		.timeSinceLastJump = 0, 
		.jumpImpulse = parameters.attribute("jumpimpulse").as_float()
	};
	
	SetStartingParameters();

	return true;
}

bool Player::Start() 
{

	CreatePhysBody();
	texture->SetCurrentAnimation("idle");
	if(!texture->Start("idle"))
	{
		LOG("Couldnt start %s anim", texture->GetCurrentAnimName());
		return false;
	}
	texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);


	return true;
}

bool Player::Update()
{
	if(timeUntilReset > 120)
	{
		SetStartingPosition();
		timeUntilReset = -1;
		if(hp <= 0)
		{
			if((uint)score > scoreList.first)
			{
				scoreList.first = (uint)score;
				//app->SaveToConfig("scene", "Character", "highscore", std::to_string(scoreList.first))
			}
			scoreList.second = (uint)score;
			ResetScore();
			hp = 3;
		}
	}
	else if(timeUntilReset >= 0)
	{
		timeUntilReset++;
	}
	else
	{
		score += 0.002f * (float)scoreMultiplier;
	}

	if(jump.bJumping)
	{
		if(app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP) jump.bJumping = false;
		jump.timeSinceLastJump++;
	}

	b2Vec2 vel = pBody->body->GetLinearVelocity();
	b2Vec2 impulse = b2Vec2_zero;
	float maxVel = 5.0f;
	bool moveCamera = false;

	if(app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !jump.bJumping && jump.currentJumps <= jump.maxJumps)
	{
		jump.bJumping = true;
		jump.timeSinceLastJump = 0;
		jump.currentJumps++;

		impulse.y = jump.jumpImpulse * -1;

		pBody->body->SetLinearVelocity(b2Vec2(vel.x, 0));
		pBody->body->ApplyLinearImpulse(b2Vec2(0, impulse.y), pBody->body->GetWorldCenter(), true);
	}

	if(app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		if(vel.y != 0) impulse.x = b2Max(vel.x - 0.15f, maxVel * -1);
		else impulse.x = b2Max(vel.x - 0.25f, maxVel * -1);
		moveCamera = true;
		if(texture->GetCurrentAnimName() != "walk")
			texture->SetCurrentAnimation("walk");
		dir = 1;
	}
	if(app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		if(vel.y != 0) impulse.x = b2Min(vel.x + 0.15f, maxVel);
		else impulse.x = b2Min(vel.x + 0.25f, maxVel);
		moveCamera = true;
		if(texture->GetCurrentAnimName() != "walk")
			texture->SetCurrentAnimation("walk");
		dir = 0;
	}
	if(impulse.x == 0) impulse.x = vel.x * 0.98f;

	pBody->body->SetLinearVelocity(b2Vec2(impulse.x, pBody->body->GetLinearVelocity().y));


	if(app->input->GetKey(SDL_SCANCODE_A) == KEY_IDLE && app->input->GetKey(SDL_SCANCODE_D) == KEY_IDLE && texture->GetCurrentAnimName() != "idle")
	{
		 texture->SetCurrentAnimation("idle");
	}
	
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	SDL_Rect camera = app->render->GetCamera();
	
	if(moveCamera && camera.x <= 0 && position.x >= startingPosition.x)
	{
		if(abs(camera.x) + cameraXCorrection <= app->map->GetWidth() * app->map->GetTileWidth())
		{
			camera.x -= (int)(vel.x * 0.98);
			if(camera.x > 0) camera.x = 0;
		}
		else if(vel.x < 0)
		{
			camera.x -= (int)(vel.x * 0.98);
		}
	}
	return true;
}

bool Player::CleanUp()
{
	return true;
}

void Player::SendContact(b2Contact *c)
{
	auto const *pBodyA = (PhysBody *)c->GetFixtureA()->GetBody()->GetUserData();
	auto const *pBodyB = (PhysBody *)c->GetFixtureB()->GetBody()->GetUserData();
	std::cout << "Fixture A: " << pBodyA << std::endl;
	std::cout << "Fixture B: " << pBodyB << std::endl;
	std::cout << "[" 
		<< std::to_string(c->GetManifold()->points[0].localPoint.x) << ", " 
		<< std::to_string(c->GetManifold()->points[0].localPoint.y) << "]" << std::endl;
	std::cout << "[" 
		<< std::to_string(c->GetManifold()->localPoint.x) << ", " 
		<< std::to_string(c->GetManifold()->localPoint.y) << "]" << std::endl;
	std::cout << "[" 
		<< std::to_string(c->GetManifold()->localNormal.x) << ", " 
		<< std::to_string(c->GetManifold()->localNormal.y) << "]" << std::endl;

}

void Player::OnCollisionStart(PhysBody* physA, PhysBody* physB) 
{
	switch (physB->ctype)
	{
		using enum ColliderLayers;
		case ITEMS:
			LOG("Collision ITEMS");
			break;
		case PLATFORMS:
			LOG("Collision PLATFORMS");
 			if(pBody->body->GetPosition().y < physB->body->GetPosition().y)
			{
				jump = {false, 0, jump.maxJumps, 0, jump.jumpImpulse};
			}
			break;
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
