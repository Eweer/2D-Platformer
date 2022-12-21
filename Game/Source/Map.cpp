#include "Map.h"
#include "App.h"
#include "Render.h"
#include "EntityManager.h"

#include "Log.h"
#include "BitMaskColliderLayers.h"

#include <memory>
#include <cmath>
#include <algorithm>
#include <utility>
#include <regex>
#include <variant>

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
		DrawLayer(layer.get());

		// Advance Tile animations of Layer
		for(auto &elem : layer->tileData)
		{
			if(!elem.active) continue;
			elem.AdvanceTimer();
		}
	}
}

void Map::DrawLayer(const MapLayer *layer) const
{
	if(auto const drawProperty = layer->GetPropertyValue("Draw");
		   !*(std::get_if<bool>(&drawProperty)))
	{
		return;
	}

	for(int x = 0; x < layer->width; x++)
	{
		for(int y = 0; y < layer->height; y++)
		{
			uint gid = layer->GetGidValue(x, y);

			if(gid <= 0) continue;

			TileSet *tileset = GetTilesetFromTileId(gid);

			SDL_Rect r = tileset->GetTileRect(gid);
			iPoint pos = MapToWorld(x, y);

			app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
		}
	}
}

bool Map::Pause(int phase)
{
	if(!mapLoaded || phase == 1 || phase == 3)
		return true;

	for(auto const &layer : mapData.mapLayers)
	{
		DrawLayer(layer.get());
	}

	return true;
}

// Translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	return {x * mapData.tileWidth, y * mapData.tileHeight};
}

// Translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(iPoint position) const
{
	return {position.x * mapData.tileWidth, position.y * mapData.tileHeight};
}

// Translates world positions to x,y coordinates
iPoint Map::WorldToCoordinates(iPoint position) const
{
	return position / iPoint(mapData.tileWidth, mapData.tileHeight);
}

// Translates world positions to x coordinates
int Map::WorldXToCoordinates(int n) const
{
	return n / mapData.tileWidth;
}

