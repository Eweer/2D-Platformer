#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "Entity.h"

#include "Physics.h"

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
	//---------- Constructors
	explicit Character();
	explicit Character(const pugi::xml_node &itemNode);
	~Character() override;

	//---------- Load Parameters
	bool Awake() override;
	void SetPaths();
	void InitializeTexture() const;

	//---------- Create character
	bool Start() override;
	void AddTexturesAndAnimationFrames();
	void CreatePhysBody() override;

	//---------- Main Loop
	bool Update() override;
	bool Pause() const override;

	//---------- Destroy Entity
	bool CleanUp() override;
	
	//---------- Collisions
	void OnCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) override;
	void BeforeCollisionStart(b2Fixture *fixtureA, b2Fixture *fixtureB, PhysBody *pBodyA, PhysBody *pBodyB) override;

	uint hp = 3;
	int dir = 1;
	SDL_Point textureOffset = {0,0};

private:
	//---- Utils
	bool CreateEntityPath(std::string &entityFolder) const;
	void SetAnimationParameters(pugi::xml_node const &animDataNode, std::string const &action);
	uint16 SetMaskFlag(
		std::string_view name,
		pugi::xml_node const &colliderGroupNode,
		pugi::xml_node const &colliderNode
	) const;

};

#endif // __CHARACTER_H__
