#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Module.h"
#include "Defs.h"

struct SDL_Window;
struct SDL_Surface;

class Window : public Module
{
public:

	Window();

	// Destructor
	~Window() final;

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called before quitting
	bool CleanUp() final;

	// Change title
	void SetTitle(std::string const &title);

	SDL_Window *GetWindow() const;

	SDL_Surface *GetSurface() const;

	// Retrive window size
	void GetWindowSize(uint &w, uint &h) const;

	// Retrieve window scale
	uint GetScale() const;

private:
	// The window we'll be rendering to
	SDL_Window* window = nullptr;

	// The surface contained by the window
	SDL_Surface* screenSurface = nullptr;

	std::string title = "";
	uint width;
	uint height;
	uint scale;
};

#endif // __WINDOW_H__
