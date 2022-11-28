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

struct TileInfo
{
	TileHitBox collider;
	int firstAnimGid;
	std::vector<std::pair<int, int>> tileAnim;
};

// L04: DONE 2: Create a struct to hold information for a TileSet
// Ignore Terrain Types and Tile Types for now, but we want the image!
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
	std::unordered_map<int, TileInfo> tileInfo;

	// L05: DONE 7: Create a method that receives a tile id and returns it's Rect find the Rect associated with a specific tile id
	SDL_Rect GetTileRect(int gid) const;
};

//  We create an enum for map type, just for convenience,
// NOTE: Platformer game will be of type ORTHOGONAL
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
	std::vector<uint> data;

	propertiesUnorderedmap properties;

	// Short function to get the gid value of x,y
	inline uint GetGidValue(int x, int y) const
	{
		return data[(y * width) + x];
	}

	variantProperty GetPropertyValue(const char *pName) const;

};

// L04: DONE 1: Create a struct needed to hold the information to Map node
struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	std::vector<TileSet *> tilesets;
	MapTypes type;

	// L05: DONE 2: Add a list/array of layers to the map
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

	// L05: DONE 8: Create a method that translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;

	MapData mapData;

private:

	bool LoadMap(pugi::xml_node const &mapFile);

	// L04: DONE 4: Create and call a private function to load a tileset
	bool LoadTileSet(pugi::xml_node const &mapFile);
	std::pair<TileHitBox, bool> LoadHitboxInfo(const pugi::xml_node &hitbox) const;

	// L05
	bool LoadAllLayers(pugi::xml_node const &mapNode);
	std::unique_ptr<MapLayer> LoadLayer(pugi::xml_node const &node);
	std::shared_ptr<PhysBody> CreateCollider (int gid, int i, int j, TileSet const *tileset) const;
	propertiesUnorderedmap LoadProperties(pugi::xml_node const &node) const;

	// L06: DONE 2
	TileSet *GetTilesetFromTileId(int gid) const;

	void LogLoadedData() const;

	std::string mapFileName;
	std::string mapFolder;
	bool mapLoaded = false;
	std::vector<std::shared_ptr<PhysBody>> collidersOnMap;
};

#endif // __MAP_H__