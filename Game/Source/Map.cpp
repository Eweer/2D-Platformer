#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"

#include "Defs.h"
#include "Log.h"

#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <variant>
#include <unordered_map>


#include "SDL_image/include/SDL_image.h"

Map::Map() : Module()
{
	name = "map";
}

// Destructor
Map::~Map() = default;

// Called before render is available
bool Map::Awake(pugi::xml_node &config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	mapFileName = config.child("mapfile").attribute("path").as_string();
	mapFolder = config.child("mapfolder").attribute("path").as_string();

	return ret;
}

void Map::Draw()
{
	if(mapLoaded == false)
		return;

	//ListItem<MapLayer*>* mapLayerItem = mapData.mapLayers.start;

	//for(auto const &layer : mapData.mapLayers)

	while(mapLayerItem != nullptr)
	{
		if (*(std::get_if<bool>(&mapLayerItem->data->GetPropertyValue("Draw"))))
		{
			for (int x = 0; x < mapLayerItem->data->width; x++)
			{
				for (int y = 0; y < mapLayerItem->data->height; y++)
				{
					// L05: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->GetGidValue(x, y);

					//L06: DONE 3: Obtain the tile set using GetTilesetFromTileId
					TileSet* tileset = GetTilesetFromTileId(gid);

					SDL_Rect r = tileset->GetTileRect(gid);
					iPoint pos = MapToWorld(x, y);

					app->render->DrawTexture(tileset->texture, pos.x, pos.y,&r);
				}
			}
		}
		mapLayerItem = mapLayerItem->next;
	}
}

// L05: DONE 8: Create a method that translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	ret.x = x * mapData.tileWidth;
	ret.y = y * mapData.tileHeight;

	return ret;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int gid) const
{
	SDL_Rect rect = {0};
	int relativeIndex = gid - firstgid;

	// L05: DONE 7: Get relative Tile rectangle
	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
	rect.y = margin + (tileWidth + spacing) * (relativeIndex / columns);

	return rect;
}


// L06: DONE 2: Pick the right Tileset based on a tile id
TileSet *Map::GetTilesetFromTileId(int gid) const
{
	ListItem<TileSet *> *item = mapData.tilesets.start;
	TileSet *set = nullptr;

	while(item)
	{
		set = item->data;
		if(gid < (item->data->firstgid + item->data->tilecount))
		{
			break;
		}
		item = item->next;
	}

	return set;
}

// Called before quitting
bool Map::CleanUp()
{
	LOG("Unloading map");

	return true;
}

