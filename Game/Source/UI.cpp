#include "App.h"
#include "UI.h"
#include "Textures.h"
#include "Fonts.h"
#include "Render.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Player.h"
#include "Textures.h"
#include "Input.h"

#include "Defs.h"
#include "Point.h"

#include <format>
#include <unordered_map>
#include <filesystem>

UI::UI() : Module()
{
	name = "ui";
}

UI::~UI() = default;

// Called before render is available
bool UI::Awake(pugi::xml_node &config)
{
	parameters = config;
	
	return true;
}

// Called before the first frame
bool UI::Start()
{
	commonBars = std::make_shared<SharedBar>(
		app->tex->Load((std::string(parameters.child("bar").attribute("path").as_string()) + std::string("/bg.png")).c_str()),
		app->tex->Load((std::string(parameters.child("bar").attribute("path").as_string()) + std::string("/border.png")).c_str()),
		app->tex->Load((std::string(parameters.child("bar").attribute("path").as_string()) + std::string("/borderfull.png")).c_str())
	);

	for(auto const &entry : std::filesystem::directory_iterator(parameters.child("bar").attribute("path").as_string()))
	{
		if(!entry.is_directory()) continue;
		
		auto path = entry.path().string();

		if(path == "." || path == "..") continue;

		Bar b =
		{
			.left = app->tex->Load((path + std::string("/left.png")).c_str()),
			.right = app->tex->Load((path + std::string("/right.png")).c_str()),
			.point = app->tex->Load((path + std::string("/point.png")).c_str()),
			.full = app->tex->Load((path + std::string("/full.png")).c_str()),
			.common = commonBars
		};
		bars[entry.path().filename().string()] = b;
	}

	for(auto const &elem : parameters.children())
	{
		if(StrEquals(elem.name(), "bar") || StrEquals(elem.name(), "path")) continue;
		uiElements[elem.name()] = app->tex->Load(std::string(elem.attribute("path").as_string()).c_str());
	}
	fCleanCraters = app->fonts->Load("CleanCraters");
	return true;
}

// Called each loop iteration
bool UI::PreUpdate()
{
	// Return all coordinates to their original values
	pTopLeft = {10, 10};

	pBottomRight = {app->render->viewport.w - 10, app->render->viewport.h - 10};
	
	pBottomLeft = {10, app->render->viewport.h - 40};

	pMiddle = {app->render->viewport.w / 2, app->render->viewport.h / 2};

	return true;
}

// Called each loop iteration
bool UI::PostUpdate()
{
	if(bDrawUI)
	{
		DrawFPS(pTopLeft);
		DrawGravity(pTopLeft);
		DrawMousePosition(pTopLeft);
		DrawCameraPosition(pTopLeft);
		DrawUIPosition(pTopLeft);
		DrawPlayerPosition(pTopLeft);
		DrawPlayerJumps(pTopLeft);
		DrawPlayerAnimation(pTopLeft);
	}

	DrawPlayerHP(pBottomLeft);
	DrawPlayerSkill(pBottomRight);

	if(bDrawPause) DrawPause(pMiddle);

	if(bSavingGame) DrawSaving(pBottomRight);
	if(!bSavingGame && degree > 0) DrawSavingCheck(pBottomRight);

	if(app->input->GetKey(SDL_SCANCODE_F3) == KeyState::KEY_DOWN)
		bDrawUI = !bDrawUI;

	return true;
}

void UI::DrawPlayerHP(iPoint &position)
{
	
	if(auto it = bars.find("HP"); it != bars.end())
	{
		auto const &player = app->entityManager->player;
		if(player->bHurt)
		{
			auto frame = (player->texture->GetFloatCurrentFrame() != 0) ? player->texture->GetFloatCurrentFrame() : static_cast<float>(player->texture->GetFrameCount());
			auto lastFrame = static_cast<float>(player->texture->GetFrameCount());
			auto decrease = static_cast<int>(frame / lastFrame * static_cast<float>(player->damageTaken));
			int originalHP = player->hp + player->damageTaken;
			int hpToDraw = originalHP - decrease;
			it->second.DrawBar(position, hpToDraw);
		}
		else it->second.DrawBar(position, app->entityManager->player->hp);
	}
}

void UI::DrawSavingCheck(iPoint &position)
{
	degree--;
	if(degree == 0)
	{
		app->GameSaved();
		return;
	}
	uint h = 0;
	uint w = 0;

	if(auto it = uiElements.find("check");
	   it != uiElements.end())
	{
		app->tex->GetSize(it->second.get(), w, h);

		app->render->DrawTexture(
			it->second.get(),
			position.x + w/4+ app->render->camera.x * -1,
			position.y - h + app->render->camera.y * -1
		);
		position -= iPoint(w/4, h);
	}
}

void UI::DrawSaving(iPoint &position)
{
	degree += 360.0f * DEGTORAD;

	if(degree >= 360)
	{
		laps++;
		if(laps >= 2)
		{
			bSavingGame = false;
			degree = 120;
			laps = 0;
			return;
		}
		degree = 0;
	}
	

	if(auto it = uiElements.find("disk");
	   it != uiElements.end())
	{
		uint h = 0;
		uint w = 0;
		app->tex->GetSize(it->second.get(), w, h);
		app->render->DrawTexture(
			it->second.get(),
			position.x + static_cast<int>(static_cast<float>(w)*1.5f) + app->render->camera.x * -1,
			position.y - static_cast<int>(static_cast<float>(h)*1.5f) + app->render->camera.y * -1
		);
	}
	
	if(auto reIt = uiElements.find("recycle"); 
	   reIt != uiElements.end())
	{
		uint h = 0;
		uint w = 0;
		app->tex->GetSize(reIt->second.get(), w, h);

		app->render->DrawTexture(
			reIt->second.get(),
			position.x + w/6 + app->render->camera.x * -1,
			position.y - static_cast<int>(static_cast<float>(h)/1.2f) + app->render->camera.y * -1,
			nullptr,
			1.0f,
			degree
		);
	}
}

