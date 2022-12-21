#include "Item.h"
#include "App.h"

// Modules
#include "Textures.h"
#include "Render.h"
#include "Map.h"

// Utils
#include "Log.h"

#include <locale>	// std::tolower


Item::Item()
{
	name = "item";
}

Item::Item(TileInfo const *tileInfo, iPoint pos, int width, int height) : info(tileInfo), width(width), height(height)
{
	name = "item";
	
	itemClass = (*(std::get_if<std::string>(&tileInfo->properties.find("EntityClass")->second)));
	
	if(!itemClass.empty()) itemClass[0] = std::tolower(itemClass[0], std::locale());
	else LOG("Item does not have a class.");
	
	if(!tileInfo->collider.empty())
	{
		shape.Create("chain", tileInfo->collider[0].points);
		colliderOffset = iPoint(tileInfo->collider[0].x, tileInfo->collider[0].y);
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
}

void Item::CreatePhysBody()
{
	// Create PhysBody
	iPoint pBodyPos = position;

	auto itemBody = app->physics->CreateBody(
		position + colliderOffset
	);

	auto fixtureDef = app->physics->CreateFixtureDef(
		shape,
		(uint16)CL::ColliderLayers::ITEMS,
		(uint16)CL::ColliderLayers::PLAYER,
		true
	);

	itemBody->CreateFixture(fixtureDef.get());

	auto physBodyPtr = app->physics->CreatePhysBody(
		itemBody,
		iPoint(width, height),
		CL::ColliderLayers::ITEMS
	);

	pBody = std::move(physBodyPtr);
	pBody->listener = this;
}

Item::~Item() = default;

bool Item::Start() 
{
	SpawnEntity();

	return true;
}

bool Item::Update()
{  
	
	if(auto tex = anim->GetCurrentTexture().get(); tex) app->render->DrawTexture(tex, position.x, position.y);
	return true;
}

bool Item::Pause() const
{
	
	if(auto tex = anim->GetCurrentTexture().get(); tex) app->render->DrawTexture(tex, position.x, position.y);
	return true;
}

void Item::BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
{
	if((pBodyB->ctype & CL::ColliderLayers::PLAYER) == CL::ColliderLayers::PLAYER)
	{
 		PickUpEffect();
		Disable();
	}
}

void Item::PickUpEffect() const { /* TODO */ };

bool Item::HasSaveData() const
{
	return true;
}

bool Item::LoadState(pugi::xml_node const &data)
{
	return false;
}

pugi::xml_node Item::SaveState(pugi::xml_node const &data)
{
	std::string saveData2 = "<{} {}=\"{}\"/>\n";
	std::string saveOpenData2 = "<{} {}=\"{}\">\n";
	std::string saveData4 = "<{} {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveOpenData4 = "<{} {}=\"{}\" {}=\"{}\">\n";
	std::string saveData6 = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveFloatData = "<{} {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string dataToSave = "<item>\n";
	dataToSave += AddSaveData(saveData4, "item", "imgvariation", imageVariation, "class", itemClass);
	dataToSave += AddSaveData(saveData4, "position", "x", position.x, "y", position.y);
	dataToSave += AddSaveData(saveData4, "entity", "active", active, "disablenextupdate", disableOnNextUpdate);
	dataToSave += "</item>";

	app->AppendFragment(data, dataToSave.c_str());

	return data;
}
