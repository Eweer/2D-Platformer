#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"
#include "Map.h"
#include "List.h"

#include <deque>
#include <utility>		//std::pair

struct EntityInfo
{
	std::vector<std::unique_ptr<Entity>> entities;
	std::deque<uint> emptyElements;
	std::unordered_map<int, std::unique_ptr<Animation>> animation;
};

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called after Awake
	bool Start() final;

	// Called every frame
	bool Update(float dt) final;

	// Called before quitting
	bool CleanUp() final;

	// Additional methods
	void CreateEntity(ColliderLayers type, pugi::xml_node parameters = pugi::xml_node());

	bool DestroyEntity(Entity const *entity);

	bool DestroyEntity(ColliderLayers type, int id);

	bool LoadAllTextures();

	bool LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height);

	bool IsEntityActive(Entity const *entity = nullptr) const;

	bool DoesEntityExist(Entity const *entity = nullptr) const;

	// Type of entity, pair<entities, list of empty spots>
	std::unordered_map<ColliderLayers, EntityInfo> allEntities;

};


#endif // __ENTITYMANAGER_H__
