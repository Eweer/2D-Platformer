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

#include <locale>	// std::tolower
#include <bit>		// std::bit_cast
#include <ranges>
#include <iostream>


Item::Item() : Entity(ColliderLayers::ITEMS)
{
	name = "item";
}

Item::Item(TileInfo const *tileInfo, iPoint pos, int width, int height) : Entity(ColliderLayers::ITEMS), info(tileInfo), width(width), height(height)
{
	name = "item";
	
	itemClass = (*(std::get_if<std::string>(&tileInfo->properties.find("EntityClass")->second)));
	
	if(!itemClass.empty()) itemClass[0] = std::tolower(itemClass[0], std::locale());
	else LOG("Item does not have a class.");
	
	for(auto const &elem : tileInfo->collider)
	{
		colliderMap["idle"].emplace_back(
			std::make_pair<ShapeData, iPoint>(
				ShapeData("chain", elem.points),
				iPoint(elem.x, elem.y)
			)
		);
	}
	
	imageVariation = *(std::get_if<int>(&tileInfo->properties.find("ImageVariation")->second));
	
	if(imageVariation < 0)
	{
		imageVariation = 0;
		LOG("Item %itemClass does not have a valid image variation.");
	}
	
	texturePath = *(std::get_if<std::string>(&tileInfo->properties.find("TexturePath")->second));
	fxPath = *(std::get_if<std::string>(&tileInfo->properties.find("FxPath")->second));
	
	startingPosition = pos;
	position = startingPosition;

	CreatePhysBody();
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

void Item::CreatePhysBody()
{
	// Create PhysBody
	iPoint pBodyPos = position;
	
	if(!colliderMap.begin()->second.empty())
	{
		pBodyPos.x += colliderMap.begin()->second.front().second.x;
		pBodyPos.y += colliderMap.begin()->second.front().second.y;
	}

	
	auto itemBody = app->physics->CreateBody(
		pBodyPos
	);

	std::unique_ptr<PhysBody>physBodyPtr(
		app->physics->CreatePhysBody(
			itemBody,
			iPoint(width, height),
			ColliderLayers::ITEMS
		)
	);
	pBody = std::move(physBodyPtr);
	pBody->listener = this;

	// Fill all fixtureDef (all colliders of item) of PhysBody and respective X,Y offsets
	for(auto &[action, shapeData] : colliderMap)
	{
		auto rShapeData = std::ranges::subrange(shapeData.rbegin(), shapeData.rend());
		for(auto &[shape, offset] : rShapeData)
		{
			std::unique_ptr<b2FixtureDef> fixtureDef(
				app->physics->CreateFixtureDef(
					shape,
					(uint16)ColliderLayers::ITEMS,
					(uint16)ColliderLayers::PLAYER,
					true
				)
			);
			pBody->fixtures.push_back(std::move(fixtureDef));
			pBody->offsets.push_back(offset);
		}
	}

	// If the body has colliders
	if(!pBody->fixtures.empty())
	{
		auto itemPhysBody = pBody.get();
		itemBody->CreateFixture(itemPhysBody->fixtures.front().get());
		colliderOffset = pBody->offsets.front();
	}
}

Item::~Item() = default;

void Item::SendContact(b2Contact *c)
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

bool Item::Awake() 
{
	SetStartingParameters();

	return true;
}

bool Item::Start() 
{
	return true;
}

bool Item::Update()
{  
	//position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - 16
	//position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - 16

	if(anim) app->render->DrawTexture(anim->GetCurrentFrame(), position.x, position.y);
	return true;
}

bool Item::Pause() const
{
	if(anim) app->render->DrawTexture(anim->GetCurrentFrame(), position.x, position.y);
	return true;
}

bool Item::CleanUp()
{
	return true;
}

void Item::AddTexturesAndAnimationFrames()
{
	// TODO
}
