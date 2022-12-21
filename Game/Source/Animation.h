#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#pragma warning( disable : 4018 )

#include "App.h"
#include "Textures.h"

#include "Defs.h"
#include "Point.h"

#include <vector>
#include <locale>

#include "SDL/include/SDL_rect.h"

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

	std::shared_ptr<SDL_Texture> UpdateAndGetFrame()
	{
		if(TimeSinceLastFunctionCall > 0) TimeSinceLastFunctionCall += 0.1f;
		if(TimeSinceLastFunctionCall > FunctionCooldown) TimeSinceLastFunctionCall = 0;
		if(GetFrameCount() == -1) return nullptr;

		//if it's not active, we just return frame
		if(!bActive) return frames[currentAnimName][(uint)currentFrame];

		//if it's active and finished, it's no longer finished
		if(bFinished)
			bFinished = !bFinished;

		//if it's active we increase the frame
		currentFrame += frameSpeed.at(currentAnimName);

		//if no more animations in frames[current + 1]
		if((uint)currentFrame >= frames.at(currentAnimName).size()|| (int)currentFrame < 0)
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
					if(frameSpeed.at(currentAnimName) > 0) currentFrame = (float)frames.at(currentAnimName).size() - 1;
					else Stop();

					frameSpeed.at(currentAnimName) *= -1;
					break;
				}
				case LOOP_FROM_START:
				{
					currentFrame = 0.0f;
					break;
				}
				case LOOP_FORWARD_BACKWARD:
				{
					currentFrame = (frameSpeed.at(currentAnimName) < 0) ? 0 : (float)frames.at(currentAnimName).size() - 1;
					frameSpeed.at(currentAnimName) *= -1;
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
		if(currentAnimName == name) return;
		if(frames.contains(name))
		{
			currentAnimName = name;
			currentFrame = 0.0f;
		}
	}

	std::shared_ptr<SDL_Texture> GetCurrentTexture() const
	{
		if(currentAnimName.empty()) return nullptr;
		if(frames.empty()) return nullptr;
		if(!frames.contains(currentAnimName)) return nullptr;
		if(frames.at(currentAnimName).empty()) return nullptr;

		if((int)currentFrame >= frames.at(currentAnimName).size())
			return frames.at(currentAnimName).at(frames.at(currentAnimName).size() - 1);
		else
			return frames.at(currentAnimName).at((uint)currentFrame);
	}

	std::shared_ptr<SDL_Texture> GetAnimationByName(std::string_view name) const
	{
		if(auto tex = frames.find(name); tex != frames.end() && !tex->second.empty())
			return tex->second.front();
		return nullptr;
	}

	int GetCurrentFrame() const
	{
		return static_cast<int>(currentFrame);
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
		name[0] = std::tolower(animName[0], std::locale());
		frames[name].push_back(app->tex->Load(pathToPNG));
		if(currentAnimName == "unknown") currentAnimName = name;
		return GetFrameCount(name);
	}
	
	Animation *AddSingleFrame(std::shared_ptr<SDL_Texture> texture)
	{
		frames["unknown"].push_back(texture);
		return this;
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

	float GetFloatCurrentFrame() const
	{
		return currentFrame;
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
		frameSpeed[currentAnimName] = animSpeed;
	}

	float GetSpeed() const
	{
		return frameSpeed.at(currentAnimName);
	}

	void SetAnimStyle(int i)
	{
		currentStyle = static_cast<AnimIteration>(i);
	}

	void SetCurrentFrame(int i)
	{
		if(in_range(i, 0, frames[currentAnimName].size()))
		{
			currentFrame = floor(static_cast<float>(i));
		}
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
		if(currentAnimName.empty()) return "unknown";
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
	
	void DoLoopsOfAnimation(uint loops, AnimIteration style)
	{
		if(loops <= 0) return;
		Reset();
		SetAnimStyle(style);
		loopsToDo = loops;
		Start();
	}

	void setPivot(SDL_Point const &p)
	{
		animPivot = {
			.x = p.x,
			.y = p.y
		};
	}

	int GetCurrentIndex() const
	{
		return static_cast<int>(floor(currentFrame));
	}

	std::vector<std::shared_ptr<SDL_Texture>>GetAnim(std::string_view anim)
	{
		if(auto it = frames.find(anim); it != frames.end())
			return it->second;
		return std::vector<std::shared_ptr<SDL_Texture>>();
	}

private:
	float FunctionCooldown = 1.1f;
	float TimeSinceLastFunctionCall = 0;
	float currentFrame = 0;
	std::string currentAnimName = "unknown";
	AnimIteration currentStyle = AnimIteration::NEVER;
	AnimIteration baseStyle = AnimIteration::NEVER;
	bool bActive = false;
	bool bFinished = false;
	SDL_Point animPivot = {.x = 0, .y = 0};
	uint loopsToDo = 0;
	uint width = 0;
	uint height = 0;
	std::unordered_map<std::string, std::vector<std::shared_ptr<SDL_Texture>>, StringHash, std::equal_to<>> frames;
	std::unordered_map<std::string, float, StringHash, std::equal_to<>> frameSpeed;
	std::shared_ptr<SDL_Texture> staticImage = nullptr;
};
#endif	// __ANIMATION_H__