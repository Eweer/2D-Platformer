#include "App.h"
#include "Render.h"
#include "Textures.h"

#include "Defs.h"
#include "Log.h"

#include <functional>
#include <list>
#include <memory>

#include "SDL_image/include/SDL_image.h"
//#pragma comment(lib, "../Game/Source/External/SDL_image/libx86/SDL2_image.lib")

Textures::Textures() : Module()
{
	name = "textures";
}

// Destructor
Textures::~Textures() = default;

// Called before render is available
bool Textures::Awake(pugi::xml_node& config)
{
	LOG("Init Image library");

	// Load support for the PNG image format
	if(int flags = IMG_INIT_PNG; (IMG_Init(flags) & flags) != flags)
	{
		LOG("Could not initialize Image lib. IMG_Init: %s", IMG_GetError());
		return false;
	}

	return true;
}

// Called before the first frame
bool Textures::Start()
{
	LOG("Start textures");
	return true;
}

// Called before quitting
bool Textures::CleanUp()
{
	LOG("Freeing textures and Image library");

	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture* Textures::Load(const char* path)
{
	SDL_Texture* texture = nullptr;

	if(SDL_Surface* surface = IMG_Load(path)) 
	{
		texture = LoadSurface(surface);
		SDL_FreeSurface(surface);
	}	
	else LOG("Could not load surface with path: %s. IMG_Load: %s", path, IMG_GetError());

	return texture;
}


// Translate a surface into a texture
SDL_Texture* Textures::LoadSurface(SDL_Surface* surface)
{
	if(auto texturePtr = app->render->LoadTexture(surface);
		texturePtr != nullptr)
	{
		textures.emplace_back(std::move(texturePtr));
	}
	else LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());

	return textures.back().get();
}

// Unload texture
bool Textures::UnLoad(SDL_Texture const *texture)
{
	for(auto it = textures.begin(); it != textures.end(); it++)
	{
		if ((*it).get() == texture)
		{
			(*it).reset();
			textures.erase(it);
			texture = nullptr;
			return true;
		}
	}

	return false;
}

// Retrieve size of a texture
void Textures::GetSize(SDL_Texture* const texture, uint &width, uint &height) const
{
	SDL_QueryTexture(texture, nullptr, nullptr, (int*) &width, (int*) &height);
}
