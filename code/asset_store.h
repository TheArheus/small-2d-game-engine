
#if !defined ASSET_STORE_H

typedef std::map<std::string, texture_t*> texture_storage;

typedef std::unordered_map<u32, glyph_t*> font_t;
typedef std::map<std::string, font_t> font_storage;

class asset_store
{
private:
    texture_storage Textures;
    font_storage Fonts;

public:
    asset_store()  = default;
    ~asset_store() = default;

    void ClearAssets();

    void AddTexture(const std::string AssetID, const std::string& FilePath);
    texture_t* GetTexture(const std::string AssetID);

    void AddFont(const std::string AssetID, const std::string& FilePath, i32 FontSize);
    font_t GetFont(const std::string AssetID);
};

#define ASSET_STORE_H
#endif
