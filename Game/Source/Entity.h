#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Point.h"
#include "Input.h"
#include "Render.h"
#include "Animation.h"
#include "Log.h"
#include "Defs.h"
#include "BitMaskColliderLayers.h"

#include <string>
#include <regex>
#include <unordered_map>
#include <memory>
#include <variant>

class PhysBody;

enum class SensorFunction
{
	DEATH = 0,
	POWER,
	HP_UP,
	UNKNOWN
};

enum class RenderModes
{
	IMAGE = 0,
	ANIMATION,
	NO_RENDER,
	UNKNOWN
};

class Entity
{
public:

	explicit Entity() = default;

	explicit Entity(ColliderLayers type);

	explicit Entity(pugi::xml_node const &itemNode);

	virtual ~Entity() = default;

	virtual bool Awake();

	// Sets position, file paths and textures.
	virtual bool SetStartingParameters();

	virtual bool Start();

	virtual bool Update();

	virtual bool CleanUp();

	virtual bool LoadState(pugi::xml_node const &);

	virtual pugi::xml_node SaveState(pugi::xml_node const &);

	void Enable();
	
	void Disable();

	virtual void SetPaths();

	virtual void SetPathsToLevel();

	virtual void AddTexturesAndAnimationFrames();

	void CreatePhysBody(Uint16 collisionCategory = 0, Uint16 collisionMask = 0);

	uint GetParameterBodyType() const;

	virtual void OnCollision(PhysBody *physA, PhysBody *physB) {};

	bool active = true;

	std::string name = "unknown";
	ColliderLayers type = ColliderLayers::UNKNOWN;

	iPoint position;
	iPoint startingPosition;
	std::unique_ptr<Animation> texture;
	int imageVariation = 0;
	RenderModes renderMode = RenderModes::UNKNOWN;
	std::shared_ptr<PhysBody> pBody;

	pugi::xml_node parameters;
	std::string texturePath;
	std::string texLevelPath;

	std::string fxPath;
	std::string fxLevelPath;
};

#endif // __ENTITY_H__