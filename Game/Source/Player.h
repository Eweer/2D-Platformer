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
	
	~Player() final;

	bool Awake() final;

	bool Start() final;

	bool Update() final;

	bool CleanUp() final;

	// L07 DONE 6: Define OnCollision function for the player. Check the virtual function on Entity class
	void OnCollision(PhysBody* physA, PhysBody* physB) final;

private:
	entityJump jump;
	uint cameraXCorrection;
	uint cameraYCorrection;



};

#endif // __PLAYER_H__