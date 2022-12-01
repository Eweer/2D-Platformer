#include "EntityManager.h"
#include "Player.h"
#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"

#include "Defs.h"
#include "Log.h"

#include <algorithm>
#include <vector>

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
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item.first)
		{
			if(!entity.get() || !entity->active) continue;
			if(!entity->Awake()) return false;
		}
	}
	return true;
}

bool EntityManager::Start() 
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item.first)
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
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item.first)
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
		if(entities2[type].second.empty()) [[likely]]
		{
			entities2[type].first.push_back(std::make_unique<Player>(parameters));
		}
		else [[unlikely]]
		{
			entities2[type].first[entities2[type].second.front()].reset(new Player(parameters));
			entities2[type].second.pop_front();
		}
		break;

	case ColliderLayers::ITEMS:
		if(entities2[type].second.empty()) [[likely]]
		{
			entities2[type].first.push_back(std::make_unique<Item>());
		}
		else [[unlikely]]
		{
			entities2[type].first[entities2[type].second.front()].reset(new Item());
			entities2[type].second.pop_front();
		}
		break;

	default: 
		LOG("Entity could not be created.");
		break;
	}
}

bool EntityManager::DestroyEntity(Entity const *entity)
{
	if(auto vec = entities2.find(entity->type); vec != entities2.end())
	{
		for(auto &item : vec->second.first)
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

	entities2[type].first.at(id)->CleanUp();
	entities2[type].second.push_back(id);

	return false;
}

bool EntityManager::LoadAllTextures() const
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item.first)
		{
			if(!entity.get() || !entity->active) continue;
			entity->AddTexturesAndAnimationFrames();
		}
	}
	return true;
}

bool EntityManager::LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height)
{
	return true;
}

bool EntityManager::Update(float dt)
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item.first)
		{
			if(!entity.get() || !entity->active) continue;
			if(!entity->Update()) return false;
		}
	}
	return true;
}