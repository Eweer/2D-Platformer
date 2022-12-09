#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Entity.h"
#include "Character.h"

#include "Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Player : public Character
{
public:

	Player();

	explicit Player(const pugi::xml_node &itemNode);
	
	~Player() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool Pause() const final;

	bool CleanUp() final;

	void SendContact(b2Contact *c) final;

	void OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) final;
	void BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) final;

private:
	CharacterJump jump;
	uint cameraXCorrection = 0;
	uint cameraYCorrection  = 0;

	bool bKeepMomentum = false;
	b2Vec2 velocityToKeep = {0.0f, 0.0f};

	std::string playerCharacter;

	friend class UI;
};

#endif // __PLAYER_H__