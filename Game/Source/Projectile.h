#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Physics.h"
#include "Render.h"
#include "Input.h"
#include "Physics.h"
#include "Animation.h"
#include "Point.h"
#include "BitMaskColliderLayers.h"

class ProjectileData
{
public:
	ProjectileData() = default;
	explicit ProjectileData(std::string const &s, std::vector<b2Vec2> const &p, iPoint pos, iPoint wh, CL::ColliderLayers cl, int sp)
		: shape(ShapeData(s, p)), position(pos), width_height(wh), bitmask(cl), speed(sp)
	{
	};
	ProjectileData(const ProjectileData &other) = default;
	ProjectileData &operator=(const ProjectileData &other)
	{
		shape.Create(other.shape.data);
		position = other.position;
		width_height = other.width_height;
		bitmask = other.bitmask;
		speed = other.speed;
		return *this;
	}
	~ProjectileData() = default;

	ShapeData shape;
	iPoint position = {0,0};
	iPoint width_height = {0,0};
	CL::ColliderLayers bitmask = CL::ColliderLayers::UNKNOWN;
	int speed = 0;
};

class Projectile
{
public:
	Projectile() = default;
	explicit Projectile(std::vector<SDL_Texture *> const &anim, iPoint origin, ProjectileData &info)
	{
		if(anim.empty())
		{
			LOG("Projectile could not be created. Anim not found.");
			return;
		}
		animation = anim;
		position = origin;

		animOffset = info.position;

		// Create Body
		auto bodyPtr = app->physics->CreateBody(
			origin,
			BodyType::DYNAMIC
		);
		bodyPtr->SetGravityScale(0);
		bodyPtr->SetBullet(true);

		// Create Fixture
		using enum CL::ColliderLayers;
		uint16 cat = 0x0001;
		if((info.bitmask & PLAYER) == PLAYER) cat = static_cast<uint16>(PLAYER);
		else if((info.bitmask & ENEMIES) == ENEMIES) cat = static_cast<uint16>(ENEMIES);
		auto fixPtr = app->physics->CreateFixtureDef(
			info.shape,
			cat,
			static_cast<uint16>(info.bitmask)
		);
		bodyPtr->CreateFixture(fixPtr.get());
		
		// Create PhysBody
		auto pbodyPtr = app->physics->CreatePhysBody(
			bodyPtr,
			info.width_height,
			static_cast<CL::ColliderLayers>(cat)
		);

		pBody = std::move(pbodyPtr);
		
		direction = PIXEL_TO_METERS(app->input->GetMousePosition() - origin);
		direction.Normalize();
		auto s = PIXEL_TO_METERS(info.speed);
		pBody->body->SetLinearVelocity({direction.x * s, direction.y * s});
	}
	Projectile(const Projectile &) = default;
	Projectile(Projectile &&) = default;
	~Projectile() = default;

	Projectile &operator =(const Projectile &) = default;

	bool Update()
	{
		position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
		position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
		app->render->DrawCharacterTexture(
			animation[0],
			iPoint(position.x - animOffset.x, position.y - animOffset.y)
		);
		return true;
	}

	iPoint position = {0,0};
	iPoint animOffset = {0,0};
	std::vector<SDL_Texture *>animation;
	b2Vec2 direction = {0,0};
	std::unique_ptr<PhysBody> pBody;
};

#endif // __PROJECTILE_H__
