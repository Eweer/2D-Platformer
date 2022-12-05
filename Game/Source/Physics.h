#pragma once
#include "Module.h"
#include "Entity.h"
#include "Defs.h"
#include "BitMaskColliderLayers.h"

#include <unordered_map>
#include <variant>
#include <utility>

#include "Box2D/Box2D/Box2D.h"

// Tell the compiler to reference the compiled Box2D libraries

#ifdef _DEBUG
#pragma comment( lib, "../Game/Source/External/Box2D/libx86/DebugLib/Box2D.lib" )
#else
#pragma comment( lib, "../Game/Source/External/Box2D/libx86/ReleaseLib/Box2D.lib" )
#endif

#define GRAVITY_X 0.0f
#define GRAVITY_Y -8.0f

#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

// if type = "circle" -> data[0] = radius
// if type = "edge" -> data[0] = point1, data[1] = point2
// if type = "rectangle" -> data[0] = width, data[1] = height
// if type = "polygon" || "chain" -> data = points
struct ShapeData
{
	std::unique_ptr<b2Shape> shape;
	std::vector<int> data;

	ShapeData() = delete;
	explicit ShapeData(std::string const &name, std::vector<int> const &newData = std::vector<int>()) : data(newData)
	{
		if (StrEquals(name, "circle"))			shape = std::make_unique<b2CircleShape>();
		else if (StrEquals(name, "edge"))		shape = std::make_unique<b2EdgeShape>();
		else if (StrEquals(name, "rectangle"))	shape = std::make_unique<b2PolygonShape>();
		else if (StrEquals(name, "polygon"))	shape = std::make_unique<b2PolygonShape>();
		else if (StrEquals(name, "chain"))		shape = std::make_unique<b2ChainShape>();
		else LOG("Error in creating Shape (ShapeData). No shape with name %s exists", name);
	}
	explicit ShapeData(b2Shape const *newShape, std::vector<int> const &newData = std::vector<int>()) : data(newData)
	{
		if (!newShape) LOG("Error in creating Shape (ShapeData). Shape constructor is nullptr");
		else
		{
			switch (newShape->GetType())
			{
				case b2Shape::Type::e_circle:
					shape = std::make_unique<b2CircleShape>();
					break;
				case b2Shape::Type::e_edge:
					shape = std::make_unique<b2EdgeShape>();
					break;
				case b2Shape::Type::e_polygon:
					shape = std::make_unique<b2PolygonShape>();
					break;
				case b2Shape::Type::e_chain:
					shape = std::make_unique<b2ChainShape>();
					break;
				default:
					break;
			}
		}
	}
	~ShapeData() = default;

	b2Shape *CreateShape()
	{
		switch (shape.get()->GetType())
		{
			case b2Shape::Type::e_circle:
			{
				if (!data.empty())
					dynamic_cast<b2CircleShape *>(shape.get())->m_radius = PIXEL_TO_METERS(data[0]);
				else
					LOG("Radius for creating circle shape not found.");
				break;
			}
			case b2Shape::Type::e_edge:
			{
				break;
			}
			case b2Shape::Type::e_polygon:
			{
				// if it's a polygon with only 2 points
				// it means it's a rectangle, where data[0] == width && data[1] == height
				if (data.size() == 2)
				{
					dynamic_cast<b2PolygonShape *>(shape.get())->SetAsBox(
						PIXEL_TO_METERS(data[0]),
						PIXEL_TO_METERS(data[1])
					);
					break;
				}
				else //if it's not a rectangle, it's a plain polygon
				{
					auto p = std::vector<b2Vec2>();
					for (uint i = 0; i < data.size() / 2; ++i)
					{
						p.push_back(b2Vec2(
							PIXEL_TO_METERS(data[i * 2 + 0]),
							PIXEL_TO_METERS(data[i * 2 + 1])
						));
					}
					dynamic_cast<b2PolygonShape *>(shape.get())->Set(p.data(), p.size());
				}
				break;
			}
			case b2Shape::Type::e_chain:
			{
				auto p = std::vector<b2Vec2>();
				for (uint i = 0; i < data.size() / 2; ++i)
				{
					p.push_back(b2Vec2(
						PIXEL_TO_METERS(data[i * 2 + 0]),
						PIXEL_TO_METERS(data[i * 2 + 1])
					));
				}
				dynamic_cast<b2ChainShape *>(shape.get())->CreateLoop(p.data(), p.size());
				break;
			}
			default:
				break;
		}
		return shape.get();
	}
};

// types of bodies
enum class BodyType
{
	DYNAMIC,
	STATIC,
	KINEMATIC,
	UNKNOWN
};

enum class RevoluteJoinTypes
{
	IPOINT,
	BOOL,
	FLOAT,
	INT,
	UNKNOWN
};

struct RevoluteJointSingleProperty
{
	RevoluteJoinTypes type;
	union
	{
		iPoint p;
		bool b;
		float f;
		int i;
	};

