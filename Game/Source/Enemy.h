#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Character.h"

class Enemy : public Character
{
public:
	Enemy();
	explicit Enemy(const pugi::xml_node &itemNode, int newId);
	~Enemy() final;

	bool Awake() override;
};

#endif // __ENEMY_H__
