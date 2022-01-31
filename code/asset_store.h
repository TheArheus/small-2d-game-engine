
#if !defined ASSET_STORE_H

typedef std::map<std::string, texture_t*> texture_storage;

class asset_store
{
private:
    texture_storage Textures;

public:
    asset_store()  = default;
    ~asset_store() = default;

    void ClearAssets();
    void AddTexture(const std::string AssetID, const std::string& FilePath);
    texture_t* GetTexture(const std::string AssetID);

};

#define ASSET_STORE_H
#endif
