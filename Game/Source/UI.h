#ifndef __UI_H__
#define __UI_H__

#include "Module.h"
#include "Entity.h"
#include "Player.h"

#include "Defs.h"
#include "Point.h"

#include <unordered_map>

class UI : public Module
{
public:

	UI();

	~UI() final;

	// Called before render is available
	bool Awake(pugi::xml_node &) final;

	// Called before the first frame
	bool Start() final;

	// Called each loop iteration
	bool PreUpdate() final;

	// Called each loop iteration
	bool PostUpdate() final;

	bool Pause(int phase) final;

	bool TogglePauseDraw();
	bool ToggleSavingIcon();

private:
	void DrawPause(iPoint &position) const;
	void DrawFPS(iPoint &position) const;
	void DrawGravity(iPoint &position) const;
	void DrawPlayerPosition(iPoint &position) const;
	void DrawUIPosition(iPoint &position) const;
	void DrawCameraPosition(iPoint &position) const;
	void DrawMousePosition(iPoint &position) const;
	void DrawPlayerJumps(iPoint &position) const;
	void DrawPlayerAnimation(iPoint &position) const;
	void DrawSaving(iPoint &position);
	void DrawSavingCheck(iPoint &position);
	void DrawPlayerSkill(iPoint &position) const;

	int IncreaseY(int font) const;

	std::unordered_map<std::string, SDL_Texture *, StringHash, std::less_equal<>> uiElements;

	float degree = 0.0f;
	int laps = 0;

	bool bSavingGame = false;
	bool bDrawPause = false;
	int fCleanCraters = 0;
	iPoint pTopLeft = {10, 10};
	iPoint pMiddle = {0, 0};
	iPoint pBottomLeft = {0,0};

	pugi::xml_node parameters;
};

#endif
