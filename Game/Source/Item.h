#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	
	~Item() override;

	bool Awake() override;

	bool Start() override;

	bool Update() override;

	bool CleanUp() override;
	
	bool isPicked = false;
};

#endif // __ITEM_H__