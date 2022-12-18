#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "Entity.h"

#include "Physics.h"

struct CharacterJump
{
	bool bOnAir = false;
	int currentJumps = 0;
	int maxJumps = 0;
	float jumpImpulse = 0.0f;
};

class Character : public Entity
{
public:
	//---------- Constructors
	explicit Character();
	explicit Character(const pugi::xml_node &itemNode, int newId);
	~Character() override;

	//---------- Load Parameters
	void SetPaths();
	void InitializeTexture() const;

	//---------- Create character
	bool Start() override;
	void AddTexturesAndAnimationFrames();
	void CreatePhysBody() override;
	void RestartLevel() override;
	//---------- Main Loop
	bool Update() override;
	bool Pause() const override;
	
	//---------- Collisions
	void OnCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) override;
	void BeforeCollisionStart(b2Fixture const *fixtureA, b2Fixture const *fixtureB, PhysBody const *pBodyA, PhysBody const *pBodyB) override;

	int dir = 0;
	SDL_Point textureOffset = {0,0};

	uint hp = 3;
	int iFrames = 0;

	CharacterJump jump;
	bool bKeepMomentum = false;
	b2Vec2 velocityToKeep = {0.0f, 0.0f};

private:
	//---- Utils
	bool CreateEntityPath(std::string &entityFolder) const;
	void SetAnimationParameters(pugi::xml_node const &animDataNode, std::string const &action) const;
	uint16 SetMaskFlag(
		std::string_view name,
		pugi::xml_node const &colliderGroupNode,
		pugi::xml_node const &colliderNode
	) const;

};

#endif // __CHARACTER_H__
