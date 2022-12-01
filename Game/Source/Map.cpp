#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Physics.h"
#include "EntityManager.h"

#include "Defs.h"
#include "Log.h"
#include "BitMaskColliderLayers.h"

#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <variant>
#include <unordered_map>
#include <utility>

#include <iostream>


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

	mapFileName = config.child("mapfile").attribute("path").as_string();
	mapFolder = config.child("mapfolder").attribute("path").as_string();

	return true;
}

void Map::Draw() const
{
	if(!mapLoaded)
		return;
	
	for(auto const &layer : mapData.mapLayers)
	{
		if(auto const drawProperty = layer->GetPropertyValue("Draw");
		   !*(std::get_if<bool>(&drawProperty)))
		{
			continue;
		}
		
		for (int x = 0; x < layer->width; x++)
		{
			for (int y = 0; y < layer->height; y++)
			{
				uint gid = layer->GetGidValue(x, y);
				
				if(gid <= 0) continue;

				TileSet* tileset = GetTilesetFromTileId(gid);
				
				SDL_Rect r = tileset->GetTileRect(gid);
				iPoint pos = MapToWorld(x, y);

				app->render->DrawTexture(tileset->texture, pos.x, pos.y,&r);
			}
		}

		for(auto &elem : layer->tileData)
		{
			if(!elem.active) continue;
			elem.AdvanceTimer();
		}
	}
}

// Ttranslates x,y coordinates from map positions to world positions
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

	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + (tileWidth + spacing) * (relativeIndex % columns);
	rect.y = margin + (tileWidth + spacing) * (relativeIndex / columns);

	return rect;
}


// Pick the right Tileset based on a tile id
TileSet *Map::GetTilesetFromTileId(int gid) const
{
	for(auto &tileset : mapData.tilesets)
		if(gid < (tileset->firstgid + tileset->tilecount))
			return tileset.get();

	LOG("Tileset for gid %i not found", gid);
	return nullptr;
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

	LogLoadedData();

	app->entityManager->LoadAllTextures();
	
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
	for(auto const &elem : mapFile.child("map").children("tileset"))
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

		for(auto const &tileInfoNode : elem.children("tile"))
		{
			retTileSet->tileInfo.insert_or_assign(tileInfoNode.attribute("id").as_int(), LoadTileInfo(tileInfoNode));
		}
		
		mapData.tilesets.emplace_back(std::move(retTileSet));
	}

	return true;
}

std::unique_ptr<TileInfo> Map::LoadTileInfo(const pugi::xml_node &tileInfoNode) const
{
	auto tileInfo = std::make_unique<TileInfo>();

	if(tileInfoNode.attribute("class"))
		tileInfo->type = tileInfoNode.attribute("class").as_string();
	tileInfo->properties = LoadProperties(tileInfoNode);
	tileInfo->collider = LoadHitboxInfo(tileInfoNode, tileInfo->properties);
	tileInfo->animation = LoadAnimationInfo(tileInfoNode, tileInfo->properties);

	return tileInfo;
}

std::shared_ptr<TileAnimationInfo> Map::LoadAnimationInfo(const pugi::xml_node &tileInfoNode, XML_Properties_Map_t const &properties) const
{
	auto retAnim = std::make_shared<TileAnimationInfo>();

	for(auto const &animFrameNode : tileInfoNode.child("animation").children())
	{
		retAnim->frames.push_back(
			std::make_pair(
				animFrameNode.attribute("tileid").as_int(),
				animFrameNode.attribute("duration").as_int()/10
			)
		);
		for(auto const &[name, value] : properties)
		{
			if(name == "variance_min") retAnim->varianceMin = (uint)*std::get_if<int>(&value);
			if(name == "variance_max") retAnim->varianceMax = (uint)*std::get_if<int>(&value);

		}
	}

	return retAnim;
}

