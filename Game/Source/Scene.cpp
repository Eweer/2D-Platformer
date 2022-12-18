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
#include "Player.h"
#include "Pathfinding.h"

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

	for(auto const &elem : config.children("background"))
	{
		backgroundInfo[elem.attribute("map").as_string()] =
			std::make_pair<std::string, int>(
				elem.attribute("path").as_string(),
				elem.attribute("frames").as_int()
			);
	}

	// Instantiate the player using the entity manager
	if (config.child("player"))
		app->entityManager->CreateEntity("player", config.child("player"));

	for(auto const &elem : config.children("enemy"))
		app->entityManager->CreateEntity("enemy", elem);

	return true;
}

// Called before the first frame
bool Scene::Start()
{	
	// Load map
	if(app->map->Load()) app->pathfinding->SetWalkabilityMap();

	// Load background
	if(auto it = backgroundInfo.find("Mountain");
	   it != backgroundInfo.end())
	{
		std::string bgFolder = it->second.first;
		int frames = it->second.second;

		for(int i = 1; i <= frames; i++)
		{
			std::string filePath = std::format("{}{}.png", bgFolder, i);
			
			background.emplace_back(BGInfo(app->tex->Load(filePath.c_str()), {0,0}, 0.2f * i, fPoint(0,0)));

			uint w;
			uint h;
			app->tex->GetSize(background.back().texture, w, h);
			background.back().size.x = static_cast<float>(w);
		}
	}

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
	for(auto &elem : background)
	{
		app->render->DrawBackground(elem.texture, elem.position - elem.size * 2.0f, 2.0f);
		app->render->DrawBackground(elem.texture, elem.position, 2.0f);
		app->render->DrawBackground(elem.texture, elem.position + elem.size * 2.0f, 2.0f);
		elem.position.x -= elem.increase;

		if(elem.position.x <= elem.size.x * -2.0f)
		{
			elem.position.x = 0;
		}
	}

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
	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		return false;
	
	return true;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}
