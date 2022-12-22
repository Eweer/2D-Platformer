#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Character.h"
#include "Pathfinding.h"

enum class BehaviourState : int
{
	IDLE = 0x0000,
	PATROL = 0x0001,
	AGGRO = 0x0002,
	DEAD = 0x0004
};

class Enemy : public Character
{
public:
	Enemy();
	explicit Enemy(const pugi::xml_node &itemNode, int newId);
	~Enemy() final;

	bool Awake() override;
	bool Update() override;
	void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) final;
	bool SetPath(iPoint destination);

	b2Vec2 SetPathMovementParameters(iPoint currentCords);
	void DrawDebug() const final;
	void DrawDebugPath() const;
	b2Vec2 SetGroundPathMovement();
	b2Vec2 SetAirPathMovement(iPoint currentCoords);

	BehaviourState SetBehaviour(iPoint playerPosition, iPoint screenSize);

	bool HasSaveData() const final;
	bool LoadState(pugi::xml_node const &data) final;
	pugi::xml_node SaveState(pugi::xml_node const &data) final;

	void SpecificRestart() final;

	int attackCD = 60;
	int attackTimer = 0;

	std::string enemyClass = "";

	int currentPathIndex = 0;
	std::string lastDirection = "none";
	std::unique_ptr<std::vector<iPoint>> path;
	bool bRequestPath = false;
	PathfindTerrain pTerrain = PathfindTerrain::GROUND;

	int aggroRadius = 0;
	int patrolRadius = 5;

	BehaviourState behaviour = BehaviourState::IDLE;

	int tileYOnDeath = 0;

	bool bAtTile = false;

	bool bAttack = false;
	bool bDeath = false;
	bool bHurt = false;
	bool bIdle = false;
	bool bWalk = false;
};

#endif // __ENEMY_H__
