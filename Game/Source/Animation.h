#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "App.h"
#include "Textures.h"
#include "Defs.h"

#include <vector>
#include <memory>

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

	SDL_Texture *GetCurrentFrame()
	{
		if(TimeSinceLastFunctionCall > 0) TimeSinceLastFunctionCall += 0.1f;
		if(TimeSinceLastFunctionCall > FunctionCooldown) TimeSinceLastFunctionCall = 0;

		//if it's not active, we just return frame
		if(!bActive) return frames[(int)currentFrame];

		//if it's active and finished, it's no longer finished
		if(bFinished) bFinished = !bFinished;

		//if it's active we increase the frame
		currentFrame += speed;

		//if no more animations in frames[current + 1]
		if((uint)currentFrame >= frames.size() + 1 || (int)currentFrame < 0)
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
					if(speed > 0) currentFrame = (float)frames.size() - 1;
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
					currentFrame = (speed < 0) ? 0 : (float)frames.size() - 1;
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
		if((int)currentFrame >= frames.size())
			return frames[(int)frames.size() - 1];
		else
			return frames[(int)currentFrame];
	}

	Animation *AddStaticImage(const char *pathToPNG)
	{
		staticImage = app->tex->Load(pathToPNG);
		return this;
	}

	Animation *AddSingleFrame(const char *pathToPNG)
	{
		frames.push_back(app->tex->Load(pathToPNG));
		return this;
	}

	Animation *AddSingleFrame(SDL_Texture *texture)
	{
		frames.push_back(texture);
		return this;
	}

	bool CleanUp()
	{
		for(auto &elem : frames) app->tex->UnLoad(elem);
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
			if((int)currentFrame < frames.size() - 1) currentFrame++;
			else currentFrame = 0;
			TimeSinceLastFunctionCall += 0.1f;
		}
	}
	
	uint GetFrameCount() const
	{
		return frames.size();
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

	void Start()
	{
		bActive = true;
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
		return (uint)currentFrame == frames.size() - 1;
	}
	
	iPoint GetFrameSize() const
	{
		iPoint ret;
		SDL_QueryTexture(frames[0], nullptr, nullptr, &ret.x, &ret.y);
		return ret;
	}

	void DoLoopsOfAnimation(uint loops, AnimIteration style)
	{
		if(loops <= 0) return;
		Reset();
		SetAnimStyle(style);
		loopsToDo = loops;
		Start();
	}

private:
	float FunctionCooldown = 1.1f;
	float TimeSinceLastFunctionCall = 0;
	float speed = 0;
	float currentFrame = 0;
	AnimIteration currentStyle = AnimIteration::NEVER;
	AnimIteration baseStyle = AnimIteration::NEVER;
	bool bActive = false;
	bool bFinished = false;
	uint loopsToDo = 0;
	uint width = 0;
	uint height = 0;
	std::vector<SDL_Texture *> frames;
	SDL_Texture *staticImage = nullptr;
};
#endif	// __ANIMATION_H__