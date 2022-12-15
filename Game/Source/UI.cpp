#include "App.h"
#include "UI.h"
#include "Textures.h"
#include "Fonts.h"
#include "Render.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Player.h"
#include "Input.h"

#include "Defs.h"
#include "Point.h"

#include <format>

UI::UI() : Module()
{
	name = "ui";
}

UI::~UI() = default;

// Called before render is available
bool UI::Awake(pugi::xml_node &)
{
	return true;
}

// Called before the first frame
bool UI::Start()
{
	fCleanCraters = app->fonts->Load("CleanCraters");
	player = app->entityManager->GetPlayerCharacter();
	return true;
}

// Called each loop iteration
bool UI::PreUpdate()
{
	// Return all coordinates to their original values
	pTopLeft = {10, -1 * app->render->camera.y + 10};
	if(pTopLeft.x < 10) pTopLeft.x = 10;
	pMiddle = {app->render->camera.w / 2, app->render->camera.h / 2};
	return true;
}

// Called each loop iteration
bool UI::PostUpdate()
{
	DrawFPS(pTopLeft);
	DrawGravity(pTopLeft);
	DrawMousePosition(pTopLeft);
	DrawCameraPosition(pTopLeft);
	DrawUIPosition(pTopLeft);
	DrawPlayerPosition(pTopLeft);
	DrawPlayerJumps(pTopLeft);
	DrawPlayerAnimation(pTopLeft);
	if(bDrawPause) DrawPause(pMiddle);
	return true;
}

void UI::DrawPause(iPoint &position) const
{
	app->fonts->DrawMiddlePoint("GAME PAUSED", position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
}

void UI::DrawFPS(iPoint &position) const
{
	// Always Draw this
	app->fonts->Draw(std::format("Current FPS: {}", app->render->fps), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;

	if(!app->render->vSyncActive)
	{
		// Non-sense to Draw this if VSync is active
		app->fonts->Draw(std::format("Target FPS: {}", app->render->fpsTarget), position, fCleanCraters);
		position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	}

	// VSync state
	if(app->render->vSyncActive != app->render->vSyncOnRestart)
		app->fonts->Draw("Restart for VSync changes.", position, fCleanCraters);
	else
		app->fonts->Draw(std::format("VSync is {}.", app->render->vSyncActive ? "enabled" : "disabled"), position, fCleanCraters);
	
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
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
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;

}

void UI::DrawPlayerPosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Player position: \"{},{}\"",
			player->position.x,
			player->position.y
		),
		position,
		fCleanCraters
	);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
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
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
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
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
}

void UI::DrawMousePosition(iPoint &position) const
{
	app->fonts->Draw(
		std::format(
			"Mouse position: \"{},{}\"",
			app->input->mouseX,
			app->input->mouseY
		),
		position,
		fCleanCraters
	);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
}

void UI::DrawPlayerJumps(iPoint &position) const
{
	app->fonts->Draw(std::format("Max Jumps: {}", player->jump.maxJumps), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	app->fonts->Draw(std::format("Is on air: {}", player->jump.bOnAir ? "Yes." : "No."), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	app->fonts->Draw(std::format("Current Jumps: {}", player->jump.currentJumps), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	app->fonts->Draw(std::format("Jump Impulse: {}", player->jump.jumpImpulse), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
}

void UI::DrawPlayerAnimation(iPoint &position) const
{
	app->fonts->Draw(std::format("Able to move: {}", player->bAbleToMove ? "Yes." : "No."), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	app->fonts->Draw(std::format("Locked Animation: {}", player->bLockAnim ? "Yes." : "No."), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	app->fonts->Draw(std::format("Current Veloicty: {:.1f}, {:.1f}", player->pBody->body->GetLinearVelocity().x, player->pBody->body->GetLinearVelocity().y), position, fCleanCraters);
	position.y += app->fonts->fonts[fCleanCraters].lineHeight + app->fonts->fonts[fCleanCraters].spacing.y;
	
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
