#pragma once
#include "Entity.h"

struct entityJump
{
	bool bJumping;
	int currentJumps;
	int maxJumps;
	int timeSinceLastJump;
	float jumpImpulse;
};

class Character : public Entity
{
public:

	explicit Character();

	explicit Character(ColliderLayers type);

	explicit Character(const pugi::xml_node &itemNode);

	~Character() override;

	bool Awake() override;

	bool Start() override;

	bool Update() override;

	bool CleanUp() override;

	void OnCollision(PhysBody *physA, PhysBody *physB) override;

	void ResetScore();

	uint GetScore() const;

	void AddMultiplier(uint n);

	int GetTimeUntilReset() const;

	std::pair<uint, uint> GetScoreList() const;
	
	void SetStartingPosition();

	void AddTexturesAndAnimationFrames() override;
	
	virtual void jumpOnNextUpdate(bool bStartJumpOnUpdate) {};

	float score = 0;
	uint scoreMultiplier = 1;
	std::pair<uint, uint> scoreList;

	uint hp = 3;

	SDL_Texture *hpTexture = nullptr;

	int timeUntilReset = -1;

	int dir = 1;

	std::string currentCharacter = "";
};
