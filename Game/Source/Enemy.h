#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Character.h"
#include "Pathfinding.h"

class Enemy : public Character
{
public:
	Enemy();
	explicit Enemy(const pugi::xml_node &itemNode, int newId);
	~Enemy() final;

	bool Awake() override;
	bool Update() override;
	void OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) override;
	bool SetPath(iPoint destination);

	int currentPathIndex = 0;
	std::unique_ptr<std::vector<iPoint>> path;

	bool bAttack = false;
	bool bDeath = false;
	bool bHurt = false;
	bool bIdle = false;
	bool bWalk = false;
};

#endif // __ENEMY_H__
