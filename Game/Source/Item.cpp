#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{
	name = "item";
}

Item::~Item() = default;

bool Item::Awake() 
{
	SetStartingParameters();

	return true;
}

bool Item::Start() 
{

	using enum ColliderLayers;
	uint16 maskFlag = 0x0001;
	maskFlag = (uint16)(PLAYER);
	CreatePhysBody((uint16)ITEMS, maskFlag);

	return true;
}

bool Item::Update()
{
	// L07 DONE 4: Add a physics to an item - update the position of the object from the physics.  
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - 16;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - 16;

	app->render->DrawTexture(texture->GetCurrentFrame(), position.x, position.y);

	return true;
}

bool Item::CleanUp()
{
	return true;
}