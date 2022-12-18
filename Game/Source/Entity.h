#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Point.h"
#include "Animation.h"
#include "BitMaskColliderLayers.h"
#include "PugiXml/src/pugixml.hpp"

class b2Fixture;
class PhysBody;
enum class BodyType;

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
	explicit Entity(pugi::xml_node const &itemNode, int newId);
	virtual ~Entity() = default;

	virtual bool Awake();

	void Enable();
	virtual bool Start();
	virtual void SpawnEntity();
	virtual void CreatePhysBody() { /* Method to Override */ };
	BodyType BodyTypeStrToEnum(std::string const &str) const;

	void Disable();
	virtual bool Stop();
	virtual bool UpdateProjectiles();

	virtual bool Update();
	virtual bool Pause() const;
	
	virtual bool CleanUp();

	virtual bool LoadState(pugi::xml_node const &);
	virtual pugi::xml_node SaveState(pugi::xml_node const &);

	virtual void OnCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) { /* Method to Override */ };
	virtual void OnCollisionEnd(PhysBody const *physA, PhysBody const *physB) { /* Method to Override */ };
	virtual void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) { /* Method to Override */ };

	virtual void DrawDebug() const { /* Method to Override */ };

	bool active = true;

	bool disableOnNextUpdate = false;

	int id = -1;
	std::string name = "unknown";
	CL::ColliderLayers type = CL::ColliderLayers::UNKNOWN;

	iPoint position;
	iPoint startingPosition;
	iPoint colliderOffset;

	std::unique_ptr<Animation> texture;
	int imageVariation = -1;

	std::unique_ptr<PhysBody> pBody;

	pugi::xml_node parameters;
	std::string texturePath;
	std::string fxPath;
};

#endif // __ENTITY_H__