// Load new map
bool Map::Load()
{
	pugi::xml_document mapFileXML;

	if(auto result = mapFileXML.load_file(mapFileName.c_str()); !result)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapFileName, result.description());
		return false;
	}

	if(!LoadMap(mapFileXML))
	{
		LOG("Could not load map.");
		return false;
	}

	if(!LoadTileSet(mapFileXML))
	{
		LOG("Could not load tile set.");
		return false;
	}

	if(!LoadAllLayers(mapFileXML.child("map")))
	{
		LOG("Could not load map.");
		return false;
	}

	// L07 DONE 3: Create colliders
	// Later you can create a function here to load and create the colliders from the map
	PhysBody *c1 = app->physics->CreateRectangle(224 + 128, 543 + 32, 256, 64, STATIC);
	// L07 DONE 7: Assign collider type
	c1->ctype = ColliderType::PLATFORM;

	PhysBody *c2 = app->physics->CreateRectangle(352 + 64, 384 + 32, 128, 64, STATIC);
	// L07 DONE 7: Assign collider type
	c2->ctype = ColliderType::PLATFORM;

	PhysBody *c3 = app->physics->CreateRectangle(256, 704 + 32, 576, 64, STATIC);
	// L07 DONE 7: Assign collider type
	c3->ctype = ColliderType::PLATFORM;

	PhysBody *c4 = app->physics->CreateRectangle(640 + 352/2, 704 + 30, 352, 61, STATIC);
	c4->ctype = ColliderType::PLATFORM;

	PhysBody *c5 = app->physics->CreateRectangle(768 + 64, 480 + 31, 128, 63, STATIC);
	c5->ctype = ColliderType::PLATFORM;

	PhysBody *c6 = app->physics->CreateRectangle(640 + 64, 320 + 32, 128, 64, STATIC);
	c6->ctype = ColliderType::PLATFORM;

	//4x4 platforms
	PhysBody *c7 = app->physics->CreateRectangle(1024 + 32, 384 + 32, 64, 64, STATIC);
	c7->ctype = ColliderType::PLATFORM;

	PhysBody *c8 = app->physics->CreateRectangle(1152 + 32, 288 + 32, 64, 64, STATIC);
	c8->ctype = ColliderType::PLATFORM;

	//5x2 platform
	PhysBody *c9 = app->physics->CreateRectangle(1312 + 80, 224 + 32, 160, 64, STATIC);
	c9->ctype = ColliderType::PLATFORM;

	//2x3 platforms - bridge
	PhysBody *c10 = app->physics->CreateRectangle(1088 + 48, 640 + 32, 96, 64, STATIC);
	c10->ctype = ColliderType::PLATFORM;

	PhysBody *c11 = app->physics->CreateRectangle(1280 + 48, 640 + 32, 96, 64, STATIC);
	c11->ctype = ColliderType::PLATFORM;

	//17x2 final platform
	PhysBody *c12 = app->physics->CreateRectangle(1473 + 272, 704 + 32, 544, 64, STATIC);
	c12->ctype = ColliderType::PLATFORM;

	//2x20 vertical limits
	PhysBody *c13 = app->physics->CreateRectangle(1984 + 16, 55 + 336, 32, 672, STATIC);
	c13->ctype = ColliderType::PLATFORM;

	PhysBody *c14 = app->physics->CreateRectangle(32 + 16, 64 + 336, 32, 672, STATIC);
	c14->ctype = ColliderType::PLATFORM;

	//60x1 ceiling
	PhysBody *c15 = app->physics->CreateRectangle(64 + 960, 32 + 16, 1920, 32, STATIC);
	c15->ctype = ColliderType::PLATFORM;

	mapFileXML.reset();

	return mapLoaded = true;
}

// Load the map properties
bool Map::LoadMap(pugi::xml_node const &mapFile)
{
	pugi::xml_node map = mapFile.child("map");

	if(!map)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		return false;

	}

	// Load map general properties
	mapData.height = map.attribute("height").as_int();
	mapData.width = map.attribute("width").as_int();
	mapData.tileHeight = map.attribute("tileheight").as_int();
	mapData.tileWidth = map.attribute("tilewidth").as_int();

	return true;
}

// Load the tileset properties
bool Map::LoadTileSet(pugi::xml_node const &mapFile)
{
	for(auto const &elem : mapFile.child("map").child("tileset"))
	{
		auto retTileSet = std::make_unique<TileSet>();

		retTileSet->name = elem.attribute("name").as_string();
		retTileSet->firstgid = elem.attribute("firstgid").as_int();
		retTileSet->margin = elem.attribute("margin").as_int();
		retTileSet->spacing = elem.attribute("spacing").as_int();
		retTileSet->tileWidth = elem.attribute("tilewidth").as_int();
		retTileSet->tileHeight = elem.attribute("tileheight").as_int();
		retTileSet->columns = elem.attribute("columns").as_int();
		retTileSet->tilecount = elem.attribute("tilecount").as_int();

		auto path = mapFolder + elem.child("image").attribute("source").as_string();
		retTileSet->texture = app->tex->Load(path.c_str());

		mapData.tilesets.Add(retTileSet.get());
	}

	return true;
}

