#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Map.h"
#include "BitMaskColliderLayers.h"

#include "Defs.h"
#include "Log.h"

#include <format>

Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene() = default;

// Called before render is available
bool Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	// Instantiate the player using the entity manager
	if (config.child("player")) {
		app->entityManager->CreateEntity("player", config.child("player"));
	}

	return ret;
}

// Called before the first frame
bool Scene::Start()
{	
	// Load map
	app->map->Load();

	// Set the window title with map/tileset info
	std::string title = std::format(
		"Map:{}x{} Tiles:{}x{} Tilesets:{}",
		std::to_string(app->map->GetWidth()),
		std::to_string(app->map->GetHeight()),
		std::to_string(app->map->GetTileWidth()),
		std::to_string(app->map->GetTileHeight()),
		std::to_string(app->map->GetTileSetSize())
	); 

	app->win->SetTitle(title.c_str());

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	// Request App to Load / Save when pressing the keys F5 (save) / F6 (load)
	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		app->SaveGameRequest();

	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		app->LoadGameRequest();

	// Draw map
	app->map->Draw();
	
	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}
