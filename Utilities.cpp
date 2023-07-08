
#include "Utilities.h"

#include "zerrors.h"


#include <fstream>
#include <cmath>
#include <codecvt>


std::string loadTextFile(const std::string& fileName) {
    std::ifstream input;
    input.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        input.open(fileName, std::ios::binary);
    }
    catch (const std::exception& exc) {
        ZTHROW(FileNotFoundException()) << "Could not open input file: '" << fileName << "'. Error: " << exc.what();
    }

    try {
        // @todo This is an extremely inefficient implementation.
        std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return data;
    }
    catch (const std::exception& exc) {
        ZTHROW() << "Error while reading file: '" << fileName << "'. Error: " << exc.what();
    }
}

std::tuple<int, float> divide(float value, float divisor) {
    ZASSERT(divisor > 0.0f);

    float integral;
    float fractional = std::modf(std::fabs(value) / std::fabs(divisor), &integral);
    int result = static_cast<int>(integral);
    float remainder = fractional * divisor;
    if (std::signbit(value)) {
        result = -result - 1;
        remainder = 1.0f - remainder;
    }

    return { result, remainder };
}

raylib::Rectangle loadJsonRect(const nlohmann::json& json) {
    return { json["x"].get<float>(), json["y"].get<float>(), json["width"].get<float>(), json["height"].get<float>() };
}

// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest.
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;

    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                    Rectangle{dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, Rectangle{dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }

        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                        Rectangle{dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, Rectangle{dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                        Rectangle{dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }

            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }

        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                        Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }

            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}

std::map<char32_t, char32_t> dropDiacriticsMap = {
    {U'ź', U'z'},
    {U'ń', U'n'},
    {U'ż', U'z'},
    {U'ó', U'o'},
    {U'ł', U'l'},
    {U'ś', U's'},
    {U'ć', U'c'},
    {U'ę', U'e'},
    {U'ą', U'a'},

    {U'Ź', U'Z'},
    {U'Ń', U'N'},
    {U'Ż', U'Z'},
    {U'Ó', U'O'},
    {U'Ł', U'L'},
    {U'Ś', U'S'},
    {U'Ć', U'C'},
    {U'Ę', U'E'},
    {U'Ą', U'A'},
};

std::string textForFont(bool allowLowercase, bool allowDiacritics, const std::u32string& text) {
    std::u32string newText;
    for (auto c32 : text) {
        char32_t c = c32;
        if (dropDiacriticsMap.contains(c)) {
            if (!allowDiacritics)
                c = dropDiacriticsMap[c];
        }
        else
        if ((c < 32) || (c > 127)) {
            c = U'?';
        }
        if (!allowLowercase)
            c = std::toupper(c);
        newText.push_back(c);
    }

    std::wstring_convert<std::codecvt<char32_t, char, std::mbstate_t>, char32_t> utfConverter;
    auto outText = utfConverter.to_bytes(newText);
    return outText;
}

std::vector<int> loadCharset(const std::string& fileName) {
    char* text = LoadFileText(fileName.c_str());
    int codepointsCount = 0;
    int* codepoints = LoadCodepoints(text, &codepointsCount);
    std::vector<int> charset(codepoints, codepoints + codepointsCount);
    UnloadFileText(text);
    UnloadCodepoints(codepoints);
    return charset;
}

std::u32string loadUnicodeStringFromJson(const nlohmann::json& json, const std::string& key) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utfConverter;
    auto textRaw = json[key].get<std::string>();
    auto text = utfConverter.from_bytes(textRaw);
    return text;
}
