#ifndef __MAP_H__
#define __MAP_H__

#include "Module.h"
#include "List.h"
#include "Point.h"
#include "Defs.h"
#include "Log.h"
#include "Physics.h"

#include <unordered_map>
#include <functional>
#include <vector>
#include <variant>
#include <memory>

#include "PugiXml/src/pugixml.hpp"

using variantProperty = std::variant<int, bool, float, std::string>;
using propertiesUnorderedmap = std::unordered_map<std::string, variantProperty, StringHash, std::equal_to<>>;

struct TileHitBox
{
	int x = 0;
	int y = 0;
	std::string shape = "";
	int width = 0;
	int height = 0;
	uint16 cat;
	std::vector<int> data;
};

struct TileAnim
{
	std::vector<std::pair<int, int>> frames;
};

struct TileInfo
{
	TileHitBox collider;
	TileAnim tileAnim;
};

struct TileSet
{
	std::string name;
	int	firstgid;
	int margin;
	int	spacing;
	int	tileWidth;
	int	tileHeight;
	int columns;
	int tilecount;

	SDL_Texture *texture;
	
	// index, TileInfo
	std::unordered_map<int, TileInfo> tileInfo;

	std::vector<int> animatedTiles;
	
	SDL_Rect GetTileRect(int gid) const;
};

struct AnimatedTile
{
	uint staticGid = 0;
	bool active = false;
	std::shared_ptr<TileAnim> frames;
	float currentFrame = 0.0f;
};

enum class MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};

struct MapLayer
{
	std::string name = "";
	int id = -1;
	int width = 0;
	int height = 0;
	std::vector<AnimatedTile> tileData;
	propertiesUnorderedmap properties;
	
	inline uint GetGidValue(int x, int y) const
	{
		return tileData[(y * width) + x].staticGid;
	}

	variantProperty GetPropertyValue(const char *pName) const;

};

struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	std::vector<TileSet *> tilesets;
	MapTypes type;

	// index, std::pair<id of tile, duration>
	std::vector<std::unique_ptr<MapLayer>> mapLayers;
};

class Map : public Module
{
public:

	Map();

	// Destructor
	virtual ~Map();

	// Called before render is available
	bool Awake(pugi::xml_node &conf) final;

	// Called each loop iteration
	void Draw() const;

	// Called before quitting
	bool CleanUp() final;

	// Load new map
	bool Load();

	// Translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;

	MapData mapData;

private:

	bool LoadMap(pugi::xml_node const &mapFile);
	
	bool LoadTileSet(pugi::xml_node const &mapFile);
	std::pair<TileHitBox, bool> LoadHitboxInfo(const pugi::xml_node &hitbox) const;
	std::shared_ptr<PhysBody> CreateCollider (int gid, int i, int j, TileSet const *tileset) const;
	
	bool LoadAllLayers(pugi::xml_node const &mapNode);
	std::unique_ptr<MapLayer> LoadLayer(pugi::xml_node const &node);
	propertiesUnorderedmap LoadProperties(pugi::xml_node const &node) const;
	
	TileSet *GetTilesetFromTileId(int gid) const;

	void LogLoadedData() const;

	std::string mapFileName;
	std::string mapFolder;
	bool mapLoaded = false;
	std::vector<std::shared_ptr<PhysBody>> collidersOnMap;
};

#endif // __MAP_H__