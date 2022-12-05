#include "App.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Scene.h"
#include "EntityManager.h"
#include "Map.h"
#include "Physics.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <sstream>

#include <memory>

// Constructor
App::App(int argc, char* args[]) : argc(argc), args(args)
{
	input = std::make_unique<Input>();
	win = std::make_unique<Window>();
	render = std::make_unique<Render>();
	tex = std::make_unique<Textures>();
	audio = std::make_unique<Audio>();
	physics = std::make_unique<Physics>();
	scene = std::make_unique<Scene>();
	entityManager = std::make_unique<EntityManager>();
	map = std::make_unique<Map>();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(input.get());
	AddModule(win.get());
	AddModule(tex.get());
	AddModule(audio.get());
	AddModule(physics.get());
	AddModule(scene.get());
	AddModule(entityManager.get());
	AddModule(map.get());

	// Render last to swap buffer
	AddModule(render.get());
}

// Destructor
App::~App() = default;

void App::AddModule(Module* mod)
{
	mod->Init();
	modules.push_back(mod);
}

// Called before render is available
bool App::Awake()
{
	// L01: DONE 3: Load config from XML
	if(!LoadConfig()) return false;

	title = configNode.child("app").child("title").child_value(); 

	for(auto const &item : modules)
	{
		if(pugi::xml_node node = configNode.child(item->name.c_str());
		   !item->Awake(node))
		{
			return false;
		}
	}

	return true;
}

// Called before the first frame
bool App::Start()
{
	for(auto const &item : modules)
	{
		if(!item->Start()) return false;
	}

	return true;
}

// Called each loop iteration
bool App::Update()
{
	PrepareUpdate();

	if (input->GetWindowEvent(WE_QUIT))
		return false;
	if(!PreUpdate())
		return false;
	if(!DoUpdate())
		return false;
	if(!PostUpdate())
		return false;

	FinishUpdate();
	return true;
}

// Load config from XML file
bool App::LoadConfig()
{
	if (pugi::xml_parse_result parseResult = configFile.load_file("config.xml"); 
		parseResult) 
	{
		configNode = configFile.child("config");
	}
	else
	{
		LOG("Error in App::LoadConfig(): %s", parseResult.description());
		return false;
	}

	return true;
}

// ---------------------------------------------
void App::PrepareUpdate()
{
}

// ---------------------------------------------
void App::FinishUpdate()
{
	// L03: DONE 1: This is a good place to call Load / Save methods
	if (loadGameRequested) LoadFromFile();
	if (saveGameRequested) SaveToFile();
}

// Call modules before each loop iteration
bool App::PreUpdate()
{
	for(auto const &item : modules)
	{
		if(!item->active) continue;
		if(!item->PreUpdate()) return false;
	}

	return true;
}

// Call modules on each loop iteration
bool App::DoUpdate()
{
	for(auto const &item : modules)
	{
		if(!item->active) continue;
		if(!item->Update(dt)) return false;
	}

	return true;
}

// Call modules after each loop iteration
bool App::PostUpdate()
{
	for(auto const &item : modules)
	{
		if(!item->active) continue;
		if(!item->PostUpdate()) return false;
	}

	return true;
}

// Called before quitting
bool App::CleanUp()
{
	for(auto const &item : modules)
		if(!item->CleanUp()) return false;
	
	return true;
}

// ---------------------------------------
int App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* App::GetArgv(int index) const
{
	if (index < argc)
		return args[index];
	else
		return nullptr;
}

// ---------------------------------------
std::string App::GetTitle() const
{
	return title;
}

// ---------------------------------------
std::string App::GetOrganization() const
{
	return organization;
}

// L02: DONE 1: Implement methods to request load / save and methods 
// for the real execution of load / save (to be implemented in TODO 5 and 7)
void App::LoadGameRequest()
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist
	loadGameRequested = true;
}

// ---------------------------------------
void App::SaveGameRequest() 
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist and... should we overwriten
	saveGameRequested = true;
}


// L02: DONE 5: Implement the method LoadFromFile() to actually load a xml file
// then call all the modules to load themselves
bool App::LoadFromFile()
{
	pugi::xml_document gameStateFile;

	if (pugi::xml_parse_result result = gameStateFile.load_file("save_game.xml"); !result)
	{
		LOG("Could not load xml file savegame.xml. pugi error: %s", result.description());
		return false;
	}
	
	for(auto const &item : modules)
	{
		if(const auto moduleName = item->name.c_str();
		   !item->LoadState(gameStateFile.child("save_state").child(moduleName)))
		{
			return false;
		}
	}

	return !(loadGameRequested = false);
}

// L02: DONE 7: Implement the xml save method SaveToFile() for current state
// check https://pugixml.org/docs/quickstart.html#modify
bool App::SaveToFile() 
{
	auto saveDoc = std::make_unique<pugi::xml_document>();
	pugi::xml_node saveStateNode = saveDoc->append_child("save_state");
	for(auto const &item : modules)
	{
		if(item->HasSaveData())
		{
			if(auto ret = item->SaveState(saveStateNode); !ret.empty())
				saveStateNode.child("save_state").append_copy(ret);
			else
				LOG("Error saving state of module %s", item->name.c_str());
		}
	}
	saveGameRequested = saveDoc->save_file("save_game.xml");

	return !saveGameRequested;
}

uint App::GetLevelNumber() const
{
	return levelNumber;
}
