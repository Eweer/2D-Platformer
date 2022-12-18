#ifndef __UI_H__
#define __UI_H__

#include "Module.h"
#include "Entity.h"
#include "Player.h"

#include "Defs.h"
#include "Point.h"

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

	void DrawPlayerSkill(iPoint &position) const;

	bool Pause(int phase) final;

	bool TogglePauseDraw();

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

	int IncreaseY(int font) const;

	bool bDrawPause = false;
	int fCleanCraters = 0;
	iPoint pTopLeft = {10, 10};
	iPoint pMiddle = {0, 0};
	iPoint pBottomLeft = {0,0};

};

#endif
