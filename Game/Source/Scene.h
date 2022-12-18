#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"

struct SDL_Texture;

struct BGInfo
{
	SDL_Texture *texture;
	fPoint position;
	float increase;
	fPoint size;
};

class Scene : public Module
{
public:

	Scene();

	// Destructor
	~Scene() final;

	// Called before render is available
	bool Awake(pugi::xml_node& config) final;

	// Called before the first frame
	bool Start() final;

	// Called before all Updates
	bool PreUpdate() final;

	// Called each loop iteration
	bool Update(float dt) final;

	// Called before all Updates
	bool PostUpdate() final;

	// Called before quitting
	bool CleanUp() final;

	iPoint bgPosition = {0, 0};
	float bgSpeed = 0.2f;
	float bgScale = 1.0f;
	std::vector<BGInfo> background;
	std::unordered_map<std::string, std::pair<std::string, int>, StringHash, std::less_equal<>> backgroundInfo;
};

#endif // __SCENE_H__