#include "EntityManager.h"
#include "Player.h"
#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"
#include "BitMaskColliderLayers.h"
#include "Animation.h"

#include "Defs.h"
#include "Log.h"

#include <algorithm>
#include <vector>
#include <regex>
#include <variant>

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
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			if(!entity->Awake()) return false;
		}
	}
	return true;
}

bool EntityManager::Start() 
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			if(!entity->Start()) return false;
		}
	}
	return true;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			if(!DoesEntityExist(entity.get())) continue;
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

bool EntityManager::DestroyEntity(std::string type, int id)
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
	std::string aux = *(std::get_if<std::string>(&tileInfo->properties.find("Type")->second));
	
	allEntities[aux].entities.push_back(std::make_unique<Item>(tileInfo, pos, width, height));
	LoadItemAnimations(tileInfo);

	return true;
}

void EntityManager::LoadItemAnimations(TileInfo const *tileInfo)
{
	std::string layer = *(std::get_if<std::string>(&tileInfo->properties.find("Type")->second));

	struct dirent **folderList;
	const char *dirPath = "Assets/Animations/Items/"; //allEntities[layer].entities.at(0).get()->texturePath.c_str();
	int nItemFolder = scandir(dirPath, &folderList, nullptr, DescAlphasort);

	// https://regex101.com/r/L9Wa29/1
	// Looks for a file named:
	// m[0] = file name
	// m[1] = (ItemType) -> Coin, Gem, Potion... Any word
	// m[2] = (From 0 to 3 digis) -> Image Variation
	// m[3] = (?:0 or 1 underscores)(AnimationName) -> Idle, rotating... Might not be one
	// _anim -> The word anim
	// m[4] = (0 to 3 digits) -> Frame number
	// (.png or .jpg) -> File extension

	std::regex r2(R"(([A-aZ-z]+)(\d{1,3})(?:_?([A-aZ-z]*))_anim(\d{0,3})\.(?:png|jpg))");

	if (nItemFolder < 0) return;

	while (nItemFolder--)
	{
		
		std::smatch m;

		// Check if file matches the regex
		if (std::string animFileName(folderList[nItemFolder]->d_name); 
			!std::regex_match(animFileName, m, r2))
		{
			free(folderList[nItemFolder]);
			continue;
		}
		
		// Check if we have an entiy with m[1] type
		if(auto entityTypeInfo = allEntities.find(m[1]); 
		   entityTypeInfo == allEntities.end())
			continue;

		// Check if we have an entity with m[2] animation number
		bool bVariationFound = false;
		int variation;
		for(auto const &elem : allEntities[m[1]].entities)
		{
			if(variation = elem.get()->imageVariation; 
			   variation == stoul(m[2]))
			{
				bVariationFound = true;
				break;
			}
		}
		if(!bVariationFound) continue;

		// Check if we already have the animation on the Animation map
		if(auto anim = allEntities[m[1]].animation.find(stoul(m[2]));
		   anim != allEntities[m[1]].animation.end())
			continue;
		
		// If we don't have the animation, we create a new one
		allEntities[m[1]].animation.emplace(std::make_unique<Animation>());

		// Add the frame to animation map
		// if there's no name (ex: coin0_anim000.png), it will be stored with the name "single"
		if(allEntities[m[1]].animation[variation]->AddFrame((std::string(dirPath) + std::string(m[0])).c_str(), std::string(m[3])) != 1) [[likely]]
			continue;


		// if it's the first frame we add with m[3] name, set up animation variables
		if(auto speed = tileInfo->properties.find("AnimationSpeed");
		   speed != tileInfo->properties.end() && std::get<float>(speed->second) > 0)
			allEntities[layer].animation[variation]->SetSpeed(std::get<float>(speed->second));
		else
			allEntities[layer].animation[variation]->SetSpeed(0.2f);

		if(auto speed = tileInfo->properties.find("AnimationStyle");
		   speed != tileInfo->properties.end() && std::get<int>(speed->second) > 0)
			allEntities[layer].animation[variation]->SetAnimStyle(static_cast<AnimIteration>(std::get<int>(speed->second)));
		else
			allEntities[layer].animation[variation]->SetAnimStyle(AnimIteration::LOOP_FROM_START);

		free(folderList[nItemFolder]);
	}
	free(folderList);
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
		for(auto const &[entityType, entityInfo] : item)
		{
			for(auto const &entity : entityInfo.entities)
			{
				if(!IsEntityActive(entity.get())) continue;
				if(!entity->Update()) return false;
			}
		}
	}
	return true;
}