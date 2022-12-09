#ifndef __UI_H__
#define __UI_H__

#include "Module.h"

#include "Defs.h"
#include "Point.h"

class UI : public Module
{
public:

	UI();

	~UI() final;

	// Called before render is available
	bool Awake(pugi::xml_node &) final;

	// Called before the first frame
	bool Start() final;

	// Called each loop iteration
	bool PreUpdate() final;

	// Called each loop iteration
	bool Update(float dt) final;

	// Called each loop iteration
	bool PostUpdate() final;

	void DrawFPS(iPoint &position) const;

	// Called before quitting
	bool CleanUp() final;

private:
	int fCleanCraters = 0;
	iPoint pTopLeft = {10, 10};
};

#endif
