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
	explicit Projectile(std::vector<SDL_Texture *> const &anim, iPoint origin, ProjectileData &info, b2Vec2 dir)
	{
		if(anim.empty())
		{
			LOG("Projectile could not be created. Anim not found.");
			return;
		}
		
		animation = anim;
		position = origin;

		direction = dir;
		direction.Normalize();

		animOffset = info.position;

		uint tw = 0;
		uint th = 0;
		app->tex->GetSize(anim[0], tw, th);
		
		flipValue = (direction.x < 0) ? 2 : 0;
		rotationCenter.x = info.position.x;
		rotationCenter.y = to_bool(flipValue) ? th - info.position.y : info.position.y;
		if(info.position.y == 0 && flipValue == 2)
		{
			rotationCenter.y = 0;
			origin.y += th;
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

		// Create Fixture
		using enum CL::ColliderLayers;
		auto fixPtr = app->physics->CreateFixtureDef(
			info.shape,
			static_cast<uint16>(BULLET),
			static_cast<uint16>(info.bitmask)
		);
		bodyPtr->CreateFixture(fixPtr.get());
		
		// Create PhysBody
		if((info.bitmask & PLAYER) != PLAYER) source = PLAYER;
		else if((info.bitmask & ENEMIES) != ENEMIES) source = ENEMIES;

		auto pbodyPtr = app->physics->CreatePhysBody(
			bodyPtr,
			info.width_height,
			BULLET
		);

		pBody = std::move(pbodyPtr);

		auto s = PIXEL_TO_METERS(info.speed);
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
		if(!bExploding)
		{
			position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
			position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
			if(currentFrame >= 3) currentFrame = 0;
		}

		app->render->DrawCharacterTexture(
				animation[currentFrame],
				iPoint(position.x - animOffset.x, position.y - animOffset.y),
				false,
				rotationCenter,
				iPoint(INT_MAX, INT_MAX),
				degree,
				flipValue
		);
		return true;
	}

	void OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB)
	{
		bExploding = true;
		bDestroyPBody= true;
		animTimer = 0;
	}
	
	virtual void OnCollisionEnd(PhysBody *physA, PhysBody *physB) { /* Method to Override */ };
	virtual void BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) { /* Method to Override */ };

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
	std::vector<SDL_Texture *>animation;
	b2Vec2 direction = {0,0};
	std::unique_ptr<PhysBody> pBody;
};

#endif // __PROJECTILE_H__
