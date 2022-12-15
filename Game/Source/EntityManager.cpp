#include "EntityManager.h"
#include "App.h"

#include "Player.h"
#include "Item.h"
#include "Enemy.h"

#include "Map.h"

#include "Defs.h"
#include "Log.h"

#include <regex>
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

	itemPath = config.attribute("items").as_string();

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

void EntityManager::CreateEntity(std::string const &entityClass, pugi::xml_node const &parameters)
{
	// Get correct ID (check if queue is empty or not)
	int id = (allEntities[entityClass].emptyElements.empty())
		? allEntities[entityClass].entities.size()
		: allEntities[entityClass].emptyElements.front();

	// Initialize pointer if node is a character, nullptr otherwise
	std::unique_ptr<Character>characterPtr = nullptr;
	if(StrEquals(entityClass, "player"))
	{
		characterPtr = std::make_unique<Player>(parameters);
		allEntities[entityClass].type = CL::ColliderLayers::PLAYER;
	}
	else if(StrEquals(entityClass, "enemy"))
	{
		characterPtr = std::make_unique<Enemy>(parameters, id);
		allEntities[entityClass].type = CL::ColliderLayers::ENEMIES;
	}
	else
	{
		// If it's not a previous case, we either misstyped something or it's an item/object
		LOG("Entity %s could not be created.", entityClass);
		return;
	}

	// Move the pointer to its place
	if(allEntities[entityClass].emptyElements.empty()) [[likely]]
		allEntities[entityClass].entities.push_back(std::move(characterPtr));
	else [[unlikely]]
	{
		allEntities[entityClass].entities[id].reset(characterPtr.release());
		allEntities[entityClass].emptyElements.pop_front();
	}
}

bool EntityManager::DestroyEntity(Entity const *entity, std::string_view type)
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
	using enum CL::ColliderLayers;
	auto excludedFlag = PLATFORMS | ITEMS;
	for(auto const &[name, info] : allEntities)
	{
		// If the type matches any value in excludedFlag go to next entity
		if((info.type & excludedFlag) != 0) continue;

		// If it doesn't, add textures for all entities with that name
		for(auto const &entity : info.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			dynamic_cast<Character *>(entity.get())->AddTexturesAndAnimationFrames();
		}
	}
	return true;
}

bool EntityManager::LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height)
{
	std::string aux = *(std::get_if<std::string>(&tileInfo->properties.find("EntityClass")->second));
	aux[0] = std::tolower(aux[0], std::locale());
	
	allEntities[aux].entities.push_back(std::make_unique<Item>(tileInfo, pos, width, height));
	allEntities[aux].type = static_cast<CL::ColliderLayers>(*(std::get_if<int>(&tileInfo->properties.find("ColliderLayers")->second)));

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
	const char *dirPath = itemPath.c_str();

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
		std::string fileName = itemPath + std::string(m[0]);

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

bool EntityManager::PreUpdate()
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			if(entity->disableOnNextUpdate) entity->Stop();
		}
	}
	return true;
};

bool EntityManager::PostUpdate()
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			entity->StopProjectiles();
		}
	}
	return true;
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

bool EntityManager::Pause(int phase)
{
	for(auto const &[entityType, entityInfo] : allEntities)
	{
		for(auto const &entity : entityInfo.entities)
		{
			if(!IsEntityActive(entity.get())) continue;
			if(!entity->Pause()) return false;
		}
	}
	return true;
}

Player *EntityManager::GetPlayerCharacter() const
{
	if(auto it = allEntities.find("player");
	   it != allEntities.end())
		return dynamic_cast<Player *>(it->second.entities.front().get());
	return nullptr;
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