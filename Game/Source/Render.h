#ifndef __RENDER_H__
#define __RENDER_H__

#include "Module.h"
#include "Defs.h"
#include "Point.h"

#include <memory>
#include <functional>

#include "SDL/include/SDL.h"

class Render : public Module
{
public:

	Render();

	// Destructor
	~Render() final;

	// Called before render is available
	bool Awake(pugi::xml_node &) final;

	// Called before the first frame
	bool Start() final;

	// Called each loop iteration
	bool PreUpdate() final;
	bool Update(float dt) final;
	bool PostUpdate() final;

	// Called before quitting
	bool CleanUp() final;

	// Drawing
	bool DrawTexture(
		SDL_Texture *texture,
		int x,
		int y,
		const SDL_Rect *section = nullptr,
		float speed = 1.0f,
		double angle = 0,
		int pivotX = INT_MAX,
		int pivotY = INT_MAX,
		SDL_RendererFlip flip = SDL_FLIP_NONE
	) const;

	bool DrawCharacterTexture(
		SDL_Texture *texture,
		iPoint const &pos,
		const bool flip = false,
		SDL_Point pivot = SDL_Point(INT_MAX, INT_MAX),
		const iPoint offset = iPoint(INT_MAX, INT_MAX),
		const double angle = 0
	) const;

	bool DrawRectangle(
		const SDL_Rect &rect,
		SDL_Color color,
		bool filled = true,
		bool useCamera = true,
		SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_BLEND
	) const;

	bool DrawLine(
		iPoint v1,
		iPoint v2,
		SDL_Color color,
		bool use_camera = true,
		SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_BLEND
	) const;

	bool DrawCircle(
		iPoint center,
		int radius,
		SDL_Color color,
		bool use_camera = true,
		SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_BLEND
	) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	bool LoadState(pugi::xml_node const &) final;
	pugi::xml_node SaveState(pugi::xml_node const &) const final;

	bool HasSaveData() const final;

	std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>> LoadTexture(SDL_Surface *surface);

	SDL_Rect GetCamera() const;

private:

	void SetViewPort(const SDL_Rect &rect) const;
	void ResetViewPort() const;

	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>> renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;

	// -------- Vsync
	bool vSyncActive = true;
	bool vSyncOnRestart = true;

	// -------- No Vsync
	// Max fps we want to achieve
	uint32 fpsTarget = 60;
	
	// -------- Required for capping FPS
	// Delay required in ms to get the fps target
	uint32 ticksForNextFrame = 0;
	// Last tick in which we updated render
	uint32 renderLastTime = 0;
	
	// -------- Required for showing FPS on screen
	// Current FPS
	uint32 fps = 0;
	// Last tick in which we updated the current fps
	uint32 fpsTimer = 0;
};

#endif // __RENDER_H__
