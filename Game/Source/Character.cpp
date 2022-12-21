#include "Character.h"
#include "App.h"

#include "Render.h"

#include "Log.h"

#include <regex>


constexpr uint Character_SIZE = 30;

//---------- Constructors
Character::Character() = default;
Character::Character(pugi::xml_node const &itemNode = pugi::xml_node(), int newId = 0) : Entity(itemNode, newId) {}
Character::~Character() = default;

//---------- Load Parameters


void Character::SetPaths()
{
	texturePath = parameters.parent().attribute("texturepath").as_string();

	fxPath = parameters.parent().attribute("audiopath").as_string();
	fxPath += parameters.parent().attribute("fxfolder").as_string();

}

void Character::InitializeTexture() const
{
	if(!texture) return;

	if(!texture->Start("idle"))
		LOG("Couldnt start anim");
	if(texture->GetAnimStyle() != AnimIteration::LOOP_FROM_START)
		texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);
}

//---------- Create character

bool Character::Start()
{
	SpawnEntity();
	InitializeTexture();

	return true;
}

void Character::AddTexturesAndAnimationFrames()
{
	texture = std::make_unique<Animation>();
	
	std::string entityFolder = "";
	if(!CreateEntityPath(entityFolder)) return;
	
	auto animDataNode = parameters.child("animationdata");
	
	textureOffset = {
		.x = animDataNode.child("properties").attribute("pivotx").as_int(),
		.y = animDataNode.child("properties").attribute("pivoty").as_int()
	};

	texture->setPivot(textureOffset);
	
	struct dirent **folderList;
	const char *dirPath = entityFolder.c_str();
	int nCharacterFolder = scandir(dirPath, &folderList, nullptr, DescAlphasort);
	
	if(nCharacterFolder < 0) return;

	//for each file/folder in Character folder
	while(nCharacterFolder--)
	{
		if(folderList[nCharacterFolder]->d_name[0] == '.')
		{
			free(folderList[nCharacterFolder]);
			continue;
		}
		struct dirent **nameList;
		std::string animationPath = entityFolder + std::string(folderList[nCharacterFolder]->d_name) + "/";
		int nAnimationContents = scandir(animationPath.c_str(), &nameList, nullptr, DescAlphasort);
		
		if(nAnimationContents < 0) break;

		//for each file in subfolders of Character folder
		while(nAnimationContents--)
		{
			if(nameList[nAnimationContents]->d_name[0] == '.')
			{
				free(nameList[nAnimationContents]);
				continue;
			}
			
			std::string frameName = nameList[nAnimationContents]->d_name;
			std::string framesPath = animationPath + std::string(nameList[nAnimationContents]->d_name);

			LOG("Loaded %s.", framesPath.c_str());

			auto action = std::string(folderList[nCharacterFolder]->d_name);
			action[0] = std::tolower(action[0], std::locale());

			//if it's not the first frame with such name we continue looping
			if(texture->AddFrame(framesPath.c_str(), action) != 1) [[likely]]
				continue;

			//if it's the first frame we set the action animation parameters 
			// (or default them in case they don't exist)
			texture->SetCurrentAnimation(action);
			SetAnimationParameters(animDataNode, action);

			free(nameList[nAnimationContents]);
		}
		free(nameList);
		free(folderList[nCharacterFolder]);
	}
	free(folderList);
}

