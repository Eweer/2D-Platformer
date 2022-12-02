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

	bool CleanUp() final;

	void OnCollision(PhysBody* physA, PhysBody* physB) final;

private:
	entityJump jump;
	uint cameraXCorrection;
	uint cameraYCorrection;



};

#endif // __PLAYER_H__