int Map::WorldYToCoordinates(int n) const
{
	return n / mapData.tileHeight;
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
		retTileSet->texture = app->tex->Load(path.c_str()).get();

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

	tileInfo->properties = LoadProperties(tileInfoNode);
	tileInfo->collider = LoadHitboxInfo(tileInfoNode, tileInfo->properties);
	std::ranges::reverse(tileInfo->collider);
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

std::vector<TileColliderInfo> Map::LoadHitboxInfo(const pugi::xml_node &tileNode, XML_Properties_Map_t const &properties) const
{
	std::vector<TileColliderInfo> retVec;

	if(pugi::xml_node tileObjectGroup = tileNode.child("objectgroup"); 
	   tileObjectGroup.empty())
		return retVec;

	for(auto const &elem : tileNode.child("objectgroup").children())
	{
		TileColliderInfo retHitBox;

		retHitBox.x = elem.attribute("x").as_int();
		retHitBox.y = elem.attribute("y").as_int();
		retHitBox.width = elem.attribute("width").as_int();
		retHitBox.height = elem.attribute("height").as_int();

		if(auto const collisionLayer = properties.find("ColliderLayers"); collisionLayer == properties.end())
			retHitBox.cat = 0x8000;
		else
			retHitBox.cat = (uint16)*std::get_if<int>(&collisionLayer->second);

		pugi::xml_node shapeInfo = elem.first_child();
		if(shapeInfo.empty())
		{
			retHitBox.shape = "rectangle";
			retHitBox.x += retHitBox.width / 2;
			retHitBox.y += retHitBox.height / 2;
			retHitBox.points.push_back(
				{ 
					(float32)(retHitBox.width / 2),
					(float32)(retHitBox.height / 2)
				}
			);
			retVec.emplace_back(retHitBox);
			return retVec;
		}

		retHitBox.shape = shapeInfo.name();
		const std::string xyStr = shapeInfo.attribute("points").as_string();
		static const std::regex r(R"((-?\d{1,3})(?:\.\d+)*,(-?\d{1,3})(?:\.\d+)*)");
		auto xyStrBegin = std::sregex_iterator(xyStr.begin(), xyStr.end(), r);
		auto xyStrEnd = std::sregex_iterator();

		for(std::sregex_iterator i = xyStrBegin; i != xyStrEnd; ++i)
		{
			std::smatch match = *i;
			retHitBox.points.push_back(
				{
					PIXEL_TO_METERS(stoi(match[1].str())),
					PIXEL_TO_METERS(stoi(match[2].str()))
				}
			);
		}

		if(StrEquals(retHitBox.shape, "polygon") && retHitBox.points.size() > b2_maxPolygonVertices * 2)
			retHitBox.shape = "chain";

		retVec.emplace_back(retHitBox);
	}
		
	return retVec;
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
			if(objectNode.child("properties").empty())
			{
				auto gid = objectNode.attribute("gid").as_uint();
				TileSet const *tileset = GetTilesetFromTileId(gid);
				auto aux = tileset->tileInfo.find(gid - tileset->firstgid);
				TileInfo const *tileInfo = aux->second.get();

				app->entityManager->LoadEntities(tileInfo, position, width, height);
			}
			else
			{
				using enum CL::ColliderLayers;
				std::vector<b2Vec2> temp;
				temp.emplace_back(b2Vec2(static_cast<float>(width/2), static_cast<float>(height/2)));
				iPoint pos(width/2, height/2);
				ShapeData shapeData("rectangle", temp);
				auto body = app->physics->CreateBody(position + pos);
				auto fixtureDef = app->physics->CreateFixtureDef(shapeData, static_cast<uint>(TRIGGERS), static_cast<uint>(PLAYER),true);
				body->CreateFixture(fixtureDef.get());
				auto pbPtr = app->physics->CreatePhysBody(body, iPoint(width, height), TRIGGERS);
				terrainColliders.push_back(std::move(pbPtr));

			}
		}
	}

	app->entityManager->LoadItemAnimations();

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
	for(iPoint pos = {0,0}; 
		auto const &elem : node.child("data").children("tile"))
	{	
		TileImage retTileAnim;
		if(int gid = elem.attribute("gid").as_int(); gid > 0)
		{
			TileSet const *tileset = GetTilesetFromTileId(gid);
			if(auto colliderCreated = CreateCollider(gid, pos.x, pos.y, tileset);
			   colliderCreated != nullptr)
			{
				terrainColliders.emplace_back(std::move(colliderCreated));
			}

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

inline std::unique_ptr<PhysBody> Map::CreateCollider(int gid, int i, int j, TileSet const *tileset) const
{
	if(auto colliderInfo = tileset->tileInfo.find(gid-1);
	   colliderInfo != tileset->tileInfo.end())
	{
		if(colliderInfo->second->collider.empty()) return nullptr;
		
		TileColliderInfo collider = colliderInfo->second->collider[0];

		if(collider.shape.empty() || collider.points.empty())
			return nullptr;

		auto retCollider = std::make_unique<PhysBody>();

		ShapeData shape(collider.shape, collider.points);

		iPoint colliderPos = MapToWorld(i, j);
		colliderPos.x += collider.x;
		colliderPos.y += collider.y;

		retCollider = app->physics->CreateQuickPlatform(
			shape,
			{colliderPos.x, colliderPos.y},
			{collider.width, collider.height}
		);

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
				valueToEmplace = elem.attribute("value").as_string();
				break;
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

std::unique_ptr<navPointMatrix> Map::CreateWalkabilityMap()
{
	return CreateWalkabilityNodes();
}


std::unique_ptr<navPointMatrix> Map::CreateWalkabilityNodes() const
{
	auto groundWalkabilityMap = std::make_unique<navPointMatrix>();
	for(int i = 0; i < mapData.width; i++)
	{
		std::vector<NavPoint> aux;
		for(int j = 0; j < mapData.height; j++)
			aux.emplace_back();
		groundWalkabilityMap->emplace_back(aux);
	}

	for(auto const &layer : mapData.mapLayers)
	{
		for(int y = 0; y < layer->height - 1; y++)
		{
			bool platformStarted = false;
			std::vector<NavPoint> row;
			for(int x = 0; x < layer->width - 1; x++)
			{
				using enum CL::NavType;
				// If we already have a NavPoint of another layer we don't want to overwrite it
				if(groundWalkabilityMap->at(x).at(y).type != NONE) continue;

				// Check if current tile is a free node
				uint currentTileGid = layer->GetGidValue(x, y);
				if(IsWalkable(currentTileGid) || IsTerrain(currentTileGid))
				{
					groundWalkabilityMap->at(x).at(y).type = TERRAIN;
					continue;
				}

				// Check if bottom tile is walkable terrain
				uint lowerGid = layer->GetGidValue(x, y + 1);
				if(lowerGid <= 0 || !IsWalkable(lowerGid)) continue;

				// If platform is not started, we assign it as left edge and start a new platform
				if(!platformStarted)
				{
					platformStarted = true;
					groundWalkabilityMap->at(x).at(y).type = LEFT;
				}

				// Check lower right tile
				uint lowerRightGid = layer->GetGidValue(x + 1, y + 1);
				// If there's no tile
				if(lowerRightGid <= 0)
				{
					if(groundWalkabilityMap->at(x).at(y).type == LEFT) groundWalkabilityMap->at(x).at(y).type = SOLO;
					else groundWalkabilityMap->at(x).at(y).type = RIGHT;
					platformStarted = false;
				}

				if((IsTerrain(lowerRightGid) || IsWalkable(lowerRightGid)) && groundWalkabilityMap->at(x).at(y).type != LEFT)
					groundWalkabilityMap->at(x).at(y).type = PLATFORM;

				// Check right tile
				uint rightGid = layer->GetGidValue(x + 1, y);
				// If there's no tile
				if(rightGid <= 0 && groundWalkabilityMap->at(x).at(y).type != LEFT) continue;

				// If there's info about the tile
				if(IsTerrain(rightGid) || IsWalkable(rightGid))
				{
					if(groundWalkabilityMap->at(x).at(y).type == LEFT) groundWalkabilityMap->at(x).at(y).type = SOLO;
					else groundWalkabilityMap->at(x).at(y).type = RIGHT;
					platformStarted = false;
				}
			}
		}
	}

	return groundWalkabilityMap;
}

bool Map::IsWalkable(uint gid) const
{
	TileSet *tileset = GetTilesetFromTileId(gid);
	// If there's info about the tile
	if(auto tileInfo = tileset->tileInfo.find(gid-1);
	tileInfo != tileset->tileInfo.end())
	{
		// If it's a collisionable tile
		auto walkProperty = (*std::get_if<bool>(&tileInfo->second->properties.find("Walkability")->second));
		if(walkProperty)
		{
			return true;
		}
	}
	return false;
}

bool Map::IsTerrain(uint gid) const
{
	TileSet *tileset = GetTilesetFromTileId(gid);
	// If there's info about the tile
	if(auto tileInfo = tileset->tileInfo.find(gid-1);
	tileInfo != tileset->tileInfo.end())
	{
		// If it's a collisionable tile
		auto walkProperty = (*std::get_if<bool>(&tileInfo->second->properties.find("Terrain")->second));
		if(walkProperty)
		{
			return true;
		}
	}
	return false;
}

std::string_view Map::GetMapFolderName() const
{
	return mapFolder;
}
