#include "Physics.h"
#include "App.h"
#include "Input.h"
#include "Window.h"
#include "Player.h"
#include "Render.h"

#include "Defs.h"
#include "Log.h"

#include "math.h"

#include <variant>
#include <memory>

#include "Box2D/Box2D/Box2D.h"
#include "SDL/include/SDL_keycode.h"

Physics::Physics() : Module()
{
	name = "physics";
}

// Destructor
Physics::~Physics() = default;

//--------------- 

bool Physics::Start()
{
	LOG("Creating Physics 2D environment");

	// Create a new World
	world = std::make_unique<b2World>(b2Vec2(GRAVITY_X, -GRAVITY_Y));

	// Set this module as a listener for contacts
	world->SetContactListener(this);

	//Setting up so we can use joints
	b2BodyDef bd;
	ground = world->CreateBody(&bd);

	return true;
}

bool Physics::PreUpdate()
{
	float newGrav = b2_maxFloat;
	for (uint keyIterator = SDL_SCANCODE_1; keyIterator <= SDL_SCANCODE_0; keyIterator++)
	{
		if (app->input->GetKey(keyIterator) == KEY_DOWN) 
		{
			newGrav = (float)((int)keyIterator - (int)SDL_SCANCODE_1 + 1);
			
			if (app->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) newGrav *= -1;
			if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) newGrav *= 2;
			
			if (keyIterator == SDL_SCANCODE_0) newGrav = 0;
		}
	}

	if (newGrav != b2_maxFloat)
	{
		b2Vec2 newGravVec;
		if (app->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
			newGravVec = { newGrav, world->GetGravity().y };
		else
			newGravVec = { world->GetGravity().x, newGrav };
		world->SetGravity(newGravVec);
	}

	// Step (update) the World
	if (stepActive || (!stepActive && app->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN))
		world->Step(1.0f / 60.0f, 6, 2);

	if (app->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN) ToggleStep();

	return true;
}

bool Physics::PostUpdate()
{
	// Activate or deactivate debug mode
	if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		debug = !debug;

	if (app->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
		debugWhileSelected = !debugWhileSelected;

	if (!debug) return true;

	//  Iterate all objects in the world and draw the bodies until
	//  there are no more bodies OR
	//  we are dragging an object around AND not debugging draw in the meantime
	for (b2Body *b = world->GetBodyList(); b && (!selected || (selected && debugWhileSelected)); b = b->GetNext())
	{
		for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext())
		{
			if (app->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && IsMouseOverObject(f))
			{
				selected = f->GetBody();
				break;
			}
			switch (f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::Type::e_circle:
				{
					auto const *circleShape = dynamic_cast<b2CircleShape *>(f->GetShape());
					b2Vec2 pos = f->GetBody()->GetPosition() + circleShape->m_p;
					app->render->DrawCircle(METERS_TO_PIXELS(pos), METERS_TO_PIXELS(circleShape->m_radius), SDL_Color(255,255,255,255));
					break;
				}
				// Draw polygons ------------------------------------------------
				case b2Shape::Type::e_polygon:
				{
					auto const *itemToDraw = dynamic_cast<b2PolygonShape *>(f->GetShape());
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, SDL_Color(255, 255, 0, 255));
					break;
				}
				// Draw chains contour -------------------------------------------
				case b2Shape::Type::e_chain:
				{
					auto const *itemToDraw = dynamic_cast<b2ChainShape *>(f->GetShape());
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, SDL_Color(100, 255, 100, 255));
					break;
				}
				// Draw a single segment(edge) ----------------------------------
				case b2Shape::Type::e_edge:
				{
					auto const *edgeShape = dynamic_cast<b2EdgeShape *>(f->GetShape());
					b2Vec2 v1 = b->GetWorldPoint(edgeShape->m_vertex0);
					b2Vec2 v2 = b->GetWorldPoint(edgeShape->m_vertex1);

					app->render->DrawLine(METERS_TO_PIXELS(v1), METERS_TO_PIXELS(v2), SDL_Color(100, 100, 255, 255));
					break;
				}
				case b2Shape::Type::e_typeCount:
				{
					//Info parameter. A shape should never have this type.
					break;
				}
			}
		}
	}

	if (selected) DragSelectedObject();

	return true;
}


