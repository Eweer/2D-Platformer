#ifndef __APP_H__
#define __APP_H__

#include "Module.h"

#include <list>
#include "List.h"

#include <memory>

#include "PugiXml/src/pugixml.hpp"

constexpr auto CONFIG_FILENAME = "config.xml";
constexpr auto SAVE_STATE_FILENAME = "save_game.xml";

// Modules
class Window;
class Input;
class Render;
class Textures;
class Audio;
class Scene;
class EntityManager;
class Map;
//L07 DONE 2: Add Physics module
class Physics;

class App
{
public:

	// Constructor
	App(int argc, char* args[]);

	// Destructor
	virtual ~App();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	// Add a new module to handle
	void AddModule(Module* mod);

	// Exposing some properties for reading
	int GetArgc() const;
	const char* GetArgv(int index) const;
	std::string GetTitle() const;
	std::string GetOrganization() const;

	// L03: DONE 1: Create methods to control that the real Load and Save happens at the end of the frame
	void LoadGameRequest();
	void SaveGameRequest() ;
	bool LoadFromFile();
	bool SaveToFile();
	
	uint GetLevelNumber() const;

private:

	// Load config file
	bool LoadConfig();

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

public:

	// Modules
	std::unique_ptr<Window> win;
	std::unique_ptr<Input> input;
	std::unique_ptr<Render> render;
	std::unique_ptr<Textures> tex;
	std::unique_ptr<Audio> audio;
	std::unique_ptr<Scene> scene;
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<Map> map;
	std::unique_ptr<Physics> physics;

private:

	int argc;
	char** args;
	std::string title;
	std::string organization;

	std::list<Module*> modules;
	
	pugi::xml_document configFile;
	pugi::xml_node configNode;

	uint frames = 0;
	float dt;

	bool saveGameRequested;
	bool loadGameRequested;

	uint levelNumber = 1;
};

extern std::unique_ptr<App> app;

#endif	// __APP_H__