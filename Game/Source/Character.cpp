#include "Character.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"
#include "Animation.h"


constexpr uint Character_SIZE = 30;

Character::Character() : Entity(ColliderLayers::UNKNOWN) {}

Character::Character(ColliderLayers type) : Entity(type) {}

Character::Character(pugi::xml_node const &itemNode = pugi::xml_node()) : Entity(itemNode) {}

Character::~Character() = default;

bool Character::Awake()
{
	SetStartingParameters();
	return true;
}

bool Character::Start()
{
	//initilize textures
	CreatePhysBody();
	if(!texture->Start("idle")) LOG("Couldnt start %s anim. idle was not mapped", texture->GetCurrentAnimName());

	return true;
}

bool Character::Update()
{
	
	//Update Character position in pixels
	position.x = METERS_TO_PIXELS(pBody->body->GetTransform().p.x) - Character_SIZE/2;
	position.y = METERS_TO_PIXELS(pBody->body->GetTransform().p.y) - Character_SIZE/2;

	app->render->DrawTexture(texture->GetCurrentFrame(), position.x, position.y);

	for(int i = 0; i < hp; i++)
	{
		app->render->DrawTexture(texture->GetCurrentFrame(), 710, 930 - i*(Character_SIZE + 10));
	}

	return true;
}

bool Character::CleanUp()
{
	//if(renderMode == RenderModes::IMAGE) app->tex->UnLoad(texture.image);
	texture->CleanUp();
	return true;
}

void Character::OnCollision(PhysBody *physA, PhysBody *physB)
{
	if(timeUntilReset >= 0) return;
	switch(physB->ctype)
	{
		case ColliderLayers::ITEMS:
			if(score < 99999)
			{
				score += (float)(100 * scoreMultiplier);
				if(score > 99999) score = 99999;
			}
			LOG("Collision ITEMS");
			break;
		case ColliderLayers::UNKNOWN:
			LOG("Collision UNKNOWN");
			break;
		case ColliderLayers::PLATFORMS:
			LOG("Collision BOARD");
			break;
		default:
			LOG("HOW DID YOU GET HERE?!?!?!?");
	}
}

void Character::ResetScore()
{
	score = 0;
}

uint Character::GetScore() const
{
	return (uint)score;
}

void Character::AddMultiplier(uint n)
{
	scoreMultiplier += n;
}

int Character::GetTimeUntilReset() const
{
	return timeUntilReset;
}

std::pair<uint, uint> Character::GetScoreList() const
{
	return scoreList;
}

void Character::SetStartingPosition()
{
	if(pBody->body) app->physics->DestroyBody(pBody->body);
	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	CreatePhysBody();
}

void Character::AddTexturesAndAnimationFrames()
{
	texture = std::make_unique<Animation>();

	pugi::xml_node animDataNode = parameters.child("animationdata");

	if(!parameters.attribute("renderable").as_bool())
	{
		renderMode = RenderModes::NO_RENDER;
		return;
	}
	
	std::string entityFolder;
	if(animDataNode.attribute("animpath"))
	{
		entityFolder = animDataNode.attribute("animpath").as_string();
	}
	else if(animDataNode.attribute("name"))
	{
		entityFolder = animDataNode.attribute("name").as_string();
	}
	else
	{
		entityFolder = texLevelPath + name + "/";
		LOG("No animation folder specified for %s, defaulting to %s", name, entityFolder);
	}

	if(currentCharacter != "") entityFolder += currentCharacter +"/";
	
	texture->setPivot(animDataNode.attribute("pivotx").as_int(), animDataNode.attribute("pivoty").as_int());
	
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

			//if it's not the first frame with such name we continue looping
			if(texture->AddFrame(framesPath.c_str(), std::string(folderList[nCharacterFolder]->d_name)) != 1) [[likely]]
				continue;

			//if we have multiple frames we set renderMode to animation
			if(renderMode == RenderModes::UNKNOWN) [[unlikely]]
				renderMode = RenderModes::ANIMATION;

			if(parameters.child("animation").attribute("speed"))
				texture->SetSpeed(parameters.child("animation").attribute("speed").as_float());
			else
				texture->SetSpeed(0.2f);

			if(parameters.child("animation").attribute("style"))
				texture->SetAnimStyle(static_cast<AnimIteration>(parameters.child("animation").attribute("animstyle").as_int()));
			else
				texture->SetAnimStyle(AnimIteration::LOOP_FROM_START);

			free(nameList[nAnimationContents]);
		}
		free(nameList);
		free(folderList[nCharacterFolder]);
	}
	free(folderList);
}

//	Let's just say that today is november 24th. Tomorrow is my birthday. It's 4.13 AM. I can't even think.
//	BUT I'm just gonna leave this here and hope that I can fix it later.
//	I'm so sorry about this. But hey, at least it works.
//	If you have any issue don't hesitate to contact me. I'll be happy to help you.
//	r(R"((([a-zA-Z]+(?:_??(?:(?!(?:_image|_static|(?:_*?anim\d+)|\d+)(?:\.png|\.jpg)))[a-zA-Z]*))_?(?:(image|static|(?:anim\d+)|\d+))(\d+)*(?:\.png|\.jpg)))"); 

// haHA it didn't work. Improved version. Only took 8 minutes to fix it. I'm so proud of myself.
// Overall I've been working on this regex for 3 hours and a half. I'm surprised it took so little.
// https://regex101.com/r/8CRsUh/1
// https://regex101.com/r/viuXac/2
// final ? https://regex101.com/r/pdLoYK/1
// https://regex101.com/r/VDIgWc/1
//static const std::regex r(R"((([a-zA-Z]+(?:_??(?:(?!(?:_image|_static|(?:_*?anim\d+)|\d+)(?:\.png|\.jpg)))[a-zA-Z]*))_?(?:(image|static|(?:anim(?:\d+)*?)|\d+))(\d+)*(?:\.png|\.jpg))
//))"); //std::smatch m;
		//and im not even gonna use it.........................

