#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Physics.h"
#include "Render.h"
#include "Input.h"
#include "Log.h"
#include "Animation.h"
#include "Point.h"
#include "BitMaskColliderLayers.h"
#include "Textures.h"
#include "Physics.h"
#include "Box2D/Box2D/Box2D.h"

enum class ProjectileFreedom : uint16
{
	NONE = 0x0000,
	ONEDIR = 0x0001,
	TWODIR = 0x0002,
	FOURDIR = 0x0004,
	EIGHTDIR = 0x0008,
	ANYDIR = 0x0010,
	ALL = 0x0020
};

class ProjectileData
{
public:
	ProjectileData() = default;
	explicit ProjectileData(ShapeData const &s, bool sensor, iPoint pos, iPoint wh, int sp, ProjectileFreedom dir)
		: position(pos), width_height(wh), speed(sp), freedom(dir)
	{
		shape.Create(s.data);
		using enum CL::ColliderLayers;
		auto fixture = app->physics->CreateFixtureDef(
			shape,
			static_cast<uint16>(BULLET),
			static_cast<uint16>(ENEMIES | PLATFORMS),
			sensor
		);
		fixPtr = std::move(fixture);
	};
	ProjectileData(const ProjectileData &other) = default;

	~ProjectileData() = default;

	ShapeData shape;
	std::unique_ptr<b2FixtureDef>fixPtr;
	iPoint position = {0,0};
	iPoint width_height = {0,0};
	int speed = 0;
	ProjectileFreedom freedom = ProjectileFreedom::ANYDIR;
};

class Projectile
{
public:
	Projectile() = default;
	explicit Projectile(std::vector<std::shared_ptr<SDL_Texture>> const &anim, iPoint origin, std::unique_ptr<ProjectileData> const &info, b2Vec2 dir)
	{
		if(anim.empty())
		{
			LOG("Projectile could not be created. Anim not found.");
			return;
		}
		
		animation = anim;
		position = origin;

		animOffset = info->position;

		uint tw = 0;
		uint th = 0;
		app->tex->GetSize(anim[0].get(), tw, th);

		direction = dir;
		direction.Normalize();

		rotationCenter = {info->position.x, info->position.y};

		if(direction.x < 0)
		{
			flipValue = 2;

			if(info->freedom == ProjectileFreedom::TWODIR)
			{
				direction.x = -0.5f;
				direction.y = 0;
				origin.x -= 20;
				origin.y += 16;
			}

			if(info->position.y == 0)
			{
				rotationCenter.y = 0;
				origin.y += th;
			}
			else
				rotationCenter.y = th - rotationCenter.y;
		}
		else
		{
			flipValue = 0;

			if(info->freedom == ProjectileFreedom::TWODIR)
			{
				direction.x = 0.5f;
				direction.y = 0;
				origin.x += 40;
				origin.y += 16;
			}

		}

		degree = atan2f(direction.y, direction.x) * 180.0f/std::numbers::pi_v<float>;

		// Create Body
		auto bodyPtr = app->physics->CreateBody(
			origin,
			BodyType::DYNAMIC,
			degree
		);
		bodyPtr->SetGravityScale(0);
		bodyPtr->SetBullet(true);
		bodyPtr->CreateFixture(info->fixPtr.get());

		using enum CL::ColliderLayers;
		auto bitmask = info->fixPtr->filter.maskBits;

		// Create PhysBody
		if((bitmask & PLAYER) != PLAYER) source = PLAYER;
		else if((bitmask & ENEMIES) != ENEMIES) source = ENEMIES;

		auto pbodyPtr = app->physics->CreatePhysBody(
			bodyPtr,
			info->width_height,
			BULLET
		);

		pBody = std::move(pbodyPtr);

		auto s = PIXEL_TO_METERS(info->speed);
		pBody->body->SetLinearVelocity({direction.x * s, direction.y * s});
		pBody->pListener = this;
	}
	Projectile(const Projectile &) = default;
	Projectile(Projectile &&) = default;
	virtual ~Projectile() = default;

	Projectile &operator =(const Projectile &) = default;

	bool Update()
	{
		animTimer++;
		if(animTimer >= 6)
		{
			currentFrame++;
			animTimer = 0;
		}

		if(bExploding)
		{
			if(bDestroyPBody)
			{
				app->physics->DestroyBody(pBody->body);
				bDestroyPBody = false;
			}
			if(currentFrame >= animation.size()) return false;
		}
		else
		{
			position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
			position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
			if(currentFrame >= 3) currentFrame = 0;
		}

		app->render->DrawCharacterTexture(
				animation[currentFrame].get(),
				iPoint(position.x - animOffset.x, position.y - animOffset.y),
				false,
				rotationCenter,
				iPoint(INT_MAX, INT_MAX),
				degree,
				flipValue
		);
		return true;
	}

	void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
	{
		if((pBodyB->ctype & CL::ColliderLayers::PLATFORMS) == CL::ColliderLayers::PLATFORMS)
		{
			OnCollisionStart(fixtureA, fixtureB, pBodyA, pBodyB);
		}
	}

	void OnCollisionStart([[maybe_unused]] b2Fixture const *fixtureA, [[maybe_unused]] b2Fixture const *fixtureB, [[maybe_unused]] PhysBody const *pBodyA, [[maybe_unused]] PhysBody const *pBodyB)
	{
		bExploding = true;
		bDestroyPBody= true;
		animTimer = 0;
	}
	
	virtual void OnCollisionEnd(PhysBody *physA, PhysBody *physB) { /* Method to Override */ };

	bool Pause() const
	{
		return app->render->DrawCharacterTexture(
				animation[currentFrame].get(),
				iPoint(position.x - animOffset.x, position.y - animOffset.y),
				false,
				rotationCenter,
				iPoint(INT_MAX, INT_MAX),
				degree,
				flipValue
		);
	}

	bool bDestroyPBody = false;
	bool bExploding = false;
	int currentFrame = 0;
	int animTimer = 0;
	int flipValue = 0;
	float degree = 0.0f;
	CL::ColliderLayers source = CL::ColliderLayers::UNKNOWN;
	iPoint position = {0,0};
	SDL_Point rotationCenter = {0,0};
	iPoint animOffset = {0,0};
	std::vector<std::shared_ptr<SDL_Texture>>animation;
	b2Vec2 direction = {0,0};
	std::unique_ptr<PhysBody> pBody;
};

#endif // __PROJECTILE_H__
