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

		bgSpeed = elem.attribute("speed").as_float();
	}

	// Instantiate the player using the entity manager
	if (config.child("player"))
		app->entityManager->CreateEntity("player", config.child("player"));

	for(auto const &elem : config.children("enemy"))
		for(auto const &id : elem.children("spawn"))
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
			
			background.emplace_back(BGInfo(app->tex->Load(filePath.c_str()), {0,0}, bgSpeed * i, fPoint(0,0)));

			uint w;
			uint h;
			app->tex->GetSize(background.back().texture, w, h);
			background.back().size.x = static_cast<float>(w);
		}
	}

	if(!background.empty())
	{
		uint w;
		uint h;
		app->tex->GetSize(background.back().texture, w, h);
		bgScale = static_cast<float>(app->win->GetHeight()) / static_cast<float>(h);
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


bool Scene::Pause(int phase)
{
	for(auto &elem : background)
	{
		app->render->DrawBackground(elem.texture, elem.position + 2 - elem.size * bgScale, bgScale);
		app->render->DrawBackground(elem.texture, elem.position, bgScale);
		app->render->DrawBackground(elem.texture, elem.position - 2 + elem.size * bgScale, bgScale);
	}

	// Request App to Load / Save when pressing the keys F5 (save) / F6 (load)
	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		app->SaveGameRequest();

	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		app->LoadGameRequest();

	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	for(auto &elem : background)
	{
		app->render->DrawBackground(elem.texture, elem.position + 2 - elem.size * bgScale, bgScale);
		app->render->DrawBackground(elem.texture, elem.position, bgScale);
		app->render->DrawBackground(elem.texture, elem.position - 2 + elem.size * bgScale, bgScale);
		elem.position.x -= (elem.increase * additionalSpeed) + elem.increase;

		if(elem.position.x <= elem.size.x * (-1 * bgScale))
		{
			elem.position.x = 0;
		}
	}

	additionalSpeed = 0.0f;

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

void Scene::IncreaseBGScrollSpeed(float x)
{
	if(x >= 3.0f) additionalSpeed = 3.0f;
	else if(x <= -3.0f) additionalSpeed = -3.0f;
	else additionalSpeed = x;

	if(additionalSpeed == 0) additionalSpeed = 0.1f;
}

bool Scene::HasSaveData() const
{
	return true;
}

bool Scene::LoadState(pugi::xml_node const &data)
{
	return false;
}

pugi::xml_node Scene::SaveState(pugi::xml_node const &data) const
{
	std::string saveData2 = "<{} {}=\"{}\"/>\n";
	std::string saveOpenData2 = "<{} {}=\"{}\">\n";
	std::string saveData4 = "<{} {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveOpenData4 = "<{} {}=\"{}\" {}=\"{}\">\n";
	std::string saveData6 = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveData6OneFloat = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\" {}=\"{:.2f}\"/>\n";
	std::string saveData6OneInt = "<{} {}=\"{}\" {}=\"{:.2f}\" {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string saveFloatData = "<{} {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string dataToSave = "<scene>\n";
	for (int i = 1; auto const &elem : background)
	{
		dataToSave += AddSaveData(
			saveData6OneInt,
			"layer",
			"id", i,
			"x", elem.position.x,
			"y", elem.position.y,
			"additionalspeed", additionalSpeed
		);
		i++;
	}
	dataToSave += "</scene>";

	app->AppendFragment(data, dataToSave.c_str());

	return data;
}
