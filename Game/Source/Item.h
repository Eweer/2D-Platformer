#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Point.h"
#include "Map.h"
#include "SDL/include/SDL.h"
#include "Box2D/Box2D/Box2D.h"

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

	bool Pause() const override;

	bool CleanUp() override;

	void AddTexturesAndAnimationFrames() override;

	void SetPaths() override;

	bool SetStartingParameters() override;

	void CreatePhysBody() final;

	void SendContact(b2Contact *c) final;


	bool isPicked = false;

	TileInfo const *info = nullptr;
	std::unordered_map<std::string, std::vector<std::pair<ShapeData, iPoint>>, StringHash, std::equal_to<>> colliderMap;
	int width = 0;
	int height = 0;
	const Animation *anim;
	std::string itemClass = "Unknown";
};

#endif // __ITEM_H__