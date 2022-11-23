#include "Entity.h"

#include "Point.h"
#include "Input.h"
#include "Render.h"
#include "Animation.h"
#include "Log.h"
#include "Defs.h"
#include "dirent.h"
#include "Physics.h"
#include "App.h"

#include <string>
#include <regex>
#include <unordered_map>
#include <memory>
#include <variant>

Entity::Entity(EntityType type) : type(type) {}

Entity::Entity(pugi::xml_node const &itemNode)
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

bool Entity::Awake()
{
	SetStartingParameters();
	return true;
}

bool Entity::Start()
{
	return true;
}

bool Entity::Update()
{
	return true;
}

bool Entity::CleanUp()
{
	return true;
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
		CleanUp();
	}
}

void Entity::SetPaths()
{
	texturePath = parameters.parent().attribute("texturepath").as_string();

	fxPath = parameters.parent().attribute("audiopath").as_string();
	fxPath += parameters.parent().attribute("fxfolder").as_string();

	SetPathsToLevel();
}

void Entity::SetPathsToLevel()
{
	uint levelNumber = app->GetLevelNumber();

	std::string levelFolder = "level_" + std::to_string(levelNumber) + "/";

	texLevelPath = texturePath + levelFolder;
	fxLevelPath = fxPath + levelFolder;
}

bool Entity::SetStartingParameters()
{
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();

	SetPaths();

	return true;
}

void Entity::AddTexturesAndAnimationFrames()
{
	texture = std::make_unique<Animation>();
	
	if(!parameters.attribute("renderable").as_bool())
	{
		renderMode = RenderModes::NO_RENDER;
		return;
	}

	struct dirent **nameList;
	std::string entityFolder = texLevelPath + name + "/";

	const char *dirPath = entityFolder.c_str();
	int n = scandir(dirPath, &nameList, nullptr, DescAlphasort);
	static const std::regex r(R"(([A-Za-z]+(?:_[A-Za-z]*)*)_(?:(image|static|anim)([\d]*)).png)"); // www.regexr.com/72ogq
	bool foundOne = false;

	while(n--)
	{
		std::smatch m;

		if(std::string animFileName(nameList[n]->d_name); !std::regex_match(animFileName, m, r))
		{
			free(nameList[n]);
			continue;
		}

		auto animNameToAdd = std::string(parameters.name()) + "_" 
			+ std::string(parameters.child("animation").attribute("name").as_string());

		//example: triangle_left
		if(std::string match1 = m[1]; match1 != animNameToAdd)
		{
			if(foundOne) [[unlikely]] //As they are descalphasorted, once we found one but the name isn't the same
				return;
			else		 [[likely]]
				continue;			//there won't be any more
		}

		if(!foundOne) foundOne = true;

		std::string match2 = m[2]; // (image|static|anim)

		if(renderMode == RenderModes::UNKNOWN)
		{
			using enum RenderModes;
			if(match2 == "image") [[unlikely]]
				renderMode = IMAGE;
			else [[likely]]
			{
				renderMode = ANIMATION;
				texture->SetSpeed(parameters.child("animation").attribute("speed").as_float());
				texture->SetAnimStyle(static_cast<AnimIteration>(parameters.child("animation").attribute("animstyle").as_int()));
			}
		}

		std::string match0 = entityFolder + std::string(m[0]); //example: /Assets/Maps/ + triangle_left_anim001.png

		switch(renderMode)
		{
			case RenderModes::IMAGE:
			case RenderModes::ANIMATION: [[likely]]
				LOG("Loaded %s.", match0.c_str());
				if(match2 == "anim") texture->AddSingleFrame(match0.c_str());
				else texture->AddStaticImage(match0.c_str());
				break;
			default:
				break;
		}
		free(nameList[n]);
	}
	free(nameList);
}

void Entity::CreatePhysBody(Uint16 collisionCategory, Uint16 collisionMask)
{
	pugi::xml_node physicsNode = parameters.child("physics");
	
	if(physicsNode.empty()) [[unlikely]]
	{
		LOG("Entity %s has no physics node", name.c_str());
		return;
	}
	
	int height = physicsNode.attribute("height").as_int();
	int width = physicsNode.attribute("width").as_int();
	auto bodyType = (BodyType)GetParameterBodyType();
	float restitution = physicsNode.attribute("restitution") ? physicsNode.attribute("restitution").as_float() : 1.0f;
	float32 gravity = physicsNode.attribute("gravityscale") ? physicsNode.attribute("gravityscale").as_float() : 1.0f;

	if(physicsNode.attribute("size"))
	{
		int radius = physicsNode.attribute("size").as_int()/2;
		pBody = app->physics->CreateCircle(position.x + radius, position.y + radius, radius, bodyType, restitution, collisionCategory, collisionMask);
	}
	else if(physicsNode.attribute("width") && physicsNode.attribute("height"))
	{
		pBody = app->physics->CreateRectangle(position.x + width/2, position.y + height/2, width, height, bodyType, gravity, restitution, collisionCategory, collisionMask);
	}
	else [[unlikely]]
	{
		LOG("ERROR: Unknown shape of entity %s", name.c_str());
		return;
	}
	pBody->listener = this;
	pBody->ctype = ColliderType::PLAYER;
}

uint Entity::GetParameterBodyType() const
{
	switch(str2int(parameters.child("physics").attribute("bodytype").as_string()))
	{
		case str2int("static"):
			return (uint)BodyType::STATIC;
			break;
		case str2int("dynamic"):
			return (uint)BodyType::DYNAMIC;
			break;
		case str2int("kinematic"):
			return (uint)BodyType::KINEMATIC;
			break;
		default:
			LOG("ERROR: Invalid body type");
	}
	return (uint)BodyType::UNKNOWN;
}
