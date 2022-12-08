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

#pragma warning(push)
#pragma warning(disable : 123)

Entity::Entity(ColliderLayers type) : type(type) {}

Entity::Entity(pugi::xml_node const &itemNode) : parameters(itemNode)
{
	const std::unordered_map<std::string, ColliderLayers, StringHash, std::equal_to<>> entityTypeStrToEnum =
	{
		{"player", ColliderLayers::PLAYER},
		{"item", ColliderLayers::ITEMS}
	};

	std::smatch m;
	if(std::string itemName(itemNode.name()); !std::regex_search(itemName, m, std::regex(R"([A-Za-z]+)")))
	{
		LOG("XML %s name is not correct. [A-Za-z]+", itemNode.name());
		return;
	}

	if(!entityTypeStrToEnum.contains(m[0]))
	{
		LOG("%s does not have a mapped enum", itemNode.name());
		return;
	}

	if(static_cast<uint>(entityTypeStrToEnum.at(m[0])) >= static_cast<uint>(ColliderLayers::UNKNOWN))
	{
		LOG("%s does not have a valid ColliderLayers", m[0]);
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
	startingPosition = {
		parameters.attribute("x").as_int(),
		parameters.attribute("y").as_int()
	};
	position = startingPosition;
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

		//example: triangle_left
		if(std::string match1 = m[1]; match1 != std::string(parameters.name()))
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

BodyType Entity::GetParameterBodyType(std::string const &str) const
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
