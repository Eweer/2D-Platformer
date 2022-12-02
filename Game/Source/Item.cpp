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
#include "Map.h"
#include "Entity.h"

Item::Item() : Entity(ColliderLayers::ITEMS)
{
	name = "item";
}

Item::Item(TileInfo const *tileInfo, iPoint pos, int width, int height) : Entity(ColliderLayers::ITEMS), info(tileInfo), width(width), height(height)
{
	name = "item";
	this->type2 = *(std::get_if<std::string>(&tileInfo->properties.find("Type")->second));
	this->imageVariation = *(std::get_if<int>(&tileInfo->properties.find("ImageVariation")->second));
	this->startingPosition = pos;
}

void Item::SetPaths()
{ 
	texturePath = "Assets/Animation/Items/";
	fxPath = "Assets/Audio/Fx";
}

bool Item::SetStartingParameters()
{
	SetPaths();
	return true;
}

//void Item::SetStartingPosition()


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
	/*
	texture->SetCurrentAnimation("idle");
	if(!texture->Start("idle"))
	{
		LOG("Couldnt start %s anim", texture->GetCurrentAnimName());
		return false;
	}
	texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);
	*/
	return true;
}

bool Item::Update()
{  
	//position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - 16;
	//position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - 16;

	//app->render->DrawTexture(texture->GetCurrentFrame(), position.x, position.y);

	return true;
}

bool Item::CleanUp()
{
	return true;
}

void Item::AddTexturesAndAnimationFrames()
{

}
