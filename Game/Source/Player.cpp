#include "Player.h"
#include "App.h"

#include "Input.h"
#include "Render.h"
#include "Map.h"

#include "Log.h"

Player::Player()
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
	jump = { 
		.bJumping = false, 
		.currentJumps = 0, 
		.maxJumps = parameters.attribute("maxjumps").as_int(), 
		.timeSinceLastJump = 0, 
		.jumpImpulse = parameters.attribute("jumpimpulse").as_float(),
		.bInAir = false
	};

	startingPosition = {
		parameters.attribute("x").as_int(),
		parameters.attribute("y").as_int()
	};


	return true;
}

bool Player::Update()
{
	if(bKeepMomentum)
	{
		pBody->body->SetLinearVelocity(b2Vec2(velocityToKeep.x, 0));
		bKeepMomentum = false;
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

	if(app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !jump.bJumping && jump.currentJumps < jump.maxJumps)
	{
		jump.bJumping = true;
		jump.timeSinceLastJump = 0;
		jump.currentJumps++;
		jump.bInAir = true;
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

void Player::BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	switch (pBodyB->ctype)
	{
		using enum ColliderLayers;
		case ITEMS:
			LOG("Collision ITEMS");
			break;
		case PLATFORMS:
		{
			LOG("Collision PLATFORMS");
 			if(jump.bInAir
			   && pBody->ground->ptr == fixtureA
			   && pBody->body->GetPosition().y < pBodyB->body->GetPosition().y)
			{
				bKeepMomentum = true;
				velocityToKeep = pBody->body->GetLinearVelocity();
				jump = {
					.bJumping = false,
					.currentJumps = 0,
					.maxJumps = jump.maxJumps,
					.timeSinceLastJump = 0,
					.jumpImpulse = jump.jumpImpulse,
					.bInAir = false
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

void Player::OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
{
	if(!bKeepMomentum && jump.bInAir)
	{
	}
}