//--------------- Called before quitting

bool Physics::CleanUp()
{
	LOG("Destroying physics world");

	world.reset();
	
	return true;
}


//--------------- Collisions

void Physics::BeginContact(b2Contact *contact)
{
	// our custom PhysBody classes
	auto pBodyA = static_cast<PhysBody *>(contact->GetFixtureA()->GetBody()->GetUserData());
	auto pBodyB = static_cast<PhysBody *>(contact->GetFixtureB()->GetBody()->GetUserData());

	if(pBodyA && pBodyB)
	{	
		if(auto [i, success] = collisionMap[pBodyA->body].insert(pBodyB->body);
		   success)
		{
			collisionMap[pBodyB->body].insert(pBodyA->body);
			auto fA = contact->GetFixtureA();
			auto fB = contact->GetFixtureB();

			if(pBodyA->listener) pBodyA->listener->BeforeCollisionStart(fA, fB, pBodyA, pBodyB);
			if(pBodyB->listener) pBodyB->listener->BeforeCollisionStart(fB, fA, pBodyB, pBodyA);
		}
		else contact->SetEnabled(false);
	}
}

void Physics::EndContact(b2Contact *contact)
{
	// our custom PhysBody classes
	auto pBodyA = static_cast<PhysBody *>(contact->GetFixtureA()->GetBody()->GetUserData());
	auto pBodyB = static_cast<PhysBody *>(contact->GetFixtureB()->GetBody()->GetUserData());

	if(pBodyA && pBodyB)
	{
		auto itA = pBodyA->body->GetContactList();
		auto itB = pBodyB->body->GetContactList();
		
		while(itA && itB)
		{
			// if the current iterator contact is not the contact we are ending 
			// AND the contact is not touching
			// AND the other body of the current iterator contact is the same
			// as the one we are ending contact right now we don't end the collision
			if((itA->contact->IsTouching() && itA->other->GetUserData() == pBodyB) ||
			   (itB->contact->IsTouching() && itB->other->GetUserData() == pBodyA))
			{
				return;
			}
			itA = itA->next;
			itB = itB->next;
		}
		
		// if we got here, it means there are no more contacts in the list
		// we erase the bodies of our custom collisionSet and collisionMap
		// and, if there is a listener, send the OnCollisionEnd function
		if(auto i = collisionMap.find(pBodyA->body); i != collisionMap.end())
		{
			i->second.erase(pBodyB->body);
			if(collisionMap.at(pBodyA->body).empty()) collisionMap.erase(i);
			if(pBodyA->listener) pBodyA->listener->OnCollisionEnd(pBodyA, pBodyB);
		}
		
		if(auto i = collisionMap.find(pBodyB->body); i != collisionMap.end())
		{
			i->second.erase(pBodyA->body);
			if(collisionMap.at(pBodyB->body).empty()) collisionMap.erase(i);
			if(pBodyB->listener) pBodyB->listener->OnCollisionEnd(pBodyB, pBodyA);
		}
	}
}

void Physics::PreSolve(b2Contact *contact, const b2Manifold *oldManifold)
{
	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);
	// our custom PhysBody classes
	auto pBodyA = static_cast<PhysBody *>(contact->GetFixtureA()->GetBody()->GetUserData());
	auto pBodyB = static_cast<PhysBody *>(contact->GetFixtureB()->GetBody()->GetUserData());

	if(pBodyA && pBodyB)
	{
		auto fA = contact->GetFixtureA();
		auto fB = contact->GetFixtureB();

		if(pBodyA->listener) pBodyA->listener->OnCollisionStart(fA, fB,pBodyA, pBodyB);
		if(pBodyB->listener) pBodyB->listener->OnCollisionStart(fB, fA, pBodyB, pBodyA);
	}
	/*
	auto bodyA = contact->GetFixtureA()->GetBody();
	auto bodyB = contact->GetFixtureB()->GetBody();

	if(bodyA->GetFixtureList()->GetFilterData().categoryBits & (uint)ColliderLayers::PLATFORMS ||
	   bodyB->GetFixtureList()->GetFilterData().categoryBits & (uint)ColliderLayers::PLATFORMS )
	{
		return;
	}
	*/
	/* Play sound when colliding
	std::array<b2PointState, 2>state1{}
	std::array<b2PointState, 2>state2{}
	b2GetPointStates(state1.data(), state2.data(), oldManifold, contact->GetManifold())
	i(state2[0] == b2_addState)
	{
		b2Vec2 point = worldManifold.points[0]
		b2Vec2 vA = pBodyA->body->GetLinearVelocityFromWorldPoint(point)
		b2Vec2 vB = pBodyB->body->GetLinearVelocityFromWorldPoint(point)
		float32 approachVelocity = b2Dot(vB-vA, worldManifold.normal)
		i(approachVelocity > 1.0f)
		{
			PlayCollisionSound()
		}
	}
	*/
}

