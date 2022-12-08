#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"
#include "Map.h"

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
	~EntityManager() final;

	// Called before render is available
	bool Awake(pugi::xml_node&) final;

	// Called after Awake
	bool Start() final;

	// Called every frame
	bool Update(float dt) final;

	// Called before quitting
	bool CleanUp() final;

	// Additional methods
	void CreateEntity(std::string const &type, pugi::xml_node parameters = pugi::xml_node());

	bool DestroyEntity(Entity const *entity, std::string const &type);

	bool DestroyEntity(std::string const &type,  int id);

	bool LoadAllTextures();

	bool LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height);

	void LoadItemAnimations();

	bool IsEntityActive(Entity const *entity = nullptr) const;

	bool DoesEntityExist(Entity const *entity = nullptr) const;

	void CreateAllColliders();

	using EntityMap = std::unordered_map<std::string, EntityInfo, StringHash, std::equal_to<>>;
	
	// key1 = ColliderLayer
	// value 1, key 2 = string type of entity
	// value2 = <vector of entities, list of empty elements, map of animations>
	EntityMap allEntities;

};


#endif // __ENTITYMANAGER_H__
