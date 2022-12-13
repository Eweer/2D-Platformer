#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Character.h"
#include "Projectile.h"
#include <deque>
#include <vector>

class Player : public Character
{
public:

	Player();
	explicit Player(const pugi::xml_node &itemNode, int newId = 0);
	~Player() final;

	bool Awake() final;

	std::string ChooseAnim();
	bool StopProjectiles() override;

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

	bool bFalling = false;
	bool bDead = false;
	bool bHurt = false;
	bool bAttack2 = false;
	bool bClimbing = false;
	bool bPushing = false;
	bool bHighJump = false;
	bool bNormalJump = false;
	bool bRunning = false;
	bool bWalking = false;
	bool bIdle = false;
	bool bAttack1 = false;
	bool bLockAnim = false;
	bool bAttackQueue = false;
	bool bAbleToMove = true;

	friend class UI;
};

#endif // __PLAYER_H__