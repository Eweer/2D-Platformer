#ifndef __RENDER_H__
#define __RENDER_H__

#include "Module.h"

#include "Point.h"

#include "SDL/include/SDL.h"

class Render : public Module
{
public:

	Render();

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();


	// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = nullptr, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX, SDL_RendererFlip flip = SDL_FLIP_NONE) const;
	bool DrawCharacterTexture(SDL_Texture *texture, const iPoint pos, const bool flip = false, SDL_Point pivot = SDL_Point(INT_MAX, INT_MAX), const iPoint offset = iPoint(INT_MAX, INT_MAX), const double angle = 0);
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	// L03: DONE 6: Declare Load / Save methods
	bool LoadState(pugi::xml_node const &) final;
	pugi::xml_node SaveState(pugi::xml_node const &) const final;

	bool HasSaveData() const final;

	SDL_Renderer* renderer;
	SDL_Rect camera;
	SDL_Rect viewport;
	SDL_Color background;
	
	uint fpsTarget = 60;

	bool vSyncActive = true;
	bool vSyncOnRestart = true;
};

#endif // __RENDER_H__