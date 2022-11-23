#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Point.h"
#include "Input.h"
#include "Render.h"
#include "Animation.h"
#include "Log.h"
#include "Defs.h"

#include <string>
#include <regex>
#include <unordered_map>
#include <memory>
#include <variant>

class PhysBody;

enum class EntityType
{
	PLAYER,
	ITEM,
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

	explicit Entity(EntityType type) : type(type) {}

	explicit Entity(pugi::xml_node const &itemNode) : parameters(itemNode)
	{
		const std::unordered_map<std::string, EntityType, StringHash, std::equal_to<>> entityTypeStrToEnum =
		{
			{"player", EntityType::PLAYER},
			{"item", EntityType::ITEM}
		};

		std::smatch m;
		if(std::string itemName(itemNode.name()); !std::regex_search(itemName, m, std::regex(R"([A-Za-z]+)")))
		{
			LOG("XML %s name is not correct. [A-Za-z]+", itemNode.name());
			return;
		}

		if(!entityTypeStrToEnum.contains(m[0]))
		{
			LOG("%s string does not have a mapped enum", m[0]);
			return;
		}

		if(static_cast<uint>(entityTypeStrToEnum.at(m[0])) >= static_cast<uint>(EntityType::UNKNOWN))
		{
			LOG("%s does not have a valid EntityType", m[0]);
			return;
		}
		this->name = m[0];
		this->type = entityTypeStrToEnum.at(name);

		renderMode = RenderModes::UNKNOWN;
	}

	virtual ~Entity() = default;

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	virtual bool LoadState(pugi::xml_node const &)
	{
		return true;
	}

	virtual pugi::xml_node SaveState(pugi::xml_node const &)
	{
		return pugi::xml_node();
	}

	void Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB) 
	{
		// To Override
	};

	bool active = true;

	std::string name = "unknown";
	EntityType type = EntityType::UNKNOWN;

	iPoint position;
	std::unique_ptr<Animation> texture;
	RenderModes renderMode = RenderModes::UNKNOWN;
	PhysBody *pBody = nullptr;

	pugi::xml_node parameters;
	std::string texturePath;
	std::string texLevelPath;

	std::string fxPath;
	std::string fxLevelPath;
};

#endif // __ENTITY_H__