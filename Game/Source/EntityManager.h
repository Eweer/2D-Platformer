#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"
#include "Map.h"
#include "List.h"

#include <deque>
#include <utility>		//std::pair

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

	bool LoadAllTextures() const;

	bool LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height);
	
	List<Entity*> entities;

	// Type of entity, pair<entities, list of empty spots>
	std::unordered_map<ColliderLayers, std::pair<std::vector<std::unique_ptr<Entity>>, std::deque<int>>> entities2;
};


#endif // __ENTITYMANAGER_H__
