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
#include "BitMask.h"
#include "Map.h"

Player::Player() : Character(EntityType::PLAYER)
{
	name = "player";
}

Player::~Player() = default;

bool Player::Awake() 
{
	currentCharacter = parameters.attribute("currentcharacter").as_string();
	scoreList.first = parameters.attribute("highscore").as_uint();
	scoreList.second = 0;
	jump = {false, 0, parameters.attribute("maxjumps").as_int(), 0, parameters.attribute("jumpimpulse").as_float()};
	
	SetStartingParameters();

	return true;
}

bool Player::Start() 
{
	using enum ColliderLayers;
	uint16 maskFlag = 0x0001;
	maskFlag = (uint16)(PLATFORMS | ENEMIES | ITEMS | TRIGGERS | CHECKPOINTS);
	CreatePhysBody((uint16)PLAYER, maskFlag);
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
				//app->SaveToConfig("scene", "Character", "highscore", std::to_string(scoreList.first));
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

	if(app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !jump.bJumping && jump.currentJumps <= jump.maxJumps)
	{
		jump.bJumping = true;
		jump.timeSinceLastJump = 0;
		jump.currentJumps++;

		impulse.y = jump.jumpImpulse * -1;

		pBody->body->SetLinearVelocity(b2Vec2(vel.x, 0));
		pBody->body->ApplyLinearImpulse(b2Vec2(0, impulse.y), pBody->body->GetWorldCenter(), true);
	}

	float maxVel = 5.0f;
	bool moveCamera = false;

	if(app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		impulse.x = b2Max(vel.x - 0.25f, maxVel * -1);
		moveCamera = true;
		if(texture->GetCurrentAnimName() != "walk")
			texture->SetCurrentAnimation("walk");
		dir = 1;
	}
	if(app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		impulse.x = b2Min(vel.x + 0.25f, maxVel);
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
	
	//Update player position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - pBody->width/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - pBody->height/2;
	app->render->DrawCharacterTexture(texture->GetCurrentFrame(), iPoint(position.x - 20, position.y - pBody->height - 7), (bool) dir, texture->GetFlipPivot());
	
	if(moveCamera && app->render->camera.x <= 0 && position.x >= startingPosition.x)
	{
		if(abs(app->render->camera.x) + cameraXCorrection <= app->map->mapData.width * app->map->mapData.tileWidth)
		{
			app->render->camera.x -= (vel.x * 0.90);
			if(app->render->camera.x > 0) app->render->camera.x = 0;
		}
		else if(vel.x < 0)
		{
			app->render->camera.x -= (vel.x * 0.90);
		}
	}
	return true;
}

bool Player::CleanUp()
{
	return true;
}

// L07 DONE 6: Define OnCollision function for the player. Check the virtual function on Entity class
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {

	// L07 DONE 7: Detect the type of collision

	switch (physB->ctype)
	{
		case ColliderType::ITEM:
			LOG("Collision ITEM");
			break;
		case ColliderType::PLATFORM:
			LOG("Collision PLATFORM");
 			if((pBody->body->GetPosition().y < physB->body->GetPosition().y))
			{
				jump = {false, 0, jump.maxJumps, 0, jump.jumpImpulse};
			}
			break;
		case ColliderType::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
	}
	


}
