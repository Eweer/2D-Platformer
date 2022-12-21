#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Physics.h"

struct TileInfo;

class Item : public Entity
{
public:
	Item();
	Item(TileInfo const *tileInfo, iPoint pos, int width, int height);
	~Item() override;

	bool Start() override;
	void CreatePhysBody() final;

	bool Update() override;
	bool Pause() const override;

	void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) final;

	void PickUpEffect() const;

	bool HasSaveData() const final;
	bool LoadState(pugi::xml_node const &data) final;
	pugi::xml_node SaveState(pugi::xml_node const &data) final;

	TileInfo const *info = nullptr;
	ShapeData shape;
	int width = 0;
	int height = 0;
	std::shared_ptr<Animation> anim;
	std::string itemClass = "Unknown";
};

#endif // __ITEM_H__