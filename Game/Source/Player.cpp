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

Player::Player() : Character(EntityType::PLAYER)
{
	name = "player";
}

Player::~Player() = default;

bool Player::Awake() 
{
	scoreList.first = parameters.attribute("highscore").as_uint();
	scoreList.second = 0;
	
	SetStartingParameters();

	return true;
}

bool Player::Start() 
{
	using enum ColliderLayers;
	uint16 maskFlag = 0x0001;
	maskFlag = (uint16)(PLATFORMS | ENEMIES | ITEMS | TRIGGERS | CHECKPOINTS);
	CreatePhysBody((uint16)PLAYER, maskFlag);
	if(texture->GetFrameCount() > 1)
	{
		texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);
		texture->Start();
	}

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

	// L07 DONE 5: Add physics to the player - updated player position using physics

	int speed = 10; 
	b2Vec2 vel = b2Vec2(0, -GRAVITY_Y); 

	//L02: DONE 4: modify the position of the player using arrow keys and render the texture
	if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		//
	}
	if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		//
	}

	
	if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		vel = b2Vec2(-speed, -GRAVITY_Y);
		if(texture->GetCurrentAnimName() != "walk") 
			texture->SetCurrentAnimation("walk");
		dir = 1;
	}

	if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		vel = b2Vec2(speed, -GRAVITY_Y);
		if(texture->GetCurrentAnimName() != "walk") 
			texture->SetCurrentAnimation("walk");
		dir = 0;
	}

	if(app->input->GetKey(SDL_SCANCODE_A) == KEY_IDLE && app->input->GetKey(SDL_SCANCODE_D) == KEY_IDLE && texture->GetCurrentAnimName() != "idle")
	{
		 texture->SetCurrentAnimation("idle");
	}
	
	

	//Set the velocity of the pbody of the player
	pBody->body->SetLinearVelocity(vel);

	//Update player position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - pBody->width/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - pBody->height/2;
	app->render->DrawCharacterTexture(texture->GetCurrentFrame(), iPoint(position.x, position.y), (bool) dir);
	
	//app->render->DrawTexture(texture->GetCurrentFrame(), position.x, position.y);

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
			break;
		case ColliderType::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
	}
	


}