void Physics::PostSolve(b2Contact *contact, const b2ContactImpulse *impulse)
{
	
}

//---------------- Body Creation

b2Body *Physics::CreateBody(iPoint pos, BodyType type, float angle, fPoint damping, float gravityScale, bool fixedRotation, bool bullet) const
{
	b2BodyDef body;
	switch(type)
	{
		using enum BodyType;
		case DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case STATIC:
			body.type = b2_staticBody;
			break;
		case KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case UNKNOWN:
			LOG("CreateBoidy received UNKNOWN BodyType");
			break;
	}
	body.position.Set(PIXEL_TO_METERS(pos.x), PIXEL_TO_METERS(pos.y));
	body.angle = DEGTORAD * angle;
	body.linearDamping = damping.x;
	body.angularDamping = damping.y;
	body.gravityScale = gravityScale;
	body.fixedRotation = fixedRotation;
	body.bullet = bullet;

	return world->CreateBody(&body);
}

std::unique_ptr<b2FixtureDef> Physics::CreateFixtureDef(ShapeData &shapeData, uint16 cat, uint16 mask, bool isSensor, float density, float friction, float restitution, b2Vec2 fixPos) const
{
	auto fixture = std::make_unique<b2FixtureDef>();
	fixture->shape = shapeData.CreateShape(fixPos);
	fixture->density = density;
	fixture->friction = friction;
	fixture->restitution = restitution;
	fixture->filter.categoryBits = cat;
	fixture->filter.maskBits = mask;
	fixture->isSensor = isSensor;

	return fixture;
}

std::unique_ptr<PhysBody> Physics::CreatePhysBody(b2Body *body, iPoint width_height, ColliderLayers cType) const
{
	auto pBody = std::make_unique<PhysBody>(body, width_height, cType);
	body->SetUserData(pBody.get());

	return pBody;
}

//--------------- Create Quick Shapes

std::unique_ptr<PhysBody> Physics::CreateQuickPlatform(ShapeData &shapeData, iPoint pos, iPoint width_height)
{
	auto body = CreateBody(pos);
	auto fixtureDef = CreateFixtureDef(shapeData);
	body->CreateFixture(fixtureDef.get());
	return CreatePhysBody(body, width_height, ColliderLayers::PLATFORMS);
}

std::unique_ptr<PhysBody> Physics::CreateQuickPhysBody(iPoint position, BodyType bodyType, ShapeData shapeData, uint16 cat, uint16 mask, iPoint width_height, bool sensor)
{
	auto body = CreateBody(position, bodyType);
	auto fixtureDef = CreateFixtureDef(shapeData, cat, mask, sensor);
	body->CreateFixture(fixtureDef.get());
	return CreatePhysBody(body, width_height, static_cast<ColliderLayers>(cat));
}


//--------------- Joints

//---- Mouse
b2MouseJoint *Physics::CreateMouseJoint(PhysBody *origin, PhysBody *target, b2Vec2 position, float dampingRatio, float frequecyHz, float maxForce)
{
	// TODO: Make so it works if camera moves
	b2MouseJointDef mJointDef;
	mJointDef.bodyA = origin->body;
	mJointDef.bodyB = target->body;
	mJointDef.target = position;
	mJointDef.dampingRatio = dampingRatio;
	mJointDef.frequencyHz = frequecyHz;
	mJointDef.maxForce = maxForce * selected->GetMass();

	return ((b2MouseJoint *)world->CreateJoint(&mJointDef));
}

