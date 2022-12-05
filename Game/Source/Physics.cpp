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

const std::unordered_map<std::string, BodyType, StringHash, std::equal_to<>> Physics::bodyTypeStrToEnum{
	{"dynamic", BodyType::DYNAMIC},
	{"static", BodyType::STATIC},
	{"kinematic", BodyType::KINEMATIC},
	{"unknown", BodyType::UNKNOWN}
};

const std::unordered_map<std::string, RevoluteJoinTypes, StringHash, std::equal_to<>> Physics::propertyToType{
	{"anchor_offset", RevoluteJoinTypes::IPOINT},
	{"body_offset", RevoluteJoinTypes::IPOINT},
	{"enable_limit", RevoluteJoinTypes::BOOL},
	{"max_angle", RevoluteJoinTypes::FLOAT},
	{"min_angle", RevoluteJoinTypes::FLOAT},
	{"enable_motor", RevoluteJoinTypes::BOOL},
	{"motor_speed", RevoluteJoinTypes::INT},
	{"max_torque", RevoluteJoinTypes::INT}
};

Physics::Physics() : Module()
{
}

// Destructor
Physics::~Physics() = default;

//--------------- 

bool Physics::Start()
{
	LOG("Creating Physics 2D environment");

	// Create a new World
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));

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
		if (app->input->GetKey(keyIterator) == KEY_DOWN) {
			if (keyIterator == SDL_SCANCODE_0)
			{
				newGrav = 0;
				break;
			}
			newGrav = (float)((int)keyIterator - (int)SDL_SCANCODE_1 + 1);
			if (app->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) newGrav *= -1;
			if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) newGrav *= 2;
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

	// Because Box2D does not automatically broadcast collisions/contacts with sensors, 
	// we have to manually search for collisions and "call" the equivalent to the ModulePhysics::BeginContact() ourselves...
	for (b2Contact *c = world->GetContactList(); c; c = c->GetNext())
	{
		// For each contact detected by Box2D, see if the first one colliding is a sensor
		if (c->IsTouching() && c->GetFixtureA()->IsSensor())
		{
			// If so, we call the OnCollision listener function (only of the sensor), passing as inputs our custom PhysBody classes
			auto *pb1 = (PhysBody *)c->GetFixtureA()->GetBody()->GetUserData();
			auto *pb2 = (PhysBody *)c->GetFixtureB()->GetBody()->GetUserData();

			if (pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

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

	//  Iterate all objects in the world and draw the bodies
	//  until there are no more bodies or 
	//  we are dragging an object around and not debugging draw in the meantime
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
					auto const circleShape = (b2CircleShape *)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					app->render->DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(circleShape->m_radius), 255, 255, 255);
					break;
				}
				// Draw polygons ------------------------------------------------
				case b2Shape::Type::e_polygon:
				{
					auto const *itemToDraw = (b2PolygonShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, 255, 255, 0);
					break;
				}
				// Draw chains contour -------------------------------------------
				case b2Shape::Type::e_chain:
				{
					auto const *itemToDraw = (b2ChainShape *)f->GetShape();
					DrawDebug(b, itemToDraw->m_count, itemToDraw->m_vertices, 100, 255, 100);
					break;
				}
				// Draw a single segment(edge) ----------------------------------
				case b2Shape::Type::e_edge:
				{
					auto const *edgeShape = (b2EdgeShape *)f->GetShape();
					b2Vec2 v1;
					b2Vec2 v2;

					v1 = b->GetWorldPoint(edgeShape->m_vertex0);
					v1 = b->GetWorldPoint(edgeShape->m_vertex1);
					app->render->DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), 100, 100, 255);
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

	// Delete the whole physics world!
	RELEASE(world)

		return true;
}


//--------------- Callback function to collisions with Box2D

