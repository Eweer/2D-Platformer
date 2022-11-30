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
		for(auto &entity : item)
		{
			if(!entity->active) continue;
			if(!entity->Awake()) return false;
		}
	}
	return true;
}

bool EntityManager::Start() 
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item)
		{
			if(!entity->active) continue;
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
		for(auto &entity : item)
		{
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
		entities2[type].push_back(std::make_unique<Player>(parameters));
		break;

	case ColliderLayers::ITEMS:
		entities2[type].push_back(std::make_unique<Item>());
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
		for(auto &item : vec->second)
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

	entities2[type].at(id)->CleanUp();
	entities2[type].at(id).reset();

	return false;
}

bool EntityManager::LoadAllTextures() const
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item)
		{
			if(!entity->active) continue;
			entity->AddTexturesAndAnimationFrames();
		}
	}
	return true;
}

bool EntityManager::Update(float dt)
{
	for(auto const &[type, item] : entities2)
	{
		for(auto &entity : item)
		{
			if(!entity->active) continue;
			if(!entity->Update()) return false;
		}
	}
	return true;
}