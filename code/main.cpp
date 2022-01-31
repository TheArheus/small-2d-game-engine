#include <memory>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "debug.h"
#include "entity_system.cpp"
#include "asset_store.cpp"

bool is_running;

i32 PreviousFrameTime = 0;
r32 DeltaTime = 0;
r32 TimeForFrame = 0;
r32 dtForFrame = 0;

struct game_level
{
    u32 LevelNumber;
    std::unique_ptr<registry> Registry;
    std::unique_ptr<asset_store> AssetStore;
};

struct game_world
{
    u32 ChosenLevel;
    u32 LevelsCount;
    game_level* Levels;
};

internal void
LoadAllLevelsFileNames(std::vector<std::string>* LevelFileNames, std::vector<std::string>* LevelTexturesFileNames)
{
    char* LevelsRelativePath = "../assets/tilemaps/";
    char LevelsFullPathName[256];
    GetFullPathNameA(LevelsRelativePath, 256, LevelsFullPathName, nullptr);
    snprintf(LevelsFullPathName, 256, "%s*", LevelsFullPathName);

    WIN32_FIND_DATA FindDataFileNames;
    HANDLE FileHandle;

    FileHandle = FindFirstFile(LevelsFullPathName, &FindDataFileNames);

    do
    {
        if(FindDataFileNames.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
        }
        else
        {
            std::string FilePathResult = FindDataFileNames.cFileName;
            std::string FullPathResult = std::string(LevelsFullPathName);
            FullPathResult.pop_back();
            if(FilePathResult[0] != '.')
            {
                if(FilePathResult.substr(FilePathResult.find_last_of(".") + 1) == "map")
                {
                    LevelFileNames->push_back(FullPathResult + FilePathResult);
                }
                else if(FilePathResult.substr(FilePathResult.find_last_of(".") + 1) == "png")
                {
                    LevelTexturesFileNames->push_back(FullPathResult + FilePathResult);
                }
            }
        }
    }
    while(FindNextFile(FileHandle, &FindDataFileNames) != NULL);
}

internal void 
LoadTileMap(game_level* Level, std::string TileMapPath)
{
    r32 TileScale = 2.0f;
    i32 TileWidth = 25;
    i32 TileHeight = 20;
    i32 TileSize = 32;

    std::fstream MapFile;
    MapFile.open(TileMapPath);

    for(u32 Y = 0;
        Y < TileHeight;
        ++Y)
    {
        for(u32 X = 0;
            X < TileWidth;
            ++X)
        {
            char Character;
            MapFile.get(Character);
            u32 SourceRectY = std::atoi(&Character) * TileSize;
            MapFile.get(Character);
            u32 SourceRectX = std::atoi(&Character) * TileSize;
            MapFile.ignore();

            entity Tile = Level->Registry->CreateEntity();
            Level->Registry->AddComponent<transformation_compontent>(Tile, V3(X * (TileSize * TileScale), Y * (TileSize * TileScale), 1), V2(TileScale, TileScale), 0.0f);
            Level->Registry->AddComponent<sprite_component>(Tile, "tilemap_image", TileSize, TileSize, SourceRectX, SourceRectY);
        }
    }
    MapFile.close();
}

internal game_level
LoadLevel(i32 Level, std::string LevelFile, std::string LevelTexture)
{
    game_level NewLevel = {};
    NewLevel.LevelNumber = Level;

    NewLevel.Registry = std::make_unique<registry>();
    NewLevel.AssetStore = std::make_unique<asset_store>();

    NewLevel.Registry->AddSystem<movement_system>();
    NewLevel.Registry->AddSystem<rendering_system>();

    NewLevel.AssetStore->AddTexture("tank_image", "../assets/images/tank-tiger-up.png");
    NewLevel.AssetStore->AddTexture("truck_image", "../assets/images/truck-ford-up.png");

    NewLevel.AssetStore->AddTexture("tilemap_image", LevelTexture);
    LoadTileMap(&NewLevel, LevelFile);

    entity Tank = NewLevel.Registry->CreateEntity();

    NewLevel.Registry->AddComponent<transformation_compontent>(Tank, V3(100.0f, 100.0f, 2), V2(2.0f, 2.0f), 0.0f);
    NewLevel.Registry->AddComponent<body_component>(Tank);
    NewLevel.Registry->AddComponent<sprite_component>(Tank, "tank_image", 32, 32);

    return NewLevel;
}

internal void 
LoadAllLevels(game_world* World)
{
    std::vector<std::string> LevelFileNames;
    std::vector<std::string> LevelTexturesFileNames;
    LoadAllLevelsFileNames(&LevelFileNames, &LevelTexturesFileNames);
    World->LevelsCount = LevelFileNames.size(); // This number will be dynamic

    if(!World->Levels)
    {
        World->Levels = (game_level*)calloc(sizeof(game_level), World->LevelsCount);
    }

    for(u32 LevelNumber = 0;
        LevelNumber < World->LevelsCount;
        ++LevelNumber)
    {
        World->Levels[LevelNumber] = LoadLevel(LevelNumber, LevelFileNames[LevelNumber], LevelTexturesFileNames[LevelNumber]);
    }
}

internal void 
setup(game_world* World)
{

    ColorBuffer->Memory = (u32*)malloc(sizeof(u32)*ColorBuffer->Width*ColorBuffer->Height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                ColorBuffer->Width, ColorBuffer->Height);

    LoadAllLevels(World);
}

internal void 
process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type)
    {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE) is_running = false;
            break;
    }
}

internal void 
update(game_world* World)
{
    dtForFrame += TimeForFrame;
    int TimeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() + PreviousFrameTime);

    if(TimeToWait > 0 && (TimeToWait <= FRAME_TARGET_TIME))
    {
        SDL_Delay(TimeToWait);
    }

    DeltaTime = (SDL_GetTicks() - PreviousFrameTime) / 1000.0f;
    if (DeltaTime > TimeForFrame) DeltaTime = TimeForFrame;
    PreviousFrameTime = SDL_GetTicks();

    World->Levels[World->ChosenLevel].Registry->Update();
    World->Levels[World->ChosenLevel].Registry->GetSystem<movement_system>().Update(DeltaTime);
}

internal void 
render(game_world* World)
{
    RenderColorBuffer();
    ClearColorBuffer(ColorBuffer, 0xFF16161d);//0xFF056263);

    World->Levels[World->ChosenLevel].Registry->GetSystem<rendering_system>().Render(World->Levels[World->ChosenLevel].AssetStore);

    SDL_RenderPresent(renderer);
}

int 
main(int argc, char** argv)
{
    is_running = InitWindow();

    game_world* World = (game_world*)calloc(sizeof(game_world), 1);

    setup(World);

    while(is_running)
    {
        process_input();
        update(World);
        render(World);
    }

    DestroyWindow();

    return 0;
}
