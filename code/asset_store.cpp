#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void asset_store::
ClearAssets()
{
    for(auto Texture : Textures)
    {
        free(Texture.second->Memory);
        free(Texture.second);
    }
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
