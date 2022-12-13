#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Character.h"
#include "Projectile.h"

class Player : public Character
{
public:

	Player();
	explicit Player(const pugi::xml_node &itemNode, int newId = 0);
	~Player() final;

	bool Awake() final;

	bool Update() final;

	void BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) final;

private:
	bool LoadProjectileData();

	CharacterJump jump;
	uint cameraXCorrection = 0;
	uint cameraYCorrection  = 0;

	bool bKeepMomentum = false;
	b2Vec2 velocityToKeep = {0.0f, 0.0f};

	std::string playerCharacter;

	std::unordered_map<std::string, ProjectileData, StringHash, std::equal_to<>> projectileMap;
	std::vector<std::unique_ptr<Projectile>> projectiles;

	friend class UI;
};

#endif // __PLAYER_H__