#include "Entity.h"
#include "App.h"

// Modules
#include "Input.h"
#include "Render.h"
#include "Physics.h"

// Utils
#include "Log.h"

// Libraries
#include "dirent.h"

#include <regex>

#pragma warning(push)
#pragma warning(disable : 123)

Entity::Entity(pugi::xml_node const &itemNode) : parameters(itemNode) {}

bool Entity::Awake()
{
	return true;
}

bool Entity::Start()
{
	SpawnEntity();
	return true;
}

bool Entity::Update()
{
	return true;
}

bool Entity::Pause() const
{
	return true;
}

bool Entity::CleanUp()
{
	return true;
}

bool Entity::Stop()
{
	app->physics->DestroyBody(pBody->body);
	pBody.reset();
	return true;
}

// Sets starting Position and creates PhysBody
void Entity::SpawnEntity()
{
	position = startingPosition;
	CreatePhysBody();
}

bool Entity::LoadState(pugi::xml_node const &)
{
	return true;
}

pugi::xml_node Entity::SaveState(pugi::xml_node const &)
{
	return pugi::xml_node();
}

void Entity::Enable()
{
	if(!active)
	{
		active = true;
		Start();
	}
}

void Entity::Disable()
{
	if(active)
	{
		active = false;
		Stop();
	}
}

BodyType Entity::BodyTypeStrToEnum(std::string const &str) const
{
	std::string bodyType = str;
	bodyType[0] = std::tolower(bodyType[0], std::locale());
	using enum BodyType;
	switch(str2int(bodyType.c_str()))
	{
		case str2int("static"):
			return STATIC;
			break;
		case str2int("dynamic"):
			return DYNAMIC;
			break;
		case str2int("kinematic"):
			return KINEMATIC;
			break;
		default:
			LOG("ERROR: Invalid body type");
	}
	return UNKNOWN;
}


#pragma warning( pop )
