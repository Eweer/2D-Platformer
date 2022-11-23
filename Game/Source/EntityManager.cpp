#include "EntityManager.h"
#include "Player.h"
#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Scene.h"

#include "Defs.h"
#include "Log.h"

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake(pugi::xml_node& config)
{
	LOG("Loading Entity Manager");
	for (ListItem<Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		if (!item->data->active) continue;
		if (!item->data->Awake()) return false;
	}
	return true;
}

bool EntityManager::Start() 
{
	for(ListItem<Entity *> *item = entities.start; item != nullptr; item = item->next)
	{
		if(!item->data->active) continue;
		if(!item->data->Start()) return false;
	}
	return true;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	ListItem<Entity*>* item = entities.end;
	while (item != nullptr)
	{
		if(!item->data->CleanUp()) return false;
		item = item->prev;
	}
	entities.Clear();

	return true;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr; 
	switch (type)
	{
	case EntityType::PLAYER:
		entity = new Player();
		break;

	case EntityType::ITEM:
		entity = new Item();
		break;

	default: break;
	}

	// Created entities are added to the list
	AddEntity(entity);

	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	for (ListItem<Entity*>* item = entities.start; item != nullptr; item = item->next)
	{
		if (item->data == entity) entities.Del(item);
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if ( entity != nullptr) entities.Add(entity);
}

bool EntityManager::LoadAllTextures()
{
	for (ListItem<Entity *> *item = entities.start; item != nullptr; item = item->next)
	{
		if (!item->data->active) continue;
		item->data->AddTexturesAndAnimationFrames();
	}
	return true;
}

bool EntityManager::Update(float dt)
{
	for(ListItem<Entity *> *item = entities.start; item != nullptr; item = item->next)
	{
		if(!item->data->active) continue;
		if(!item->data->Update()) return false;
	}
	return true;
}