void Character::CreatePhysBody() 
{ 
	// <physics>
	auto currentNode = parameters.child("physics");
	
	if(currentNode.empty()) [[unlikely]]
	{
		LOG("Entity %s has no physics node", name.c_str());
		return;
	}
	
	type = static_cast<CL::ColliderLayers>(currentNode.attribute("colliderlayers").as_int());
	
	float32 gravity = currentNode.attribute("gravityscale") ? currentNode.attribute("gravityscale").as_float() : 1.0f;
	float32 restitution = currentNode.attribute("restitution") ? currentNode.attribute("restitution").as_float() : 1.0f;
		
	// <properties/> (or <animation> if properties doesn't exist
	if(currentNode = currentNode.parent().child("animationdata").first_child();
	   currentNode.empty())
	{
		LOG("No animationdata on %s", name);
		return;
	}

	while(currentNode) 
	{
		if(!currentNode.child("collidergroup").empty()) 
			break;
		currentNode = currentNode.next_sibling();
	}
	
	// <animation> that has the collider child or <null handle> if no node exists
	if(!currentNode)
	{
		LOG("Entity %s has no collider node", name.c_str());
		return;
	}
	
	// <collidergroup>
	for(auto const &colliderGroupNode : currentNode.children("collidergroup"))
	{
		iPoint width_height(
			colliderGroupNode.attribute("width").as_int(),
			colliderGroupNode.attribute("height").as_int()
		);

		if(!pBody)
		{
			colliderOffset = {
				colliderGroupNode.first_child().attribute("x").as_int(),
				colliderGroupNode.first_child().attribute("y").as_int()
			};
			
			auto bodyType = BodyTypeStrToEnum(colliderGroupNode.attribute("class").as_string());
			
			auto bodyPtr = app->physics->CreateBody(
				position + colliderOffset,
				bodyType,
				0.0f,
				{0.00f,0.01f},
				gravity
			);

			auto pBodyPtr = app->physics->CreatePhysBody(
				bodyPtr,
				width_height,
				type
			);

			pBody = std::move(pBodyPtr);
			pBody->listener = this;
		}
		
		for(auto const &elem : colliderGroupNode.children())
		{
			bool bSensor = currentNode.attribute("sensor").as_bool();
			float32 density = currentNode.attribute("density") ? currentNode.attribute("density").as_float() : 0.0f;

			// iterate over digits in node and add them to a b2Vec2 as x, y.
			// Will be used on shape creation
			
			std::vector<b2Vec2> tempData;
			std::string shapeType = elem.name();

			if(StrEquals(shapeType, "chain") || StrEquals(shapeType, "polygon"))
			{
				const std::string xyStr = elem.attribute("points").as_string();
				static const std::regex r(R"((-?\d{1,3})(?:\.\d+)*,(-?\d{1,3})(?:\.\d+)*)");
				auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
				auto xyStrEnd = std::sregex_iterator();

				for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
				{
					std::smatch match = *i;
					tempData.push_back(
						{
							PIXEL_TO_METERS(stoi(match[1].str())),
							PIXEL_TO_METERS(stoi(match[2].str()))
						}
					);
				}
			}
			else if(StrEquals(shapeType, "rectangle"))
			{
				tempData.push_back(
					{
						PIXEL_TO_METERS(width_height.x),
						PIXEL_TO_METERS(width_height.y)
					}
				);
			}
			else if(StrEquals(shapeType, "circle"))
			{
				float32 radius = (elem.attribute("radius").empty())
					? colliderGroupNode.attribute("radius").as_float()
					: elem.attribute("radius").as_float();

				tempData.push_back(
					{
						radius,
						0
					}
				);
			}

			// If there was no points, xml is malformed
			// We continue the loop to not crash the game
			if(tempData.empty()) continue;

			// Create the Shape
			ShapeData shape(shapeType, tempData);

			// Fix position if shape is a circle
			b2Vec2 fixPos(0, 0);
			if(shape.shape.get()->GetType() == b2Shape::e_circle)
			{
				fixPos = PIXEL_TO_METERS(
					{
						elem.attribute("x").as_int() - colliderOffset.x,
						elem.attribute("y").as_int() - colliderOffset.y
					}
				);
			}

			float32 friction = elem.attribute("friction") ? elem.attribute("friction").as_float() : 1.0f;
			uint16 maskFlag = SetMaskFlag(name, colliderGroupNode, elem);

			auto fixtureDef = app->physics->CreateFixtureDef(
				shape,
				static_cast<uint16>(type),
				maskFlag,
				bSensor,
				density,
				friction,
				restitution,
				fixPos
			);

			auto fixturePtr = pBody->body->CreateFixture(fixtureDef.get());

			if(StrEquals(elem.attribute("name").as_string(), "ground"))
			{
				pBody->ground = std::make_unique<FixtureData>(
					std::string(elem.attribute("name").as_string()),
					fixturePtr
				);
			}
			if(StrEquals(elem.attribute("name").as_string(), "top"))
			{
				pBody->top = std::make_unique<FixtureData>(
					std::string(elem.attribute("name").as_string()),
					fixturePtr
				);
			}
		}
	}
}

void Character::RestartLevel()
{
	position = startingPosition;
	hp = 3;
	iFrames = 0;
	jump.bOnAir = false;
	jump.currentJumps = 0;
	bKeepMomentum = false;
	velocityToKeep = {0,0};
	SpecificRestart();
}

//---------- Main Loop

bool Character::Update()
{
	if(iFrames > 0)
	{
		iFrames++;
		if(iFrames >= 60)
		{
			iFrames = 0;
		}
	}
	//Update Character position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x);
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y);
	app->render->DrawCharacterTexture(
		texture->UpdateAndGetFrame().get(),
		iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
		(bool)dir,
		texture->GetFlipPivot()
	);

	if(hp <= 0) Disable();

	return true;
}

bool Character::Pause() const
{
	if(auto tex = texture->GetCurrentTexture(); tex)
	{
		return app->render->DrawCharacterTexture(
			texture->GetCurrentTexture().get(),
			iPoint(position.x - colliderOffset.x, position.y - colliderOffset.y),
			(bool)dir,
			texture->GetFlipPivot()
		);
	}
	return false;
}


