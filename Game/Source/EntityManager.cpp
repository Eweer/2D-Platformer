#include "EntityManager.h"
#include "Player.h"
#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"
#include "BitMaskColliderLayers.h"
#include "Animation.h"
#include "Physics.h"

#include "Defs.h"
#include "Log.h"
#include "dirent.h"

#include <algorithm>
#include <ranges>
#include <vector>
#include <regex>
#include <variant>
#include <string>		// std::string, std::stoi
#include <locale>		// std::tolower

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
	CreateAllColliders();

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

void EntityManager::CreateEntity(std::string const &type, pugi::xml_node parameters)
{
	if(StrEquals(type, "player"))
	{
		if(allEntities[type].emptyElements.empty()) [[likely]]
		{
			allEntities[type].entities.push_back(std::make_unique<Player>(parameters));
		}
		else [[unlikely]]
		{
			allEntities[type].entities[allEntities[type].emptyElements.front()].reset(new Player(parameters));
			allEntities[type].emptyElements.pop_front();
		}
	}
	else if(StrEquals(type, "item"))
	{
		if(allEntities[type].emptyElements.empty()) [[likely]]
		{
			allEntities[type].entities.push_back(std::make_unique<Item>());
		}
		else [[unlikely]]
		{
			allEntities[type].entities[allEntities[type].emptyElements.front()].reset(new Item());
			allEntities[type].emptyElements.pop_front();
		}
	}
	else
	{
		LOG("Entity %s could not be created.", type);
	}
}

bool EntityManager::DestroyEntity(Entity const *entity, std::string const &type)
{
	if(auto vec = allEntities.find(type); vec != allEntities.end())
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

bool EntityManager::DestroyEntity(std::string const &type, int id)
{
	if(id < 0) return false;

	allEntities[type].entities.at(id)->CleanUp();
	allEntities[type].emptyElements.push_back(id);

	return false;
}

bool EntityManager::LoadAllTextures()
{
	if(auto const &it = allEntities.find("player"); it != allEntities.end())
	{
		for(auto const &entity : it->second.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			entity->AddTexturesAndAnimationFrames();
		}
	}
	return true;
}

bool EntityManager::LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height)
{
	std::string aux = *(std::get_if<std::string>(&tileInfo->properties.find("EntityClass")->second));
	aux[0] = std::tolower(aux[0], std::locale());
	
	allEntities[aux].entities.push_back(std::make_unique<Item>(tileInfo, pos, width, height));

	return true;
}

void EntityManager::LoadItemAnimations()
{
	// We loop through all the files in the directory looking for an entity 
	// that matches the name of the file.
	// 
	// https://regex101.com/r/96aJfl/3
	// itemAnimMatch[0] = file name
	// itemAnimMatch[1] = (ItemType) -> Coin, Gem, Potion... Any word
	// itemAnimMatch[2] = (From 0 to 3 digis) -> Image Variation
	// Non-capturing = (?:0 or 1 underscores)
	// itemAnimMatch[3] = (AnimationName) -> Idle, rotating...
	// itemAnimMatch[4] = (0 to 3 digits) -> Frame number
	// (.png or .jpg) -> File extension

	static const std::regex r2(R"(([A-aZ-z]+)(\d{1,3})_([A-aZ-z]+)(\d{0,3})\.(?:png|jpg))");

	// TODO: Refractor this to use <filesystem> instead of dirent
	struct dirent **folderList;
	std::string entityFolder = "Assets/Animations/Items/";
	const char *dirPath = entityFolder.c_str();

	int nItemFolder = scandir(dirPath, &folderList, nullptr, DescAlphasort);

	if (nItemFolder < 0) return;

	while (nItemFolder--)
	{
		std::smatch m;

		// Match the regex with the file name and check if it does. If it doesn't we go to the next file.
		if (std::string animFileName(folderList[nItemFolder]->d_name); 
			!std::regex_match(animFileName, m, r2))
		{
			free(folderList[nItemFolder]);
			continue;
		}
		
		// Match 1 is the class (Coin, Gem, Orc, ...) of the entity.
		std::string entityClass = m[1];
		entityClass[0] = (char)std::tolower(entityClass[0]);

		// Check if we have an entiy with m[1] class
		if(auto entityTypeInfo = allEntities.find(entityClass); 
		   entityTypeInfo == allEntities.end())
			continue;

		Item const *entity = nullptr;
		int variation = 0;

		// For all entities of that class...
		for(auto const &elem : allEntities[entityClass].entities)
		{
			// Check if we have one with m[2] animation number
			// If we do, we have to load the animation
			if(variation = elem.get()->imageVariation; 
			   variation == std::stoi(m.str(2)))
			{
				entity = dynamic_cast<Item*>(elem.get());
				break;
			}
		}
		// Check if we matched an entity or not in the loop.
		if(!DoesEntityExist(entity)) continue;

		// Check if we already have the animation on the Animation map
		if(auto anim = allEntities[entityClass].animation.find(std::stoi(m.str(2)));
		   anim == allEntities[entityClass].animation.end())
		{
			// If we don't, we create a new one
			allEntities[entityClass].animation[variation] = std::make_unique<Animation>();
		}

		// Create the path of the file
		// fileName = "Assets/Output/Item/" + "coin0_rotating000.png"
		std::string fileName = entityFolder + std::string(m[0]);

		auto animationName = std::string(m[3]);
		[[likely]] if(auto const frameCount = allEntities[entityClass].animation[variation]->AddFrame(fileName.c_str(), animationName); 
		   frameCount != 1) 
		{	
			//If we have more than one frame we don't need to set up properties
			continue;
		}

		// TODO: Load properties from XML
		/*
		if(auto speed = entity->info->properties.find("AnimationSpeed")
		   speed != entity->info->properties.end() && std::get<float>(speed->second) > 0)
			allEntities[entityClass].animation[variation]->SetSpeed(std::get<float>(speed->second))
		else
	
		if(auto speed = entity->info->properties.find("AnimationStyle")
		   speed != entity->info->properties.end() && std::get<int>(speed->second) > 0)
			allEntities[entityClass].animation[variation]->SetAnimStyle(static_cast<AnimIteration>(std::get<int>(speed->second)))
		else
		*/

		allEntities[entityClass].animation[variation]->SetSpeed(0.1f);
		allEntities[entityClass].animation[variation]->SetAnimStyle(AnimIteration::LOOP_FROM_START);

		for(auto const &elem : allEntities[entityClass].entities)
		{
			auto *itemEntity = dynamic_cast<Item *>(elem.get());
			if(!itemEntity || itemEntity->anim) continue;
			if(variation = itemEntity->imageVariation;
			   variation == std::stoi(m.str(2)))
			{
				itemEntity->anim = allEntities[entityClass].animation[variation].get();
			}
		}

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
	if(entity) return true;
	return false;
}

bool EntityManager::Update(float dt)
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		if(!StrEquals(entityType, "player"))
		{
			for(auto const &[key, anim] : entityInfo.animation)
			{
				anim->UpdateAndGetFrame();
			}
		}
		for(auto const &entity : entityInfo.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			if(!entity->Update()) return false;
		}
	}
	return true;
}

void EntityManager::CreateAllColliders()
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		if(StrEquals(entityType, "player")) continue;

		for(auto const &entity : entityInfo.entities)
		{
			if(!IsEntityActive(entity.get()) || StrEquals(entity->name, "item")) 
				continue;
		}
	}
}