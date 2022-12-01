#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Point.h"
#include "Map.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	
	Item(TileInfo const *tileInfo, iPoint pos, int width, int height);
	
	~Item() override;

	bool Awake() override;

	bool Start() override;

	bool Update() override;

	bool CleanUp() override;

	void AddTexturesAndAnimationFrames() override;

	void SetPaths() override;

	bool SetStartingParameters() override;

	bool isPicked = false;

	TileInfo const *info = nullptr;
	int width;
	int height;
	const Animation *anim;
};

#endif // __ITEM_H__