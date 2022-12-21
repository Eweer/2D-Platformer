#ifndef __UI_H__
#define __UI_H__

#include "Module.h"
#include "Entity.h"
#include "Player.h"

#include "Defs.h"
#include "Point.h"

#include <unordered_map>

struct SharedBar
{
	std::shared_ptr<SDL_Texture> bg;
	std::shared_ptr<SDL_Texture> border;
	std::shared_ptr<SDL_Texture> borderfull;
};

struct Bar
{
	void DrawBar(iPoint position, int value = 100, float scale = 4.0f) const
	{
		uint w = 0;
		uint h = 0;
		app->tex->GetSize(common->border.get(), w, h);
		position.y -= static_cast<int>(static_cast<float>(h) * (scale/1.5));

		auto increase = static_cast<int>(4.0f * scale);
		app->render->DrawImage(common->bg.get(), iPoint(position.x, position.y + increase), scale);
		app->render->DrawImage(common->border.get(), position, scale);
		position = position + increase;
		app->render->DrawImage(left.get(), position, scale);
		position.x += increase;
		for(int i = 0; i < value && i < 100; i++, position.x += static_cast<int>(scale))
		{
			app->render->DrawImage(point.get(), position, scale);
		}
		app->render->DrawImage(right.get(), position, scale);
	}

	std::shared_ptr<SDL_Texture> left;
	std::shared_ptr<SDL_Texture> right;
	std::shared_ptr<SDL_Texture> point;
	std::shared_ptr<SDL_Texture> full;
	std::shared_ptr<SharedBar> common;
};

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

	bool HasSaveData() const final;
	bool LoadState(pugi::xml_node const &data) final;
	pugi::xml_node SaveState(pugi::xml_node const &) const final;

private:

	void DrawPlayerHP(iPoint &position);
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

	std::unordered_map<std::string, std::shared_ptr<SDL_Texture>, StringHash, std::less_equal<>> uiElements;

	float degree = 0.0f;
	int laps = 0;

	bool bDrawUI = false;
	bool bSavingGame = false;
	bool bDrawPause = false;
	int fCleanCraters = 0;
	iPoint pTopLeft = {10, 10};
	iPoint pMiddle = {0, 0};
	iPoint pBottomRight = {0,0};
	iPoint pBottomLeft = {0,0};

	std::shared_ptr<SharedBar> commonBars;
	
	std::unordered_map<std::string, Bar, StringHash, std::equal_to<>> bars;

	pugi::xml_node parameters;
};

#endif
