#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_truetype.h"

void asset_store::
ClearAssets()
{
    for(auto Texture : Textures)
    {
        free(Texture.second->Memory);
        free(Texture.second);
    }

    // TODO: Correct font storage clear
    for(auto Font : Fonts)
    {
        for(auto Glyph : Font.second)
        {
            free(Glyph.second->Memory);
            free(Glyph.second);
        }

        Font.second.clear();
    }

    Textures.clear();
    Fonts.clear();
}

void asset_store::
AddTexture(const std::string AssetID, const std::string& FilePath)
{
    i32 Width, Height, Pitch;
    unsigned char* Data = stbi_load(FilePath.c_str(), &Width, &Height, &Pitch, 0);

    if(!Data) { MessageErr("Not available to load texture: " + AssetID); return; }

    texture_t* Result = (texture_t*)malloc(sizeof(texture_t));

    Result->Memory = (u32*)malloc(Width * Height * sizeof(u32));
    Result->Memory = (u32*)Data;
    Result->Width = Width;
    Result->Height = Height;

    Textures.emplace(AssetID, Result);
}

texture_t* asset_store::
GetTexture(const std::string AssetID)
{
    texture_t* Result = Textures[AssetID];
    return Result;
}

internal glyph_t*
CreateFontChar(stbtt_fontinfo* Font, i32 FontSize, u32 CodePoint)
{
    i32 Width;
    i32 Height;
    i32 OffsetX;
    i32 OffsetY;
    u8* FontBitmap = stbtt_GetCodepointBitmap(Font, 0, stbtt_ScaleForPixelHeight(Font, FontSize), CodePoint, &Width, &Height, &OffsetX, &OffsetY);

    glyph_t* FontResult = (glyph_t*)malloc(sizeof(glyph_t));
    FontResult->Memory = (u32*)malloc(Width * Height * sizeof(u32));
    FontResult->OffsetX = OffsetX;
    FontResult->OffsetY = OffsetY;
    FontResult->Width = Width;
    FontResult->Height = Height;
    FontResult->FontSize = FontSize;

    u32 FontPitch = Width * sizeof(u32);

    u8* SrcRow = FontBitmap;
    u8* DstRow = (u8*)FontResult->Memory;
    for(u32 Y = 0;
        Y < Height;
        ++Y)
    {
        u32* DstPixel = (u32*)DstRow;
        for(u32 X = 0;
            X < Width;
            ++X)
        {
            u8 Gray  = *SrcRow++;
            //u8 Alpha = 0xFF;

            *DstPixel++ = ((Gray << 24) |
                           (Gray << 16) |
                           (Gray <<  8) |
                           (Gray <<  0));
        }
        DstRow += FontPitch;
    }

    stbtt_FreeBitmap(FontBitmap, 0);

    return FontResult;
}

void asset_store::
AddFont(const std::string AssetID, const std::string& FilePath, i32 FontSize)
{
    u32 ContentSize = 0;
    void* Content = nullptr;

    FILE* In = fopen(FilePath.c_str(), "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        ContentSize = ftell(In);
        fseek(In, 0, SEEK_SET);

        Content = malloc(ContentSize);
        fread(Content, ContentSize, 1, In);
        fclose(In);
    }
    else
    {
        MessageErr("Could not read specified file");
    }

    if(Content)
    {
        stbtt_fontinfo Font;
        stbtt_InitFont(&Font, (u8*)Content, stbtt_GetFontOffsetForIndex((u8*)Content, 0));

        font_t Result;

        Result.emplace(' ', CreateFontChar(&Font, FontSize, ' '));
        for(u32 Character = '!';
            Character <= '~';
            ++Character)
        {
            Result.emplace(Character, CreateFontChar(&Font, FontSize, Character));
        }

        Fonts.emplace(AssetID, Result);
    }
}

font_t asset_store::
GetFont(const std::string AssetID)
{
    font_t Result = Fonts[AssetID];
    return Result;
}

