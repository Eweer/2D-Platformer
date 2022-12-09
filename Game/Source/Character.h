#pragma once
#include "Entity.h"

struct CharacterJump
{
	bool bJumping = false;
	int currentJumps = 0;
	int maxJumps = 0;
	int timeSinceLastJump = 0;
	float jumpImpulse = 0.0f;
	bool bInAir = false;
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

	bool Pause() const override;

	bool CleanUp() override;
	
	void ResetScore();

	uint GetScore() const;

	void AddMultiplier(uint n);

	int GetTimeUntilReset() const;

	std::pair<uint, uint> GetScoreList() const;

	void CreatePhysBody() override;

	void SetStartingPosition();

	void AddTexturesAndAnimationFrames() override;
	
	virtual void jumpOnNextUpdate(bool bStartJumpOnUpdate) { /*Method to Override*/ };

	uint16 SetMaskFlag(std::string_view name, pugi::xml_node const &colliderGroupNode, pugi::xml_node const &colliderNode);
	
	void OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) override;
	void BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) override;
	float score = 0;
	uint scoreMultiplier = 1;
	std::pair<uint, uint> scoreList;

	uint hp = 3;

	SDL_Texture *hpTexture = nullptr;

	int timeUntilReset = -1;

	int dir = 1;

	std::string currentCharacter = "";
	SDL_Point textureOffset;
};
