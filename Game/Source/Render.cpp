#include "App.h"
#include "Window.h"
#include "Render.h"
#include "EntityManager.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <string>
#include <array>

Render::Render() : Module()
{
	name = "renderer";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render() = default;

// Called before render is available
bool Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if (vSyncActive = config.child("vsync").attribute("value").as_bool(true); vSyncActive)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(app->win->GetWindow(), -1, flags);

	if(renderer)
	{
		camera.w = app->win->GetSurface()->w;
		camera.h = app->win->GetSurface()->h;
		camera.x = 0;
		camera.y = 0;
	}
	else
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	if(renderer) SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

bool Render::DrawCharacterTexture(SDL_Texture *texture, iPoint const &pos, const bool flip, SDL_Point pivot, const iPoint offset, const double angle)
{
	SDL_Rect rect{0};

	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	rect.x = pos.x + camera.x;
	rect.y = pos.y + camera.y;

	if(flip)
	{
		rect.x -= pivot.x;
	}

	if(offset != iPoint(INT_MAX, INT_MAX))
	{
		rect.x -= offset.x;
		rect.y -= offset.y;
	}

	SDL_Point const *p = nullptr;

	if(pivot.x != INT_MAX && pivot.y != INT_MAX)
	{
		SDL_Point sdlPivot{pivot.x, pivot.y};
		p = &sdlPivot;
	}
		
	if(SDL_RenderCopyEx(renderer, texture, nullptr, &rect, angle, p, (SDL_RendererFlip)flip) == -1)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		return false;
	}

	return true;
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY, SDL_RendererFlip flip) const
{
	uint scale = app->win->GetScale();

	SDL_Rect rect = {
		.x = static_cast<int>((static_cast<float>(camera.x) * speed)) + x * static_cast<int>(scale),
		.y = static_cast<int>((static_cast<float>(camera.y) * speed)) + y * static_cast<int>(scale),
		.w = 0,
		.h = 0
	};

	if(section != nullptr)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);
	}

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point const *p = nullptr;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		SDL_Point pivot{ pivotX, pivotY };
		p = &pivot;
	}

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
{
	bool ret = true;
	[[maybe_unused]] uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	std::array<SDL_Point, 360> points{};

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = camera.x + x + (int)((float)radius * cos((float)i * factor));
		points[i].y = camera.y + y + (int)((float)radius * sin((float)i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points.data(), 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

// L03: DONE 6: Implement a method to load the state
// for now load camera's x and y
bool Render::LoadState(pugi::xml_node const &data)
{
	camera.x = data.child("camera").attribute("x").as_int();
	camera.y = data.child("camera").attribute("y").as_int();

	vSyncOnRestart = data.child("graphics").attribute("vsync").as_bool();
	fpsTarget = data.child("graphics").attribute("targetfps").as_uint();

	return true;
}

pugi::xml_node Render::SaveState(pugi::xml_node const &data) const
{
	pugi::xml_node cam = data;
	cam = cam.append_child("renderer");

	cam.append_child("camera").append_attribute("x").set_value(camera.x);
	cam.child("camera").append_attribute("y").set_value(camera.y);
	
	cam.append_child("graphics").append_attribute("vsync").set_value(vSyncOnRestart ? "true" : "false");
	cam.child("graphics").append_attribute("targetfps").set_value(std::to_string(fpsTarget).c_str());
	

	return cam;

}

bool Render::HasSaveData() const
{
	return true;
}