void Physics::BeginContact(b2Contact *contact)
{
	// Call the OnCollision listener function to bodies A and B, passing as inputs our custom PhysBody classes
	auto *pBodyA = (PhysBody *)contact->GetFixtureA()->GetBody()->GetUserData();
	auto *pBodyB = (PhysBody *)contact->GetFixtureB()->GetBody()->GetUserData();

	if (pBodyA && pBodyA->listener)
		pBodyA->listener->OnCollision(pBodyA, pBodyB);

	if (pBodyB && pBodyB->listener)
		pBodyB->listener->OnCollision(pBodyB, pBodyA);
}


//--------------- Create Shapes and Joints

std::unique_ptr<PhysBody> Physics::CreateQuickPlatform(ShapeData &shapeData, iPoint pos, iPoint width_height)
{
	auto body = CreateBody(pos);
	auto fixtureDef = CreateFixtureDef(shapeData);
	body->CreateFixture(fixtureDef.get());
	return CreatePhysBody(body, width_height, ColliderLayers::PLATFORMS);
}

std::unique_ptr<PhysBody> Physics::CreateRectangle(int x, int y, int width, int height, BodyType type, float32 gravityScale, float rest, uint16 cat, uint16 mask)
{
	auto body = CreateBody(iPoint(x, y), type);

	ShapeData box("rectangle", std::vector<int>{width, height});

	// Create FIXTURE
	auto fixture = CreateFixtureDef(box, cat, mask);

	// Add fixture to the BODY
	body->CreateFixture(fixture.get());

	// Return our PhysBody class
	return CreatePhysBody(body, iPoint(width, height));
}

std::unique_ptr<PhysBody> Physics::CreateCircle(int x, int y, int radius, BodyType type, float rest, uint16 cat, uint16 mask)
{
	// Create BODY at position x,y
	b2BodyDef body;
	switch (type)
	{
		case BodyType::DYNAMIC:
			body.type = b2_dynamicBody;
			break;
		case BodyType::STATIC:
			body.type = b2_staticBody;
			break;
		case BodyType::KINEMATIC:
			body.type = b2_kinematicBody;
			break;
		case BodyType::UNKNOWN:
			LOG("CreateRectangle Received UNKNOWN BodyType");
			return nullptr;
	}
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// Add BODY to the world
	b2Body *b = world->CreateBody(&body);

	// Create SHAPE
	b2CircleShape circle;
	circle.m_radius = PIXEL_TO_METERS(radius);

	// Create FIXTURE
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 1.0f;
	fixture.filter.categoryBits = cat;
	fixture.filter.maskBits = mask;
	fixture.restitution = rest;

	// Add fixture to the BODY
	b->CreateFixture(&fixture);

	// Create our custom PhysBody class
	auto pbody = std::make_unique<PhysBody>();
	pbody->body = b;
	b->SetUserData(pbody.get());
	pbody->width = radius * 2;
	pbody->height = radius * 2;

	// Return our PhysBody class
	return pbody;
}


//--------------- Body

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

std::unique_ptr<b2FixtureDef> Physics::CreateFixtureDef(ShapeData &shapeData, uint16 cat, uint16 mask, bool isSensor, float density, float friction, float restitution) const
{
	auto fixture = std::make_unique<b2FixtureDef>();
	fixture->shape = shapeData.CreateShape();
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


//--------------- Joints

//---- Standard
b2RevoluteJoint *Physics::CreateRevoluteJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties)
{
	b2RevoluteJointDef rJoint;
	rJoint.bodyA = anchor->body;
	rJoint.bodyB = body->body;
	rJoint.collideConnected = false;

	rJoint.localAnchorA = b2Vec2(PIXEL_TO_METERS(anchorOffset.x), PIXEL_TO_METERS(anchorOffset.y));
	rJoint.localAnchorB = b2Vec2(PIXEL_TO_METERS(bodyOffset.x), PIXEL_TO_METERS(bodyOffset.y));

	if ((rJoint.enableLimit = properties[0].b))
	{
		rJoint.upperAngle = DEGTORAD * (properties[1].f);
		rJoint.lowerAngle = DEGTORAD * (properties[2].f);
	}
	if ((rJoint.enableMotor = properties[3].b))
	{
		rJoint.motorSpeed = (float)properties[4].i;
		rJoint.maxMotorTorque = (float)properties[5].i;
	}

	auto *returnJoint = ((b2RevoluteJoint *)world->CreateJoint(&rJoint));
	return returnJoint;
}

