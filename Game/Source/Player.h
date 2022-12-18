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
	bool UpdateProjectiles() override;

	bool Update() final;

	void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) final;

	bool IsOnAir() const;

	bool Pause() const final;

	bool DidChangeTile() const;

private:
	bool LoadProjectileData();

	iPoint coordinates = {0, 0};
	bool changedTile = true;

	CharacterJump jump;

	bool bKeepMomentum = false;
	b2Vec2 velocityToKeep = {0.0f, 0.0f};

	std::string playerCharacter;

	std::unordered_map<std::string, std::unique_ptr<ProjectileData>, StringHash, std::equal_to<>> projectileMap;
	std::vector<std::unique_ptr<Projectile>> projectiles;

	bool bMoveCamera = true;

	int skillCDTimer = 0;
	int skillCD = 0;

	int bFalling = 0;
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
	bool bAbleToMove = true;

	bool bAttackQueue = false;
	b2Vec2 attackDir = {0, 0};

	friend class UI;
};

#endif // __PLAYER_H__