//---------- Collisions

void Character::BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
{
	/* To override */
}

void Character::OnCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB)
{
	/* To override */
}

//---------- Utils

bool Character::CreateEntityPath(std::string &entityFolder) const
{
	if(!parameters.attribute("class").empty())
		entityFolder = std::string(parameters.attribute("class").as_string()) + "/";

	if(!parameters.attribute("level").empty())
		entityFolder = std::string(parameters.attribute("level").as_string()) + "/" + entityFolder;

	if(!parameters.attribute("name").empty())
		entityFolder = std::string(parameters.attribute("name").as_string()) + "/" + entityFolder;

	if(!parameters.parent().attribute("texturepath").empty())
		entityFolder = std::string(parameters.parent().attribute("texturepath").as_string()) + entityFolder;

	if(entityFolder.empty())
	{
		LOG("No animation folder specified for %s", name);
		return false;
	}

	return true;
}

void Character::SetAnimationParameters(pugi::xml_node const &animDataNode, std::string const &action) const
{
	auto animationParameters = animDataNode.find_child_by_attribute("name", action.c_str());

	if(!animationParameters.empty() && animationParameters.attribute("speed"))
		texture->SetSpeed(animationParameters.attribute("speed").as_float());
	else
		texture->SetSpeed(0.2f);

	if(!animationParameters.empty() && animationParameters.attribute("style"))
		texture->SetAnimStyle(static_cast<AnimIteration>(animationParameters.attribute("animstyle").as_int()));
	else
		texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);

}

uint16 Character::SetMaskFlag(std::string_view name, pugi::xml_node const &colliderGroupNode, pugi::xml_node const &colliderNode) const
{
	using enum CL::ColliderLayers;
	uint16 maskFlag = 0x0001;
	if(StrEquals(name, "player"))
	{
		if(StrEquals(colliderGroupNode.attribute("name").as_string(), "CharacterSensor"))
			maskFlag = static_cast<uint16>(ENEMIES | TRIGGERS | CHECKPOINTS | BULLET);
		else if(StrEquals(colliderGroupNode.attribute("name").as_string(), "Terrain"))
		{
			if(StrEquals(colliderNode.attribute("name").as_string(), "Ground") ||
			   StrEquals(colliderNode.attribute("name").as_string(), "holdLeft") ||
			   StrEquals(colliderNode.attribute("name").as_string(), "holdRight"))
				maskFlag = static_cast<uint16>(ENEMIES | PLATFORMS | ITEMS);
			else
				maskFlag = static_cast<uint16>(ENEMIES | ITEMS);

		}
	}
	else if(StrEquals(name, "enemy"))
	{
		if(StrEquals(colliderGroupNode.attribute("name").as_string(), "CharacterSensor"))
			maskFlag = static_cast<uint16>(TRIGGERS | PLAYER | BULLET | ENEMIES);
		else if(StrEquals(colliderGroupNode.attribute("name").as_string(), "Terrain"))
			maskFlag = static_cast<uint16>(PLATFORMS | PLAYER | ENEMIES);
	}
	return maskFlag;
}

/*
 *	Let's just say that today is november 24th. Tomorrow is my birthday. It's 4.13 AM. I can't even think.
 *	BUT I'm just gonna leave this here and hope that I can fix it later.
 *	I'm so sorry about this. But hey, at least it works.
 *	If you have any issue don't hesitate to contact me. I'll be happy to help you.
 *	r(R"((([a-zA-Z]+(?:_??(?:(?!(?:_image|_static|(?:_*?anim\d+)|\d+)(?:\.png|\.jpg)))[a-zA-Z]*))_?(?:(image|static|(?:anim\d+)|\d+))(\d+)*(?:\.png|\.jpg)))")
 *
 *  haHA it didn't work. Improved version. Only took 8 minutes to fix it. I'm so proud of myself.
 *  Overall I've been working on this regex for 3 hours and a half. I'm surprised it took so little.
 *  https://regex101.com/r/8CRsUh/1
 *  https://regex101.com/r/viuXac/2
 *  final ? https://regex101.com/r/pdLoYK/1
 *  https://regex101.com/r/VDIgWc/1
 *  static const std::regex r(R"((([a-zA-Z]+(?:_??(?:(?!(?:_image|_static|(?:_*?anim\d+)|\d+)(?:\.png|\.jpg)))[a-zA-Z]*))_?
 *	 						  (?:(image|static|(?:anim(?:\d+)*?)|\d+))(\d+)*(?:\.png|\.jpg))))")  
 *  and im not even gonna use it.........................
*/
