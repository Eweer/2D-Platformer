#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Physics.h"
#include "Render.h"
#include "Input.h"

#include "Animation.h"
#include "BitMaskColliderLayers.h"

class Projectile
{
public:
	Projectile() = default;
	Projectile(std::vector<SDL_Texture *> const &anim, iPoint origin, CL::ColliderLayers bitmask, int speed)
	{
		if(anim.empty())
		{
			LOG("Projectile could not be created. Anim not found.");
			return;
		}
		animation = anim;
		position = origin;
		iPoint direction = app->input->GetMousePosition() - origin;
		velocity = {
			PIXEL_TO_METERS(25),
			PIXEL_TO_METERS(0)
		};
		pBody = app->physics->CreateQuickProjectile(origin, bitmask);
		pBody->body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	}

	bool Update()
	{
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
		app->render->DrawCharacterTexture(animation[0], position);
		return true;
	}

	iPoint position = {0,0};
	std::vector<SDL_Texture *>animation;
	b2Vec2 velocity = {0,0};
	std::unique_ptr<PhysBody> pBody;
};

#endif // __PROJECTILE_H__