TileColliderInfo Map::LoadHitboxInfo(const pugi::xml_node &tileNode, XML_Properties_Map_t const &properties) const
{
	TileColliderInfo retHitBox;

	pugi::xml_node tileObjectGroup = tileNode.child("objectgroup");
	if(tileObjectGroup.empty()) 
		return retHitBox;

	tileObjectGroup = tileObjectGroup.first_child();
	if(tileObjectGroup.empty()) 
		return retHitBox;

	retHitBox.x = tileObjectGroup.attribute("x").as_int();
	retHitBox.y = tileObjectGroup.attribute("y").as_int();
	retHitBox.width = tileObjectGroup.attribute("width").as_int();
	retHitBox.height = tileObjectGroup.attribute("height").as_int();

	if(auto const collisionLayer = properties.find("ColliderLayers"); collisionLayer == properties.end())
		retHitBox.cat = 0x8000;
	else
		retHitBox.cat = (uint16)*std::get_if<int>(&collisionLayer->second);

	pugi::xml_node shapeInfo = tileObjectGroup.first_child();
	if(shapeInfo.empty())
	{
		retHitBox.shape = "rectangle";
		return retHitBox;
	}

	retHitBox.shape = shapeInfo.name();
	const std::string xyStr = shapeInfo.attribute("points").as_string();
	static const std::regex r("\\d{1,3}");
	auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
	auto xyStrEnd = std::sregex_iterator();

	for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
	{
		std::smatch match = *i;
		retHitBox.points.push_back(stoi(match.str()));
	}

	return retHitBox;
}

// Iterate all layers and load each of them
bool Map::LoadAllLayers(pugi::xml_node const &node)
{
	for(auto const &layer : node.children("layer"))
	{
		mapData.mapLayers.push_back(LoadLayer(layer));
	}
	
	for(auto const &objectGroupNode : node.children("objectgroup"))
	{
		for(auto const &objectNode : objectGroupNode.children("object"))
		{
			iPoint position{objectNode.attribute("x").as_int(), objectNode.attribute("y").as_int()};
			int width = objectNode.attribute("width").as_int();
			int height = objectNode.attribute("height").as_int();
			auto gid = objectNode.attribute("gid").as_uint();
			TileSet const *tileset = GetTilesetFromTileId(gid);
			auto aux = tileset->tileInfo.find(gid - tileset->firstgid);
			TileInfo const *tileInfo = aux->second.get();

			app->entityManager->LoadEntities(tileInfo, position, width, height);
		}
	}

	return true;
}

// Loads a single layer
std::unique_ptr<MapLayer> Map::LoadLayer(pugi::xml_node const &node)
{
	auto layer = std::make_unique<MapLayer>();

	//Load the attributes
	layer->id = node.attribute("id").as_int();
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();

	//Iterate over all the tiles and get gid values
	for(iPoint pos = {0,0} ; auto const &elem : node.child("data").children("tile"))
	{	
		TileImage retTileAnim;
		if(int gid = elem.attribute("gid").as_int(); gid > 0)
		{
			TileSet const *tileset = GetTilesetFromTileId(gid);
			if(auto colliderCreated = CreateCollider(gid, pos.x, pos.y, tileset); colliderCreated != nullptr)
				collidersOnMap.emplace_back(colliderCreated);

			retTileAnim.gid = gid;
			retTileAnim.originalGid = gid;
			if(const auto &info = tileset->tileInfo.find(gid-1); 
			   info != tileset->tileInfo.end() && !info->second->animation->frames.empty())
			{
				retTileAnim.active = true;
				retTileAnim.currentFrame = 0;
				retTileAnim.timer = 0;
				retTileAnim.anim = info->second->animation;
				retTileAnim.duration = info->second->animation->frames[0].second;
				retTileAnim.duration += (info->second->animation->varianceMax > 0) ?
					((uint)std::rand() % info->second->animation->varianceMax + info->second->animation->varianceMin) :
					0;
			}
		}
		layer->tileData.emplace_back(retTileAnim);
		pos.x++;
		if(pos.x >= mapData.width)
		{
			pos.y++;
			pos.x = 0;
		}
	}

	layer->properties = LoadProperties(node);

	return std::move(layer);
}

