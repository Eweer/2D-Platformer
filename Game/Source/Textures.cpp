#include "App.h"
#include "Render.h"
#include "Textures.h"

#include "Defs.h"
#include "Log.h"

#include "SDL_image/include/SDL_image.h"
//#pragma comment(lib, "../Game/Source/External/SDL_image/libx86/SDL2_image.lib")

Textures::Textures() : Module()
{
	name = "textures";
}

// Destructor
Textures::~Textures()
{}

// Called before render is available
bool Textures::Awake(pugi::xml_node& config)
{
	LOG("Init Image library");

	// Load support for the PNG image format
	int flags = IMG_INIT_PNG;
	int init = IMG_Init(flags);

	if((init & flags) != flags)
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
	
	for (auto const &item : textures)
	{
		SDL_DestroyTexture(item);
	}
	
	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture* const Textures::Load(const char* path)
{
	SDL_Texture* texture = nullptr;
	SDL_Surface* surface = IMG_Load(path);

	if(surface) 
	{
		texture = LoadSurface(surface);
		SDL_FreeSurface(surface);
	}	
	else LOG("Could not load surface with path: %s. IMG_Load: %s", path, IMG_GetError());

	return texture;
}


// Translate a surface into a texture
SDL_Texture* const Textures::LoadSurface(SDL_Surface* surface)
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(app->render->renderer, surface);

	if(texture) textures.push_back(texture);
	else LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());

	return texture;
}

// Unload texture
bool Textures::UnLoad(SDL_Texture *texture)
{
	for (std::list<SDL_Texture *>::iterator it = textures.begin(); it != textures.end();)
	{
		if (*it == texture)
		{
			SDL_DestroyTexture(*it);
			it = textures.erase(it);
			return true;
		}
	}

	return false;
}

// Retrieve size of a texture
void Textures::GetSize(const SDL_Texture* texture, uint& width, uint& height) const
{
	SDL_QueryTexture((SDL_Texture*)texture, nullptr, nullptr, (int*) &width, (int*) &height);
}
