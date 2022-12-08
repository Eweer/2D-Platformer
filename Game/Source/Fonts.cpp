#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Textures.h"
#include "Render.h"
#include "Fonts.h"

#include <ranges>
#include <algorithm>
#include <string>

// Constructor
Fonts::Fonts() : Module()
{
	name = "fonts";
}

// Destructor
Fonts::~Fonts() = default;

bool Fonts::Awake(pugi::xml_node &config)
{
	path = config.child("property").attribute("path").as_string();
	return true;
}

// Loads the font named fontName with the folder path specified in config.xml.
// Returns:
// new font id, the font id if it was already loaded, or -1 if the texture or xml could not be loaded.
// WARNING: The font has to have a .xml file. If it doesn't, use the other Load function.
int Fonts::Load(std::string const &fontName)
{
	// Check if the font is already loaded
	auto isSameName = [&fontName](Font const &f) { return StrEquals(f.name, fontName); };
		
	if(auto result = std::ranges::find_if(fonts, isSameName);
	   result != fonts.end())
	{
		LOG("Font %s already loaded", fontName.c_str());
		return std::distance(fonts.begin(), result);
	}

	pugi::xml_document fontFile;
	std::string fullName = path + fontName + ".xml";
	if(pugi::xml_parse_result parseResult = fontFile.load_file(fullName.c_str());
	   parseResult)
	{
		Font newFont = {};
		auto fontNode = fontFile.child("font");
		
		std::string texturePath = path + fontNode.child("properties").child("texture").attribute("file").as_string();
		
		if(newFont.graphic = app->tex->Load(texturePath.c_str());
		   !newFont.graphic) 
		{
			return -1;
		}
		
		auto propertiesNode = fontNode.child("properties");
		
		newFont.spacing = {
			propertiesNode.child("info").attribute("xspacing").as_int(),
			propertiesNode.child("info").attribute("yspacing").as_int()
		};
		
		newFont.lineHeight = propertiesNode.child("common").attribute("lineHeight").as_int();
		
		newFont.scale = {
			propertiesNode.child("common").attribute("scaleW").as_float()/100.f,
			propertiesNode.child("common").attribute("scaleY").as_float()/100.f
		};
		
		for(auto const &elem : fontNode.child("chars"))
		{
			FontCharacter newChar = {};
			
			newChar.xAdvance = elem.attribute("xadvance").as_int();
			
			newChar.rect = {
				elem.attribute("x").as_int(),
				elem.attribute("y").as_int(),
				elem.attribute("width").as_int(),
				elem.attribute("height").as_int()
			};

			newChar.offset = {
				elem.attribute("xoffset").as_int(),
				elem.attribute("yoffset").as_int()
			};
			
			newFont.fontTable.try_emplace(
				elem.attribute("letter").as_string()[0],
				newChar
			);
		}
		if(freeVectorElements.empty()) [[likely]]
			fonts.push_back(newFont);
		else [[unlikely]]
		{
			fonts[freeVectorElements.top()] = newFont;
			freeVectorElements.pop();
		}
		LOG("Loaded font %s", fontName.c_str());
	}
	else LOG("Error loading font file %s: %s", fullName.c_str(), parseResult.description());

	return fonts.size() - 1;
}

void Fonts::UnLoad(int font_id)
{
	if(fonts.size() < INT_MAX && in_range(font_id, 0, static_cast<int>(fonts.size())))
	{
		freeVectorElements.emplace(font_id);
		LOG("Successfully Unloaded BMP font_id %d", font_id);
	}
	else
	{
		LOG("Something went very wrong and fonts vector is too big.");
		LOG("If you see this error, please contact me :(");
	}
}

void Fonts::Draw(std::string_view text, iPoint position, int fontId, bool isFixed, std::pair<FontDrawNewLine, int> maxX, iPoint pivot, double angle) const
{
	if(!in_range(fontId, 0, static_cast<int>(fonts.size())))
	{
		LOG("%s: Invalid font id %d", __func__, fontId);
		return;
	}
	if(text.empty())
	{
		LOG("%s: Text is empty", __func__);
		return;
	}
	
	Font const &font = fonts[fontId];
	int xAdvance = 0;
	// 0 = no new line, 1 = new line on space, 2 = new line now
	int newLine = 0;
	
	// Fix X position if it's a static text on window
	if(isFixed)
	{
		position.x += app->render->camera.x * -1;
	}
	
	for(int i = 0; auto const &elem : text)
	{
		// If the letter has a character on the map
		if(auto it = font.fontTable.find(elem);
		   it != font.fontTable.end())
		{
			using enum FontDrawNewLine;
			
			switch(newLine)
			{
				// If newLine == 1, we need to wait until a space to go to next line
				case 1: [[likely]]
					if(elem != std::string(" ")[0]) break;
					[[fallthrough]];
				// If newLine == 2, we need to go to next line now
				case 2:
					position.y += font.lineHeight;
					xAdvance = 0;
					newLine = 0;
					//In any of the two cases, if elem is an space we don't draw it
					if(elem == std::string(" ")[0]) continue;
					break;
				//If newLine == 0, we don't need to do anything
				default:
					break;
			}

			SDL_Rect rect = it->second.rect;
			app->render->DrawTexture(font.graphic,
									 position.x + xAdvance + it->second.offset.x,
									 position.y + it->second.offset.y,
									 &rect,
									 1.0,
									 angle,
									 pivot.x,
									 pivot.y
			);
			
			xAdvance += it->second.xAdvance + 2;

			if(!newLine)
			{
				newLine = CheckIfNewLine(position.x, xAdvance, maxX, i);
				if(maxX.first == NUMBER_OF_CHARS);
			}

		}
		else LOG("Character %s could not be found in %s", elem, font.name.c_str());
	}
}

// WARNING: Modifies i. If you need to use it after calling this function, you need to do it before
int Fonts::CheckIfNewLine(int x, int xAdvance, std::pair<FontDrawNewLine, int> maxX, int &i) const
{
	auto const &[mode, max] = maxX;
	using enum FontDrawNewLine;
	
	if(mode == SCREEN_FIT && xAdvance + x > app->render->camera.w - app->render->camera.x) 
		return 2;
	
	if(mode == NONE || max == 0) 
		return 0;
	
	if(mode == NUMBER_OF_CHARS) {
		if(max < i)
		{
			i = 0;
			return 1;
		}
		i++;
	}
	
	if(mode == POSITION_X_ON_MAP && max < xAdvance + x)
		return 1;
	
	if(mode == POSITION_X_ON_SCREEN && max < xAdvance)
		return 1;
	
	return 0;
}
