#include "App.h"
#include "Window.h"
#include "Render.h"
#include "EntityManager.h"
#include "Input.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <string>
#include <array>

constexpr auto ticks_for_next_frame = (1000 / 60);
constexpr auto fps_UI_seconds_interval = 1.0f;

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

	if (vSyncActive = config.child("vsync").attribute("value").as_bool(); 
		vSyncActive)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	vSyncOnRestart = vSyncActive;

	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>>renderPtr(
		SDL_CreateRenderer(app->win->GetWindow(), -1, flags),
		[](SDL_Renderer *r) { if(r) SDL_DestroyRenderer(r); }
	);
	
	renderer = std::move(renderPtr);
		
	if(!renderer)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	camera = {
			.x = 0,
			.y = 0,
			.w = app->win->GetSurface()->w,
			.h = app->win->GetSurface()->h
		};
	
	ticksForNextFrame = 1000/fpsTarget;

	return true;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// Background
	SDL_RenderGetViewport(renderer.get(), &viewport);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	if(!vSyncActive)
	{
		while(SDL_GetTicks() - renderLastTime < ticksForNextFrame)
		{
			SDL_Delay(1);
		}
	}
	
	SDL_RenderClear(renderer.get());
	return true;
}

bool Render::Update(float dt)
{
	if(app->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN)
	{
		vSyncOnRestart = !vSyncOnRestart;
	}
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer.get(),
						   background.r,
						   background.g,
						   background.g,
						   background.a
	);
	
	SDL_RenderPresent(renderer.get());
	
	// I -> increases fps target || O ->decreases fps target
	if(app->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN && fpsTarget < 1000)
	{
		fpsTarget += 10;
		ticksForNextFrame = 1000/fpsTarget;
	}
	if(app->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN && fpsTarget > 10)
	{
		fpsTarget -= 10;
		ticksForNextFrame = 1000/fpsTarget;
	}
	
	if(!vSyncActive) renderLastTime = SDL_GetTicks();
	
	fpsTimer++;
	if(SDL_GetTicks() - fpsTimer > static_cast<uint32>(fps_UI_seconds_interval * 1000))
	{
		fps = fpsTimer;
		fpsTimer = SDL_GetTicks();
	}
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect) const
{
	SDL_RenderSetViewport(renderer.get(), &rect);
}

void Render::ResetViewPort() const
{
	SDL_RenderSetViewport(renderer.get(), &viewport);
}

bool Render::DrawCharacterTexture(SDL_Texture *texture, iPoint const &pos, const bool flip, SDL_Point pivot, const iPoint offset, const double angle) const
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
		
	if(SDL_RenderCopyEx(renderer.get(), texture, nullptr, &rect, angle, p, (SDL_RendererFlip)flip) == -1)
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

	if(SDL_RenderCopyEx(renderer.get(), texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawRectangle(const SDL_Rect& rect, SDL_Color color, bool filled, bool use_camera, SDL_BlendMode blendMode) const
{
	SDL_SetRenderDrawBlendMode(renderer.get(), blendMode);
	SDL_SetRenderDrawColor(renderer.get(), color.r, color.g, color.b, color.a);

	SDL_Rect rec(rect);
	
	if(use_camera)
	{	
		uint scale = app->win->GetScale();
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	

	if(int result = filled ? SDL_RenderFillRect(renderer.get(), &rec) : SDL_RenderDrawRect(renderer.get(), &rec); 
	   result)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer.get(), r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer.get(), camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer.get(), x1 * scale, y1 * scale, x2 * scale, y2 * scale);

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

	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer.get(), r, g, b, a);

	int result = -1;
	std::array<SDL_Point, 360> points{};

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = camera.x + x + (int)((float)radius * cos((float)i * factor));
		points[i].y = camera.y + y + (int)((float)radius * sin((float)i * factor));
	}

	result = SDL_RenderDrawPoints(renderer.get(), points.data(), 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

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
