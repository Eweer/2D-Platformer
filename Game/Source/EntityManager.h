#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"

#include "BitMaskColliderLayers.h"

#include <vector>

class Player;
struct TileInfo;

struct EntityInfo
{
	CL::ColliderLayers type = CL::ColliderLayers::UNKNOWN;
	std::vector<std::unique_ptr<Entity>> entities;
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
	bool PreUpdate() final;
	void RestartLevel() const;
	bool Update(float dt) final;
	bool PostUpdate() final;

	// Called when game is paused
	bool Pause(int phase) final;

	// Called before quitting
	bool CleanUp() final;

	// ------ Entity
	// --- Constructors
	void CreateEntity(std::string const &type, pugi::xml_node const &parameters = pugi::xml_node());
	// --- Destructors
	bool DestroyEntity(std::string const &type,  int id);

	// ------ Load Assets
	bool LoadAllTextures() const;
	bool LoadEntities(TileInfo const *tileInfo, iPoint pos, int width, int height);
	void LoadItemAnimations();

private:
	// ------ Utils
	// --- Getters
	Player *GetPlayerCharacter() const;
	pugi::xml_node SaveState(pugi::xml_node const &data) const final;
	bool HasSaveData() const final;
	bool DoesEntityExist(Entity const *entity = nullptr) const;
	bool IsEntityActive(Entity const *entity = nullptr) const;
	// --- Constructors
	void CreateAllColliders() const;

	using EntityMap = std::unordered_map<std::string, EntityInfo, StringHash, std::equal_to<>>;
	// key1 = ColliderLayer
	// value 1, key 2 = string type of entity
	// value2 = <vector of entities, list of empty elements, map of animations>
	EntityMap allEntities;
	Player *player;
	std::string itemPath;

	friend class UI;
};


#endif // __ENTITYMANAGER_H__