void UI::DrawPlayerSkill(iPoint &position) const
{
	const auto &player = app->entityManager->player;

	auto tex = (player->skillCDTimer == 0) ? player->texture->GetAnimationByName("skill") : player->texture->GetAnimationByName("skill_CD");
	
	if(!tex) return;

	uint w = 0;
	uint h = 0;
	app->tex->GetSize(tex.get(), w, h);
	position -= iPoint(w, h);
	app->render->DrawTexture(tex.get(), position.x + app->render->camera.x * -1, position.y + app->render->camera.y * -1);
	if(player->skillCDTimer > 0)
	{
		app->fonts->DrawMiddlePoint(
			std::format(
				"{:.1f}",
				(player->skillCD - player->skillCDTimer)/60.0f
			),
			position + iPoint(w/2, h/2),
			fCleanCraters,
			{2.0f, 2.0f}
		);
	}
	position.y -= IncreaseY(fCleanCraters);
}

void UI::DrawPause(iPoint &position) const
{
	app->fonts->DrawMiddlePoint("GAME PAUSED", position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawFPS(iPoint &position) const
{
	// Always Draw this
	app->fonts->Draw(std::format("Current FPS: {}", app->render->fps), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);

	if(!app->render->vSyncActive)
	{
		// Non-sense to Draw this if VSync is active
		app->fonts->Draw(std::format("Target FPS: {}", app->render->fpsTarget), position, fCleanCraters);
		position.y += IncreaseY(fCleanCraters);
	}

	// VSync state
	if(app->render->vSyncActive != app->render->vSyncOnRestart)
		app->fonts->Draw("Restart for VSync changes.", position, fCleanCraters);
	else
		app->fonts->Draw(std::format("VSync is {}.", app->render->vSyncActive ? "enabled" : "disabled"), position, fCleanCraters);
	
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawGravity(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Gravity: \"{},{}\"",
			app->physics->GetWorldGravity().x,
			app->physics->GetWorldGravity().y
		),
		position,
		fCleanCraters
	);
	position.y += IncreaseY(fCleanCraters);

}

void UI::DrawPlayerPosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Player position: \"{},{}\"",
			app->entityManager->player->position.x,
			app->entityManager->player->position.y
		),
		position,
		fCleanCraters
	);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawUIPosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"UI position: \"{},{}\"",
			position.x,
			position.y
		),
		position,
		fCleanCraters
	);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawCameraPosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Camera position: \"{},{}\"",
			app->render->camera.x,
			app->render->camera.y
		),
		position,
		fCleanCraters
	);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawMousePosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Mouse position: \"{},{}\"",
			app->input->mousePosition.x,
			app->input->mousePosition.y
		),
		position,
		fCleanCraters
	);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawPlayerJumps(iPoint &position) const
{
	const auto &jump = app->entityManager->player->jump;
	app->fonts->Draw(std::format("Max Jumps: {}", jump.maxJumps), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
	app->fonts->Draw(std::format("Is on air: {}", jump.bOnAir ? "Yes." : "No."), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
	app->fonts->Draw(std::format("Current Jumps: {}", jump.currentJumps), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
	app->fonts->Draw(std::format("Jump Impulse: {}", jump.jumpImpulse), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
}

void UI::DrawPlayerAnimation(iPoint &position) const
{
	const auto &player = app->entityManager->player;
	app->fonts->Draw(std::format("Able to move: {}", player->bAbleToMove ? "Yes." : "No."), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
	app->fonts->Draw(std::format("Locked Animation: {}", player->bLockAnim ? "Yes." : "No."), position, fCleanCraters);
	position.y += IncreaseY(fCleanCraters);
	const auto &velocity = (player->pBody) ? player->pBody->body->GetLinearVelocity() : b2Vec2(0,0);
	app->fonts->Draw(std::format("Current Veloicty: {:.1f}, {:.1f}", velocity.x, velocity.y), position, fCleanCraters);
}

int UI::IncreaseY(int font) const
{
	return app->fonts->fonts[font].lineHeight + app->fonts->fonts[font].spacing.y;
}

bool UI::Pause(int phase)
{
	switch(phase)
	{
		case 1:
			return PreUpdate();
		case 3:
			return PostUpdate();
		default:
			return true;
	}
}

bool UI::TogglePauseDraw()
{
	return bDrawPause = !bDrawPause;
}

bool UI::ToggleSavingIcon()
{
	return bSavingGame = !bSavingGame;
}

bool UI::HasSaveData() const
{
	return true;
}

bool UI::LoadState(pugi::xml_node const &data)
{
	return false;
}

pugi::xml_node UI::SaveState(pugi::xml_node const &data) const
{
	std::string saveData2 = "<{} {}=\"{}\"/>\n";
	std::string saveOpenData2 = "<{} {}=\"{}\">\n";
	std::string saveData4 = "<{} {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveOpenData4 = "<{} {}=\"{}\" {}=\"{}\">\n";
	std::string saveData6 = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\"/>\n";
	std::string saveData6OneFloat = "<{} {}=\"{}\" {}=\"{}\" {}=\"{}\" {}=\"{:.2f}\"/>\n";
	std::string saveFloatData = "<{} {}=\"{:.2f}\" {}=\"{:.2f}\"/>\n";
	std::string dataToSave = "<ui>\n";
	dataToSave += AddSaveData(saveData2, "pause", "pause", bDrawPause);
	dataToSave += "</ui>";

	app->AppendFragment(data, dataToSave.c_str());

	return data;
}