inline std::shared_ptr<PhysBody> Map::CreateCollider(int gid, int i, int j, TileSet const *tileset) const
{
	if(auto colliderInfo = tileset->tileInfo.find(gid-1); colliderInfo != tileset->tileInfo.end())
	{
		if(colliderInfo->second->collider.shape.empty() && colliderInfo->second->collider.points.size() <= 0)
			return nullptr;
		TileColliderInfo collider = colliderInfo->second->collider;
		iPoint colliderPos = MapToWorld(i, j);
		std::shared_ptr<PhysBody> retCollider;
		if(collider.shape == "rectangle")
		{
			colliderPos.x += collider.width/2 + collider.x;
			colliderPos.y += collider.height/2 + collider.y;
			retCollider = app->physics->CreateRectangle(
														colliderPos.x,
														colliderPos.y,
														collider.width,
														collider.height,
														BodyType::STATIC,
														1.0f,
														0.0f,
														collider.cat
			);
		}
		else if(collider.shape == "polygon")
		{
			colliderPos.x += collider.x;
			colliderPos.y += collider.y;
			retCollider = app->physics->CreateChain(
														colliderPos.x,
														colliderPos.y,
														collider.points.data(),
														collider.points.size(),
														BodyType::STATIC,
														0.0f,
														collider.cat
			);
		}
		retCollider->ctype = (ColliderLayers)collider.cat;
		return retCollider;
	}
	return nullptr;
}

XML_Properties_Map_t Map::LoadProperties(pugi::xml_node const &node) const
{
	XML_Properties_Map_t properties;
	for(auto const &elem : node.child("properties").children("property"))
	{
		XML_Property_t valueToEmplace;
		switch(str2int(elem.attribute("type").as_string()))
		{
			case str2int("int"):
			case str2int("object"):
				valueToEmplace = elem.attribute("value").as_int();
				break;
			case str2int("float"):
				valueToEmplace = elem.attribute("value").as_float();
				break;
			case str2int("bool"):
				valueToEmplace = elem.attribute("value").as_bool();
				break;
			default:
				LOG("Variant doesn't have %s type", elem.attribute("type").as_string());
				valueToEmplace = elem.attribute("value").as_string();
				continue;
		}
		
		properties.try_emplace(elem.attribute("name").as_string(), valueToEmplace);
	}
	return properties;
}

// Ask for the value of a custom property
XML_Property_t MapLayer::GetPropertyValue(const char *pName) const
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
	LOG("width : %d					height : %d", mapData.width, mapData.height);
	LOG("tile_width : %d				tile_height : %d", mapData.tileWidth, mapData.tileHeight);

	LOG("Tilesets----");

	// Info for each loaded tileset
	for(auto const &elem : mapData.tilesets)
	{
		LOG("Name : %s	First gid : %d", elem->name.c_str(), elem->firstgid);
		LOG("Tile width : %d				Tile height : %d", elem->tileWidth, elem->tileHeight);
		LOG("Spacing : %d					Margin : %d", elem->spacing, elem->margin);
	}

	LOG("Layers----");

	// Info for each loaded layer
	for(auto const &layer : mapData.mapLayers)
	{
		LOG("Id : %d						Name : %s", layer->id, layer->name.c_str());
		LOG("Layer width : %d				Layer height : %d", layer->width, layer->height);

		for(auto &[key, value] : layer->properties)
		{
			if(value.valueless_by_exception())
			{
				LOG("Property %s has key valueless_by_exception.", key);
				continue;
			}

			switch(value.index())
			{
				case 0:
				{
					LOG("Property: %s				Value: %i ", key.c_str(), value);
					break;
				}
				case 1:
				{
					std::string boolHelper = *(std::get_if<bool>(&value)) ? "true" : "false";
					LOG("Property: %s				Value: %s ", key.c_str(), boolHelper.c_str());
					break;
				}
				case 2:
				{
					std::string strHelper = *(std::get_if<std::string>(&value));
					LOG("Property: %s				Value: %.2f ", key.c_str(), strHelper.c_str());
					break;
				}
				case 3:
				{
					LOG("Property: %s				Value: %s ", key.c_str(), value);
					break;
				}
				default:
					LOG("Case %i of property %s not covered in Switch", value, key.c_str());
					break;
			}

		}
	}
}

int Map::GetWidth() const
{
	return mapData.width;
}

int Map::GetHeight() const
{
	return mapData.height;
}

int Map::GetTileWidth() const
{
	return mapData.tileWidth;
}

int Map::GetTileHeight() const
{
	return mapData.tileHeight;
}

int Map::GetTileSetSize() const
{
	return mapData.tilesets.size();
}