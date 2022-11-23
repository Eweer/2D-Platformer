#pragma once
#include "Entity.h"

class Character : public Entity
{
public:

	explicit Character();

	explicit Character(EntityType type);

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

	float score = 0;
	uint scoreMultiplier = 1;
	std::pair<uint, uint> scoreList;

	uint hp = 3;

	SDL_Texture *hpTexture = nullptr;

	int timeUntilReset = -1;
};
