#include "EntityManager.h"
#include "Player.h"
#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"
#include "BitMaskColliderLayers.h"

#include "Defs.h"
#include "Log.h"

#include <algorithm>	//range::for_each
#include <vector>
#include <regex>

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager() = default;

// Called before render is available
bool EntityManager::Awake(pugi::xml_node& config)
{
	LOG("Loading Entity Manager");
	for(auto const &[type, item] : allEntities)
	{
		for(auto &entity : item.entities)
		{
			if(!entity.get() || !entity->active) continue;
			if(!entity->Awake()) return false;
		}
	}
	return true;
}

bool EntityManager::Start() 
{
	for(auto const &[type, item] : allEntities)
	{
		for(auto &entity : item.entities)
		{
			if(!entity.get() || !entity->active) continue;
			if(!entity->Start()) return false;
		}
	}
	return true;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	for(auto const &[type, item] : allEntities)
	{
		for(auto &entity : item.entities)
		{
			if(!entity.get()) continue;
			if(!entity->CleanUp()) return false;
		}
	}

	return true;
}

void EntityManager::CreateEntity(ColliderLayers type, pugi::xml_node parameters)
{
	switch (type)
	{
	case ColliderLayers::PLAYER:
		if(allEntities[type].emptyElements.empty()) [[likely]]
		{
			allEntities[type].entities.push_back(std::make_unique<Player>(parameters));
		}
		else [[unlikely]]
		{
			allEntities[type].entities[allEntities[type].emptyElements.front()].reset(new Player(parameters));
			allEntities[type].emptyElements.pop_front();
		}
		break;

	case ColliderLayers::ITEMS:
		if(allEntities[type].emptyElements.empty()) [[likely]]
		{
			allEntities[type].entities.push_back(std::make_unique<Item>());
		}
		else [[unlikely]]
		{
			allEntities[type].entities[allEntities[type].emptyElements.front()].reset(new Item());
			allEntities[type].emptyElements.pop_front();
		}
		break;

	default: 
		LOG("Entity could not be created.");
		break;
	}
}

bool EntityManager::DestroyEntity(Entity const *entity)
{
	if(auto vec = allEntities.find(entity->type); vec != allEntities.end())
	{
		for(auto &item : vec->second.entities)
		{
			if(item.get() == entity)
			{
				item->CleanUp();
				item.reset();
				return true;
			}
		}
	}
	return false;
}

bool EntityManager::DestroyEntity(ColliderLayers type, int id)
{
	if(id < 0) return false;

	allEntities[type].entities.at(id)->CleanUp();
	allEntities[type].emptyElements.push_back(id);

	return false;
}

bool EntityManager::LoadAllTextures()
{
	for(auto const &[type, item] : allEntities)
	{
		using enum ColliderLayers;
		switch(type)
		{
			case PLAYER:
			{
				for(auto &entity : item.entities)
				{
					if(!IsEntityActive(entity.get())) continue;
					entity->AddTexturesAndAnimationFrames();
				}
				break;
			}
			case ITEMS:
			{

				break;
			}
			default:
				break;
		}
	}
	return true;
}

bool EntityManager::LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height)
{
	auto aux = (ColliderLayers)*(std::get_if<int>(&tileInfo->properties.find("ColliderLayers")->second));
	auto variationNumber = *(std::get_if<int>(&tileInfo->properties.at("ImageVariation")));
	
	allEntities[aux].entities.push_back(std::make_unique<Item>(tileInfo, pos, width, height));

	if(auto variation = allEntities[aux].animation.find(variationNumber); 
		variation == allEntities[aux].animation.end())
	{
		allEntities[aux].animation[variationNumber] = std::make_unique<Animation>();
		LoadItemAnimation(tileInfo, aux, variationNumber);
	}


	return true;
}

void EntityManager::LoadItemAnimation(TileInfo const *tileInfo, ColliderLayers layer, int variation)
{
	auto currentAnim = allEntities[layer].animation[variation].get();
	
	struct dirent **folderList;
	const char *dirPath = allEntities[layer].entities.at(0).get()->texturePath.c_str();
	int nItemFolder = scandir(dirPath, &folderList, nullptr, DescAlphasort);
	static const std::regex r(R"(([A-Za-z]+)(\d*)_anim\d{0-3}.(?:png)|(?:jpg))");
	std::string rAux = "(Coin" + std::to_string(variation) + "_anim\d{0-3}.(?:png)|(?:jpg))";
	static const std::regex r2(R"(rAux)");

	if (nItemFolder < 0) return;

	while (nItemFolder--)
	{
		std::smatch m;
		if (std::string animFileName(folderList[nItemFolder]->d_name); 
			!std::regex_match(animFileName, m, rAux))
		{
			free(folderList[nItemFolder]);
			continue;
		}
		//if(std::string match1 = m[1]; match1 != std::string(parameters.name()))
	}

}

bool EntityManager::IsEntityActive(Entity const *entity) const
{
	if(!entity) return false;
	if(!entity->active) return false;
	return true;
}

bool EntityManager::DoesEntityExist(Entity const *entity) const
{
	if(entity != nullptr) return false;
	return false;
}

bool EntityManager::Update(float dt)
{
	for(auto const &[type, item] : allEntities)
	{
		for(auto &entity : item.entities)
		{
			if(!entity.get() || !entity->active) continue;
			if(!entity->Update()) return false;
		}
	}
	return true;
}