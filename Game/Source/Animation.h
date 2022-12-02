#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#pragma warning( disable : 4018 )

#include "App.h"
#include "Textures.h"
#include "Defs.h"
#include "Point.h"

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <cctype>

struct SDL_Texture;

enum class AnimIteration
{
	ONCE = 0,
	LOOP_FROM_START,
	FORWARD_BACKWARD,
	LOOP_FORWARD_BACKWARD,
	NEVER,
	UNKNOWN
};

class Animation
{
public:

	Animation() = default;

	~Animation() = default;

	SDL_Texture *UpdateAndGetFrame()
	{
		if(TimeSinceLastFunctionCall > 0) TimeSinceLastFunctionCall += 0.1f;
		if(TimeSinceLastFunctionCall > FunctionCooldown) TimeSinceLastFunctionCall = 0;
		if(GetFrameCount() == -1) return nullptr;

		//if it's not active, we just return frame
		if(!bActive) return frames[currentAnimName][(uint)currentFrame];

		//if it's active and finished, it's no longer finished
		if(bFinished) bFinished = !bFinished;

		//if it's active we increase the frame
		currentFrame += speed;

		//if no more animations in frames[current + 1]
		if((uint)currentFrame >= frames.at(currentAnimName).size() + 1 || (int)currentFrame < 0)
		{
			//we do things
			switch(currentStyle)
			{
				using enum AnimIteration;
				case ONCE:
				{
					Stop();
					break;
				}
				case FORWARD_BACKWARD:
				{
					if(speed > 0) currentFrame = (float)frames.at(currentAnimName).size() - 1;
					else Stop();

					speed *= -1;
					break;
				}
				case LOOP_FROM_START:
				{
					currentFrame = 0.0f;
					break;
				}
				case LOOP_FORWARD_BACKWARD:
				{
					currentFrame = (speed < 0) ? 0 : (float)frames.at(currentAnimName).size() - 1;
					speed *= -1;
					break;
				}
				default:
					Stop();
			}
			bFinished = true;
			if(loopsToDo > 0)
			{
				loopsToDo--;
				if(loopsToDo == 0)
				{
					Stop();
					SetAnimStyle(baseStyle);
				}
			}
		}
		if((int)currentFrame >= frames.at(currentAnimName).size())
			return frames[currentAnimName][frames.at(currentAnimName).size() - 1];
		else
			return frames[currentAnimName][(uint)currentFrame];
	}

	void SetCurrentAnimation(std::string const &name)
	{
		if(frames.contains(name))
		{
			currentAnimName = name;
			currentFrame = 0.0f;
		}
	}

	SDL_Texture *GetCurrentFrame() const
	{
		if((int)currentFrame >= frames.at(currentAnimName).size())
			return frames.at(currentAnimName).at(frames.at(currentAnimName).size() - 1);
		else
			return frames.at(currentAnimName).at((uint)currentFrame);
	}

	Animation *AddStaticImage(const char *pathToPNG)
	{
		staticImage = app->tex->Load(pathToPNG);
		return this;
	}

	Animation *AddSingleFrame(const char *pathToPNG)
	{
		frames["unknown"].push_back(app->tex->Load(pathToPNG));
		return this;
	}
	
	//returns the number of frames with key name after inserting the new one
	//returns -1 if name couldn't be emplaced.
	int AddFrame(const char *pathToPNG, const std::string &animName)
	{
		std::string name = animName;
		name[0] = (char)std::tolower(animName[0]);
		frames[name].emplace_back(std::move(app->tex->Load(pathToPNG)));
		if(currentAnimName == "unknown") currentAnimName = name;
		return GetFrameCount(name);
	}
	
	Animation *AddSingleFrame(SDL_Texture *texture)
	{
		frames["unknown"].push_back(texture);
		return this;
	}

	bool CleanUp()
	{
		for(auto const &[key, vec]: frames)
		{
			for(auto &elem : vec)
			{
				app->tex->UnLoad(elem);
			}
		}
		if(staticImage) app->tex->UnLoad(staticImage);
		return true;
	}

	bool GetAnimFinished() const
	{
		return bFinished;
	}

	void AdvanceFrame()
	{
		if(TimeSinceLastFunctionCall > FunctionCooldown || TimeSinceLastFunctionCall == 0)
		{
			if((int)currentFrame < frames.at(currentAnimName).size() - 1) currentFrame++;
			else currentFrame = 0;
			TimeSinceLastFunctionCall += 0.1f;
		}
	}
	
	//returns -1 if currentAnimName doesn't exist
	//otherwise it returns the number of frames in the vector
	int GetFrameCount() const
	{
		if(frames.contains(currentAnimName))
			return frames.at(currentAnimName).size();
		else
			return -1;
	}

	//returns -1 if currentAnimName doesn't exist
	//otherwise it returns the number of frames in the vector
	int GetFrameCount(std::string const &name) const
	{
		if(frames.contains(name))
			return frames.at(name).size();
		else
			return -1;
	}

	void SetSpeed(float const &animSpeed)
	{
		speed = animSpeed;
	}

	float GetSpeed() const
	{
		return speed;
	}

	void SetAnimStyle(int i)
	{
		currentStyle = static_cast<AnimIteration>(i);
	}

	void SetAnimStyle(AnimIteration i)
	{
		currentStyle = i;
		if(currentStyle == AnimIteration::LOOP_FORWARD_BACKWARD || currentStyle == AnimIteration::LOOP_FROM_START)
		{
			Start();
		}
	}

	AnimIteration GetAnimStyle() const
	{
		return currentStyle;
	}

	std::string GetCurrentAnimName() const
	{
		return currentAnimName;
	}
	
	SDL_Point GetFlipPivot() const
	{
		return animPivot;
	}

	void Start()
	{
		bActive = true;
	}
	
	bool Start(std::string const &name)
	{
		if(GetFrameCount(name) > 0)
		{ 
			SetCurrentAnimation(name);
			bActive = true;
			return true;
		}
		return false;
	}

	void Pause()
	{
		bActive = false;
	}

	void Reset()
	{
		bFinished = false;
		currentFrame = 0;
	}

	void Stop()
	{
		Pause();
		Reset();
	}

	bool IsAnimFinished() const
	{
		return bFinished;
	}

	bool IsLastFrame() const
	{
		return (uint)currentFrame == frames.at(currentAnimName).size() - 1;
	}
	
	iPoint GetFrameSize() const
	{
		/*iPoint ret;
		SDL_Texture *aux = frames[currentAnimName][0];
		SDL_QueryTexture, nullptr, nullptr, &ret.x, &ret.y);
		return ret;*/
	}

	void DoLoopsOfAnimation(uint loops, AnimIteration style)
	{
		if(loops <= 0) return;
		Reset();
		SetAnimStyle(style);
		loopsToDo = loops;
		Start();
	}

	void setPivot(const int &x, const int &y)
	{
		animPivot.x = x;
		animPivot.y = y;
	}

private:
	float FunctionCooldown = 1.1f;
	float TimeSinceLastFunctionCall = 0;
	float speed = 0;
	float currentFrame = 0;
	std::string currentAnimName = "unknown";
	AnimIteration currentStyle = AnimIteration::NEVER;
	AnimIteration baseStyle = AnimIteration::NEVER;
	bool bActive = false;
	bool bFinished = false;
	SDL_Point animPivot = {0, 0};
	uint loopsToDo = 0;
	uint width = 0;
	uint height = 0;
	std::unordered_map<std::string, std::vector<SDL_Texture *>, StringHash, std::equal_to<>> frames;
	SDL_Texture *staticImage = nullptr;
};
#endif	// __ANIMATION_H__