#include "Character.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"
#include "Animation.h"

constexpr uint Character_SIZE = 30;

Character::Character() : Entity(EntityType::UNKNOWN) {}

Character::Character(EntityType type) : Entity(type) {}

Character::Character(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

Character::~Character() = default;

bool Character::Awake()
{
	SetStartingParameters();
	
	return true;
}

bool Character::Start()
{

	//initilize textures


	
	CreatePhysBody();

	return true;
}

bool Character::Update()
{
	
	//Update Character position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - Character_SIZE/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - Character_SIZE/2;

	app->render->DrawTexture(texture->GetCurrentFrame(), position.x, position.y);

	for(int i = 0; i < hp; i++)
	{
		app->render->DrawTexture(texture->GetCurrentFrame(), 710, 930 - i*(Character_SIZE + 10));
	}

	return true;
}

bool Character::CleanUp()
{
	//if(renderMode == RenderModes::IMAGE) app->tex->UnLoad(texture.image);
	texture->CleanUp();
	return true;
}

void Character::OnCollision(PhysBody *physA, PhysBody *physB)
{
	if(timeUntilReset >= 0) return;
	switch(physB->ctype)
	{
		case ColliderType::ITEM:
			if(score < 99999)
			{
				score += (float)(100 * scoreMultiplier);
				if(score > 99999) score = 99999;
			}
			LOG("Collision ITEM");
			break;
		case ColliderType::ANIM:
			LOG("Collision ANIM");
			break;
		case ColliderType::SENSOR:
			LOG("Collision SENSOR");
			break;
		case ColliderType::PLATFORM:
			LOG("Collision BOARD");
			break;
		case ColliderType::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
		default:
			LOG("HOW DID YOU GET HERE?!?!?!?");
	}
}

void Character::ResetScore()
{
	score = 0;
}

uint Character::GetScore() const
{
	return (uint)score;
}

void Character::AddMultiplier(uint n)
{
	scoreMultiplier += n;
}

int Character::GetTimeUntilReset() const
{
	return timeUntilReset;
}

std::pair<uint, uint> Character::GetScoreList() const
{
	return scoreList;
}



void Character::SetStartingPosition()
{
	if(pBody->body) app->physics->DestroyBody(pBody->body);
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	CreatePhysBody();
}
