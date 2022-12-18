#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "Physics.h"

#include "Defs.h"
#include "Point.h"
#include "BitMaskNavType.h"


#include <functional>
#include <vector>
#include <cstdlib>			//	std::rand

#include "PugiXml/src/pugixml.hpp"

using XML_Property_t = std::variant<int, bool, float, std::string>;
using XML_Properties_Map_t = std::unordered_map<std::string, XML_Property_t, StringHash, std::equal_to<>>;

enum class NavLinkType
{
	UNKNOWN = 0x0000,
	WALK = 0x0001,
	FALL = 0x0002,
	JUMP = 0x0004
};

struct NavLink
{
	NavLink() = default;
	NavLink(iPoint p, int g, NavLinkType n)
		: destination(p), score(g), movement(n) {};
	
	iPoint destination = {0,0};
	int score = 10;
	NavLinkType movement = NavLinkType::UNKNOWN;
};

struct NavPoint
{
	NavPoint() = default;
	CL::NavType type = CL::NavType::NONE;
	std::vector<NavLink> links;
};

using navPointMatrix = std::vector<std::vector<NavPoint>>;

enum class MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};

struct TileColliderInfo
{
	int x = 0;
	int y = 0;
	std::string shape = "";
	int width = 0;
	int height = 0;
	uint16 cat = 0x0000;
	std::vector<b2Vec2> points;
};

struct TileAnimationInfo
{
	// <gid, duration>
	uint varianceMin = 0;
	uint varianceMax = 0;
	std::vector<std::pair<uint, uint>> frames;
};

struct TileInfo
{
	XML_Properties_Map_t properties;
	std::vector<TileColliderInfo> collider;
	std::shared_ptr<TileAnimationInfo> animation;
	
	explicit operator bool() const 
	{
		return properties.empty() || !collider.empty() || !animation->frames.empty();
	}
};

struct TileSet
{
	std::string name = "";
	int	firstgid = 0;
	int margin = 0;
	int	spacing = 0;
	int	tileWidth = 0;
	int	tileHeight = 0;
	int columns = 0;
	int tilecount = 0;

	SDL_Texture *texture = nullptr;
	
	// index, TileInfo
	std::unordered_map<int, std::unique_ptr<TileInfo>> tileInfo;
	
	SDL_Rect GetTileRect(int gid) const;
};



struct TileImage
{
	uint gid = 0;
	uint originalGid = 0;

	bool active = false;
	std::shared_ptr<TileAnimationInfo> anim;
	uint currentFrame = 0;
	uint timer = 0;
	uint duration;

	inline void AdvanceTimer()
	{
		timer += 1;
		if(timer >= duration)
		{
			const auto &currentAnim = anim.get();
			uint variance = 0;
			currentFrame++;
			if(currentFrame >= currentAnim->frames.size())
			{
				currentFrame = 0;
				variance = (currentAnim->varianceMax > 0) ? 
					((uint)std::rand() % currentAnim->varianceMax + currentAnim->varianceMin) : 
					0;
			}

			const auto &[nextFrame, nextTimer] = currentAnim->frames[currentFrame];
			
			gid = nextFrame + 1;

			timer = 0;
			duration = nextTimer + variance;

		}
	}
};

struct MapLayer
{
	std::string name = "";
	int id = -1;
	int width = 0;
	int height = 0;
	std::vector<TileImage> tileData;
	XML_Properties_Map_t properties;
	
	inline uint GetGidValue(int x, int y) const
	{
		return tileData[(y * width) + x].gid;
	}

	XML_Property_t GetPropertyValue(const char *pName) const;

};

struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	std::vector<std::unique_ptr<TileSet>> tilesets;
	MapTypes type;

	std::vector<std::unique_ptr<MapLayer>> mapLayers;
};

class Map : public Module
{
public:

	Map();

	// Destructor
	~Map() final;

	// Called before render is available
	bool Awake(pugi::xml_node &conf) final;

	// Called each loop iteration
	void Draw() const;

	void DrawLayer(const MapLayer *layer) const;

	bool Pause(int phase) final;

	// Called before quitting
	bool CleanUp() final;

	// Load new map
	bool Load();

	// Translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;
	iPoint MapToWorld(iPoint position) const;

	iPoint WorldToCoordinates(iPoint position) const;

	int WorldXToCoordinates(int n) const;

	int WorldYToCoordinates(int n) const;
	
	int GetWidth() const;

	int GetHeight() const;

	int GetTileWidth() const;

	int GetTileHeight() const;

	int GetTileSetSize() const;

	std::unique_ptr<navPointMatrix> CreateWalkabilityMap();

	bool IsWalkable(uint gid) const;

	bool IsTerrain(uint gid) const;

	std::string_view GetMapFolderName() const;

private:

	bool LoadMap(pugi::xml_node const &mapFile);
	
	bool LoadTileSet(pugi::xml_node const &mapFile);
	std::unique_ptr<TileInfo> LoadTileInfo(const pugi::xml_node &tileInfoNode) const;
	std::shared_ptr<TileAnimationInfo> LoadAnimationInfo(const pugi::xml_node &tileInfoNode, XML_Properties_Map_t const &properties) const;
	std::vector<TileColliderInfo> LoadHitboxInfo(const pugi::xml_node &hitbox, XML_Properties_Map_t const &properties = XML_Properties_Map_t()) const;
	std::unique_ptr<PhysBody> CreateCollider (int gid, int i, int j, TileSet const *tileset) const;
	
	bool LoadAllLayers(pugi::xml_node const &mapNode);
	std::unique_ptr<MapLayer> LoadLayer(pugi::xml_node const &node);
	XML_Properties_Map_t LoadProperties(pugi::xml_node const &node) const;
	
	TileSet *GetTilesetFromTileId(int gid) const;

	void LogLoadedData() const;

	std::unique_ptr<navPointMatrix> CreateWalkabilityNodes() const;

	MapData mapData;
	std::string mapFileName;
	std::string mapFolder;
	bool mapLoaded = false;
	std::vector<std::unique_ptr<PhysBody>> terrainColliders;
	
};

#endif // __MAP_H__