b2MouseJoint *Physics::CreateMouseJoint(b2Body *origin, b2Body *target, b2Vec2 position, float dampingRatio, float frequecyHz, float maxForce)
{
	b2MouseJointDef mJointDef;
	mJointDef.bodyA = origin;
	mJointDef.bodyB = target;
	mJointDef.target = position;
	mJointDef.dampingRatio = dampingRatio;
	mJointDef.frequencyHz = frequecyHz;
	mJointDef.maxForce = maxForce * selected->GetMass();

	return ((b2MouseJoint *)world->CreateJoint(&mJointDef));
}

void Physics::DragSelectedObject()
{
	switch (app->input->GetMouseButtonDown(SDL_BUTTON_LEFT))
	{
		case KeyState::KEY_DOWN:
		{
			mouseJoint = CreateMouseJoint(ground, selected, PIXEL_TO_METERS(app->input->GetMousePosition()));
			break;
		}
		case KeyState::KEY_REPEAT:
		{
			mouseJoint->SetTarget(PIXEL_TO_METERS(app->input->GetMousePosition()));
			app->render->DrawLine(
				app->input->GetMousePosition(),
				METERS_TO_PIXELS(selected->GetPosition()),
				SDL_Color(0, 255, 255, 255)
			);
			break;
		}
		case KeyState::KEY_UP:
		{
			DestroyMouseJoint();
			break;
		}
		case KeyState::KEY_IDLE:
			break;

	}

}

void Physics::DestroyMouseJoint()
{
	world->DestroyJoint(mouseJoint);
	if (mouseJoint) mouseJoint = nullptr;
	if (selected) selected = nullptr;
}


//--------------- Utils

//---- Debug
void Physics::DrawDebug(const b2Body *body, const int32 count, const b2Vec2 *vertices, SDL_Color color) const
{
	b2Vec2 prev = body->GetWorldPoint(vertices[0]);
	b2Vec2 v;

	for (int32 i = 1; i < count; ++i)
	{
		v = body->GetWorldPoint(vertices[i]);
		app->render->DrawLine(METERS_TO_PIXELS(prev), METERS_TO_PIXELS(v), color);
		prev = v;
	}

	v = body->GetWorldPoint(vertices[0]);
	app->render->DrawLine(METERS_TO_PIXELS(prev), METERS_TO_PIXELS(v), color);
}

bool Physics::IsMouseOverObject(b2Fixture const *f) const
{
	if (f->TestPoint(IPointToWorldVec(app->input->GetMousePosition())))
		return true;
	return false;
}

bool Physics::IsDebugActive() const
{
	return debug;
}

//---- Position
iPoint Physics::WorldVecToIPoint(const b2Vec2 &v) const
{
	return iPoint(METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y));
}

b2Vec2 Physics::IPointToWorldVec(const iPoint &p) const
{
	return b2Vec2(PIXEL_TO_METERS(p.x), PIXEL_TO_METERS(p.y));
}

//---- World
void Physics::ToggleStep()
{
	stepActive = !stepActive;
}

b2Vec2 Physics::GetWorldGravity() const
{
	return world->GetGravity();
}

//---- Destroy
void Physics::DestroyBody(b2Body *b) const
{
	if(b) world->DestroyBody(b);
}

//--------------- PhysBody

void PhysBody::GetPosition(int &x, int &y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x) - width;
	y = METERS_TO_PIXELS(pos.y) - height;
}

float PhysBody::GetRotation() const
{
	return RADTODEG * body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	for (const b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		//if point P is inside the fixture shape
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p)) return true;
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float &normal_x, float &normal_y) const
{
	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;


	for (const b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0))
		{
			// do we want the normal ?
			auto fx = (float)(x2 - x1);
			auto fy = (float)(y2 - y1);
			float dist = sqrtf((fx * fx) + (fy * fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return (int)(output.fraction * dist);
		}
	}
	return -1;
}