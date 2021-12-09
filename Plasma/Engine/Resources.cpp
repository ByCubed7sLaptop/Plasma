// By @ByCubed7 on Twitter

#include "Resources.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <bit>

#include "../Library/stb_image.h"

#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H  

using namespace std;


// Cache any resources we load
map<string, Texture2D> Resources::Textures;
map<string, Shader> Resources::Shaders;
map<string, Font> Resources::Fonts;
map<string, Tilemaps::Tilemap> Resources::tilemaps;
map<string, Wav> Resources::wavs;

unsigned int Resources::defaultFontSize = 24;

// Shader

Shader Resources::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, string name)
{
    Shaders[name] = LoadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader& Resources::GetShader(string name) { return Shaders[name]; }


// Texture

Texture2D Resources::LoadTexture(const string file, bool alpha, string name)
{
    Textures[name] = LoadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture2D& Resources::GetTexture(string name) { return Textures[name]; }


// Font

Font Resources::LoadFont(const string file, string name)
{
    Fonts[name] = LoadFontFromFile(file, defaultFontSize);
    return Fonts[name];
}

Font& Resources::GetFont(string name) { return Fonts[name]; }

// Tilemaps

Tiled::Loader Resources::tiled = Tiled::Loader();

Tilemaps::Tilemap Resources::LoadTilemap(const string file, string name)
{
    Tilemaps::Tilemap map;

    // What happens when we can't find the file?
    // Invalid format? ect.
    tiled.LoadMap(name, file);
    cout << "Loaded Map" << endl;

    Tiled::Map* tiledMap = tiled.GetMap(name);
    cout << "Got Map" << endl;

    // Convert the loaded map into a tilemap
    vector<Tiled::Layer> layers = tiledMap->Layers();

    for (int j = 0; j < layers.size(); j++)
    {
        cout << layers[j].Tiles().size() << endl;

        auto tiles = layers[j].Tiles();
        map.AddLayer();

        //*
        for (int y = 0; y < tiledMap->Height(); ++y) {
            for (int x = 0; x < tiledMap->Width(); ++x) {
                unsigned id = tiles[y][x];

                // Read the flags
                bool flipH = (id & Tiled::FLIPPED_HORIZONTALLY_FLAG);
                bool flipV = (id & Tiled::FLIPPED_VERTICALLY_FLAG);
                bool flipD = (id & Tiled::FLIPPED_DIAGONALLY_FLAG);

                // Clear the flags
                id &= ~(
                    Tiled::FLIPPED_HORIZONTALLY_FLAG    |
                    Tiled::FLIPPED_VERTICALLY_FLAG      |
                    Tiled::FLIPPED_DIAGONALLY_FLAG
                );

                // Converts the boolean mirror matrix to a rotation float and scale vector
                // NOTE: This took me 5 hours and I found it by accedent at like 4am
                glm::vec2 scale = glm::vec2(
                   flipH ^ flipD ? -1 : 1,
                   flipV ? -1 : 1
                );

                if (flipD) {
                    int temp = scale.x;
                    scale.x = scale.y;
                    scale.y = temp;
                }

                int rotat = flipD ? 90 : 0;
                
                // Add the tile
                if (id != 0) 
                {
                    map.AddTile(
                        j, 
                        map.tileset.GetIndexFromId(id), 
                        glm::vec2(x, y), 
                        rotat, scale
                    );
                }
            }
        }
    }//*/

    /*
    for (int i = 0; i < layers.size(); i++ )
    {
        map.AddLayer();

        auto layer = layers[i];
        for (int y = 0; y < layer.Tiles().size(); y++)
        {
            auto row = layer.Tiles()[y];
            for (int x = 0; x < row.size(); x++)
            {
                map.AddTile(i, row[x], glm::vec2(x, y));
            }
        }
    }*/

    tilemaps[name] = map;

    return map;
}

Tilemaps::Tilemap& Resources::GetTilemap(string name)
{
    return tilemaps[name];
}

Wav Resources::LoadWav(const std::string file, string name)
{
    Wav wav;

    std::ifstream in(file, std::ios::binary);
    if (!in.is_open())
        std::cerr << "ERROR: Could not open \"" << file << "\"" << std::endl;

    if (!LoadWavHeader(in, wav.channels, wav.sampleRate, wav.bitsPerSample, wav.size))
        std::cerr << "ERROR: Could not load wav header of \"" << file << "\"" << std::endl;

    char* data = new char[wav.size];
    in.read(data, wav.size);

    wav.data = vector<char>(data, data + wav.size);

    wav.name = name;
    wav.filepath = file;

    wavs[name] = wav;
    return wav;
}

Wav Resources::GetWav(string name)
{
    return wavs[name];
}

bool Resources::LoadWavHeader(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, ALsizei& size)
{
    char buffer[4];
    if (!file.is_open())
        return false;

    // the RIFF
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read RIFF" << std::endl;
        return false;
    }
    if (std::strncmp(buffer, "RIFF", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (header doesn't begin with RIFF)" << std::endl;
        return false;
    }

    // the size of the file
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read size of file" << std::endl;
        return false;
    }

    // the WAVE
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read WAVE" << std::endl;
        return false;
    }
    if (std::strncmp(buffer, "WAVE", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (header doesn't contain WAVE)" << std::endl;
        return false;
    }

    // "fmt/0"
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read fmt/0" << std::endl;
        return false;
    }

    // this is always 16, the size of the fmt data chunk
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read the 16" << std::endl;
        return false;
    }

    // PCM should be 1?
    if (!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read PCM" << std::endl;
        return false;
    }

    // the number of channels
    if (!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read number of channels" << std::endl;
        return false;
    }

    // convert_to_int
    std::memcpy(&channels, buffer, 2);

    // sample rate
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read sample rate" << std::endl;
        return false;
    }
    // convert_to_int
    std::memcpy(&sampleRate, buffer, 4);

    // (sampleRate * bitsPerSample * channels) / 8
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read (sampleRate * bitsPerSample * channels) / 8" << std::endl;
        return false;
    }

    // ?? dafaq
    if (!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read dafaq" << std::endl;
        return false;
    }

    // bitsPerSample
    if (!file.read(buffer, 2))
    {
        std::cerr << "ERROR: could not read bits per sample" << std::endl;
        return false;
    }
    // convert_to_int
    std::memcpy(&bitsPerSample, buffer, 2);

    // data chunk header "data"
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read data chunk header" << std::endl;
        return false;
    }
    if (std::strncmp(buffer, "data", 4) != 0)
    {
        std::cerr << "ERROR: file is not a valid WAVE file (doesn't have 'data' tag)" << std::endl;
        return false;
    }

    // size of data
    if (!file.read(buffer, 4))
    {
        std::cerr << "ERROR: could not read data size" << std::endl;
        return false;
    }
    // convert_to_int
    std::memcpy(&size, buffer, 4);

    /* cannot be at the end of file */
    if (file.eof())
    {
        std::cerr << "ERROR: reached EOF on the file" << std::endl;
        return false;
    }
    if (file.fail())
    {
        std::cerr << "ERROR: fail state set on the file" << std::endl;
        return false;
    }

    return true;
}

