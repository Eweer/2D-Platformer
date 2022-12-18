#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"

struct TileInfo;
class ShapeData;

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
	std::unordered_map<std::string, std::vector<std::pair<ShapeData, iPoint>>, StringHash, std::equal_to<>> colliderMap;
	int width = 0;
	int height = 0;
	const Animation *anim;
	std::string itemClass = "Unknown";
};

#endif // __ITEM_H__