// Iterate all layers and load each of them
bool Map::LoadAllLayers(pugi::xml_node const &node)
{
	//for(auto const &layer : mapNode.child("layer"))
	for(auto const &layer : node.child("layer"))
	{
		mapData.mapLayers.Add(LoadLayer(layer));
	}
	return true;
}

// Loads a single layer
MapLayer *Map::LoadLayer(pugi::xml_node const &node) const
{

	auto layer = std::make_unique<MapLayer>();

	//Load the attributes
	layer->id = node.attribute("id").as_int();
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();

	//Iterate over all the tiles and get gid values
	for(int i = 0; auto const &elem : node.child("data").child("tile"))
	{
		if(int gid = elem.attribute("gid").as_int(); gid > 0) layer->data.push_back(gid);
	}

	layer->properties = LoadProperties(node);
	

	return layer.release();
}


propertiesUnorderedmap Map::LoadProperties(pugi::xml_node const &node) const
{
	propertiesUnorderedmap properties;
	for(auto const &elem : node.child("properties").child("property"))
	{
		variantProperty valueToEmplace;
		switch(str2int(elem.attribute("type").name()))
		{
			case g_IntTypeStr2Int:
			case str2int("object"):
				valueToEmplace = elem.attribute("value").as_int();
				break;
			case g_FloatTypeStr2Int:
				valueToEmplace = elem.attribute("value").as_float();
				break;
			case g_BoolTypeStr2Int:
				valueToEmplace = elem.attribute("value").as_bool();
				break;
			default:
				LOG("Variant doesn't have %s type", elem.attribute("type").name());
				valueToEmplace = elem.attribute("value").as_string();
				continue;
		}
		properties.try_emplace(elem.attribute("name").as_string(), valueToEmplace);
	}
	return properties;
}

// Ask for the value of a custom property
variantProperty MapLayer::GetPropertyValue(const char *pName) const
{
	if(auto search = properties.find(pName); search != properties.end())
		return search->second;

	LOG("No property with name %s", pName);
	return false;
}

// LOG all the data loaded iterate all tilesets and LOG everything
void Map::LogLoadedData() const
{
	LOG("Successfully parsed map XML file :%s", mapFileName);
	LOG("width : %d height : %d", mapData.width, mapData.height);
	LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

	LOG("Tilesets----");

	ListItem<TileSet *> *tileset;
	tileset = mapData.tilesets.start;

	while(tileset)
	{
		LOG("Name : %s			First gid : %d", tileset->data->name, tileset->data->firstgid);
		LOG("Tile width : %d	Tile height : %d", tileset->data->tileWidth, tileset->data->tileHeight);
		LOG("Spacing : %d		Margin : %d", tileset->data->spacing, tileset->data->margin);

		tileset = tileset->next;
	}

	// Info for each loaded layer
	ListItem<MapLayer *> *mapLayer;
	mapLayer = mapData.mapLayers.start;

	while(mapLayer)
	{
		LOG("Id : %d			Name : %s", mapLayer->data->id, mapLayer->data->name);
		LOG("Layer width : %d	Layer height : %d", mapLayer->data->width, mapLayer->data->height);

		for(auto const &[key, value] : mapLayer->data->properties)
		{
			if(value.valueless_by_exception())
			{
				LOG("Property %s has key valueless_by_exception.", key);
				continue;
			}
			switch(value.index())
			{
				case 0:
					LOG("Property: %s		Value: %i, ", key, value);
					break;
				case 1:
					LOG("Property: %s		Value: %s, ", key, (*(std::get_if<bool>(&value)) ? "true" : "false"));
					break;
				case 2:
					LOG("Property: %s		Value: %.2f, ", key, value);
					break;
				case 3:
					LOG("Property: %s		Value: %s, ", key, value);
					break;
				default:
					LOG("Case %i of property %s not covered in Switch", value, key);
					break;
			}

		}

		mapLayer = mapLayer->next;
	}
}