//

void Resources::Clear()
{
    // Delete all the shaders and textures
    for (auto iter : Shaders) glDeleteProgram(iter.second.program);
    for (auto iter : Textures) glDeleteTextures(1, &iter.second.ID);
}

//

Shader Resources::LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
    // Retrieve the vertex/fragment source code from filePath
    string vertexCode;
    string fragmentCode;
    string geometryCode;

    try
    {
        // Open files
        ifstream vertexShaderFile(vShaderFile);
        ifstream fragmentShaderFile(fShaderFile);
        stringstream vShaderStream, fShaderStream;

        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        // Close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();

        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // If geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            ifstream geometryShaderFile(gShaderFile);
            stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (exception e)
    {
        // Raise error
        cout << "ERROR::SHADER: Failed to read shader files" << endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str(); // <-- Could raise a bug in the future
    
    // Create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    
    return shader;
}

Texture2D Resources::LoadTextureFromFile(const string file, bool alpha)
{
    // Create texture object
    Texture2D texture;
    
    if (alpha)
    {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }

    // Load
    int width, height, nrChannels;
    unsigned char* data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);

    // Generate texture
    texture.Generate(width, height, data);
    
    // Free image data
    stbi_image_free(data);
    return texture;
}


Font Resources::LoadFontFromFile(const string file, unsigned int size)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        cout << "ERROR: Could not init FreeType Library" << endl;
        //return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, file.c_str(), 0, &face))
    {
        cout << "ERROR: Failed to load font" << endl;
        //return -1;
    }


    FT_Set_Pixel_Sizes(face, 0, defaultFontSize);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    Font newFont = Font();

    for (unsigned char c = 0; c < 128; c++) // Only do 128 characters~
    {
        // Load glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            cout << "ERROR: Failed to load Glyph" << endl;
            continue;
        }

        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create Character
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        // Store character
        //newFont.AddCharacter(c, character);
        newFont.Characters[c] = character;
    }
    // Clear FreeType's resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return newFont;
}

vector<char> Resources::LoadWavFromFile(const std::string file)
{
    return vector<char>();
}