	RevoluteJointSingleProperty() {};
	RevoluteJointSingleProperty(const RevoluteJointSingleProperty &r) : type(r.type)
	{
		switch(type)
		{
			case RevoluteJoinTypes::BOOL:
				b = r.b;
				break;

			case RevoluteJoinTypes::FLOAT:
				f = r.f;
				break;

			case RevoluteJoinTypes::INT:
				i = r.i;
				break;

			case RevoluteJoinTypes::IPOINT:
				p = r.p;
				break;
			case RevoluteJoinTypes::UNKNOWN:
				LOG("Something went wrong in InteractiveParts doing the revolute joint");
				break;
		}
	};
	~RevoluteJointSingleProperty() {};
};

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	explicit PhysBody() = default;
	explicit PhysBody(b2Body *newBody, iPoint width_height = iPoint(0, 0), ColliderLayers cType = ColliderLayers::UNKNOWN) :
		width(width_height.x),
		height(width_height.y),
		body(newBody),
		ctype(cType)
	{}

	~PhysBody() = default;

	void GetPosition(int &x, int &y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float &normal_x, float &normal_y) const;

	int width;
	int height;
	b2Body *body;
	Entity *listener = nullptr;
	ColliderLayers ctype = ColliderLayers::UNKNOWN;
};

// Module --------------------------------------
class Physics : public Module, public b2ContactListener
{
public:

	// Constructors & Destructors
	Physics();
	~Physics() final;

	// Main module steps
	bool Start() final;
	bool PreUpdate() final;
	bool PostUpdate() final;
	bool CleanUp() final;

	// Create basic physics objects
	std::unique_ptr<PhysBody> CreateRectangle(int x, int y, int width, int height, BodyType type, float32 gravityScale = 1.0f, float rest = 0.0f, uint16 cat = (uint16)ColliderLayers::PLATFORMS, uint16 mask = (uint16)ColliderLayers::PLAYER);
	std::unique_ptr<PhysBody> CreateCircle(int x, int y, int radius, BodyType type, float rest = 0.0f, uint16 cat = (uint16)ColliderLayers::PLATFORMS, uint16 mask = (uint16)ColliderLayers::PLAYER);

	std::unique_ptr<PhysBody> CreateQuickPlatform(
		ShapeData &shapeData,
		iPoint pos,
		iPoint width_height = iPoint(0, 0)
	);

	b2Body *CreateBody(
		iPoint pos,
		BodyType type = BodyType::STATIC,
		float angle = 0.0f,
		fPoint damping = {0.0f, 0.01f},
		float gravityScale = 1.0f,
		bool fixedRotation = true,
		bool bullet = false
	) const;

	std::unique_ptr<b2FixtureDef> CreateFixtureDef(
		ShapeData &shapeData,
		uint16 cat = (uint16)ColliderLayers::PLATFORMS,
		uint16 mask = (uint16)ColliderLayers::PLAYER,
		bool isSensor = false,
		float density = 1.0f,
		float friction = 1.0f,
		float restitution = 0.0f
	) const;

	std::unique_ptr<PhysBody> CreatePhysBody(
		b2Body *body = nullptr,
		iPoint width_height = iPoint(0,0),
		ColliderLayers cType = ColliderLayers::UNKNOWN
	) const;

	// Create joints
	b2RevoluteJoint *CreateRevoluteJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties);
	b2PrismaticJoint *CreatePrismaticJoint(PhysBody *anchor, PhysBody *body, iPoint anchorOffset, iPoint bodyOffset, std::vector<RevoluteJointSingleProperty> properties);
	b2MouseJoint *CreateMouseJoint(PhysBody *ground, PhysBody *target, b2Vec2 position, float dampingRatio = 0.5f, float frequecyHz = 2.0f, float maxForce = 100.0f);
	b2MouseJoint *CreateMouseJoint(b2Body *ground, b2Body *target, b2Vec2 position, float dampingRatio = 0.5f, float frequecyHz = 2.0f, float maxForce = 100.0f);

	// b2ContactListener ---
	void BeginContact(b2Contact *contact) final;

	// Utils
	iPoint WorldVecToIPoint(const b2Vec2 &v) const;
	b2Vec2 IPointToWorldVec(const iPoint &p) const;

	void ToggleStep();

	b2Vec2 GetWorldGravity() const;

	void DestroyBody(b2Body *b = nullptr);
	void DestroyPhysBody(PhysBody *b = nullptr);

	// Get Info
	bool IsDebugActive() const;
	BodyType GetEnumFromStr(const std::string &s) const;
	RevoluteJoinTypes GetTypeFromProperty(const std::string &s) const;

private:

	// Debug
	void DrawDebug(const b2Body *body, const int32 count, const b2Vec2 *vertices, Uint8 r, Uint8 g, Uint8 b, Uint8 a = (Uint8)255U) const;

	// Joints
	void DragSelectedObject();
	bool IsMouseOverObject(b2Fixture const *f) const;
	void DestroyMouseJoint();

	// Debug mode
	bool debug = false;
	bool debugWhileSelected = true;
	bool stepActive = true;

	// Box2D World
	b2World *world = nullptr;
	b2Body *ground;

	// Mouse Joint
	b2Body *selected = nullptr;
	b2MouseJoint *mouseJoint = nullptr;

	static const std::unordered_map<std::string, BodyType, StringHash, std::equal_to<>> bodyTypeStrToEnum;
	static const std::unordered_map<std::string, RevoluteJoinTypes, StringHash, std::equal_to<>> propertyToType;	
};