b2PrismaticJoint *Physics::CreatePrismaticJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties)
{
	b2PrismaticJointDef pJoint;
	pJoint.bodyA = anchor->body;
	pJoint.bodyB = body->body;
	pJoint.collideConnected = false;

	pJoint.localAnchorA = b2Vec2(PIXEL_TO_METERS(anchorOffset.x), PIXEL_TO_METERS(anchorOffset.y));
	pJoint.localAnchorB = b2Vec2(PIXEL_TO_METERS(bodyOffset.x), PIXEL_TO_METERS(bodyOffset.y));

	pJoint.localAxisA = b2Vec2(0, 1);

	if ((pJoint.enableLimit = properties[0].b))
	{
		pJoint.lowerTranslation = DEGTORAD * (properties[1].f);
		pJoint.upperTranslation = DEGTORAD * (properties[2].f);
	}
	if ((pJoint.enableMotor = properties[3].b))
	{
		pJoint.motorSpeed = (float)properties[4].i;
		pJoint.maxMotorForce = (float)properties[5].i;
	}

	return (b2PrismaticJoint *)world->CreateJoint(&pJoint);
}

//---- Mouse
b2MouseJoint *Physics::CreateMouseJoint(PhysBody *origin, PhysBody *target, b2Vec2 position, float dampingRatio, float frequecyHz, float maxForce)
{
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
	int mouseX;
	int mouseY;
	app->input->GetMousePosition(mouseX, mouseY);
	b2Vec2 target(PIXEL_TO_METERS(mouseX), PIXEL_TO_METERS(mouseY));

	switch (app->input->GetMouseButtonDown(SDL_BUTTON_LEFT))
	{
		case KeyState::KEY_DOWN:
		{
			mouseJoint = CreateMouseJoint(ground, selected, target);
			break;
		}
		case KeyState::KEY_REPEAT:
		{
			mouseJoint->SetTarget(target);
			app->render->DrawLine(mouseX, mouseY, METERS_TO_PIXELS(selected->GetPosition().x), METERS_TO_PIXELS(selected->GetPosition().y), 0, 255, 255, 255);
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
void Physics::DrawDebug(const b2Body *body, const int32 count, const b2Vec2 *vertices, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
{
	b2Vec2 prev = body->GetWorldPoint(vertices[0]);
	b2Vec2 v;

	for (int32 i = 1; i < count; ++i)
	{
		v = body->GetWorldPoint(vertices[i]);
		app->render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), r, g, b, a);
		prev = v;
	}

	v = body->GetWorldPoint(vertices[0]);
	app->render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), r, g, b, a);
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
void Physics::DestroyBody(b2Body *b)
{
	if (b) world->DestroyBody(b);
}

void Physics::DestroyPhysBody(PhysBody *b)
{
	DestroyBody(b->body);
	if (b) delete b;

}

//---- Map manipulation
BodyType Physics::GetEnumFromStr(const std::string &s) const
{
	if (!bodyTypeStrToEnum.count(s))
	{
		LOG("Physics::GetEnumFromStr didn't find %s attribute.", s);
		return BodyType::UNKNOWN;
	}
	return bodyTypeStrToEnum.at(s);
}

RevoluteJoinTypes Physics::GetTypeFromProperty(const std::string &s) const
{
	if (!propertyToType.count(s))
	{
		LOG("Physics::GetTypeFromProperty didn't find %s attribute.", s);
		return RevoluteJoinTypes::UNKNOWN;
	}
	return propertyToType.at(s);
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