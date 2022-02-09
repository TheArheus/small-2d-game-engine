#include <memory>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "events.h"
#include "systems.h"
#include "entity_system.cpp"
#include "asset_store.cpp"

bool is_running;

camera Camera;
i32 PreviousFrameTime = 0;
r32 DeltaTime = 0;
r32 TimeForFrame = 0;
r32 dtForFrame = 0;

struct game_level
{
    u32 LevelNumber;
    std::unique_ptr<registry> Registry;
    std::unique_ptr<asset_store> AssetStore;
    std::unique_ptr<event_bus> EventBus;

    i32 MapWidth;
    i32 MapHeight;
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
LoadTileMap(game_level* Level, std::string TileMapPath, r32 TileScale, i32* MapWidth = nullptr, i32* MapHeight = nullptr)
{
    i32 TileWidth  = 25;
    i32 TileHeight = 20;
    i32 TileSize   = 32;

    if(MapWidth || MapHeight)
    {
        *MapWidth  = 0;
        *MapHeight = 0;
        *MapWidth  = TileWidth  * TileSize * TileScale;
        *MapHeight = TileHeight * TileSize * TileScale;
    }

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
            v3 EntityPos = V3(X * (TileSize * TileScale), Y * (TileSize * TileScale), 1);// + V3(Camera.P, 0.0f);
            Level->Registry->AddComponent<transformation_compontent>(Tile, EntityPos, V2(TileScale, TileScale), 0.0f);
            Level->Registry->AddComponent<sprite_component>(Tile, "tilemap_image", TileSize, TileSize, false, SourceRectX, SourceRectY);
        }
    }
    MapFile.close();
}

internal game_level
LoadLevel(i32 Level, std::string LevelFile, std::string LevelTexture, r32 TileScale)
{
    game_level NewLevel = {};
    NewLevel.LevelNumber = Level;

    NewLevel.Registry = std::make_unique<registry>();
    NewLevel.AssetStore = std::make_unique<asset_store>();
    NewLevel.EventBus = std::make_unique<event_bus>();

    NewLevel.Registry->AddSystem<camera_system>();
    NewLevel.Registry->AddSystem<damage_system>();
    NewLevel.Registry->AddSystem<movement_system>();
    NewLevel.Registry->AddSystem<rendering_system>();
    NewLevel.Registry->AddSystem<rendering_text_system>();
    NewLevel.Registry->AddSystem<animation_system>();
    NewLevel.Registry->AddSystem<collision_system>();
    NewLevel.Registry->AddSystem<keyboard_control_system>();
    NewLevel.Registry->AddSystem<projectile_emit_system>();
    NewLevel.Registry->AddSystem<projectile_life_cycle_system>();

    NewLevel.AssetStore->AddTexture("tank_image", "../assets/images/tank-tiger-up.png");
    NewLevel.AssetStore->AddTexture("truck_image", "../assets/images/truck-ford-up.png");
    NewLevel.AssetStore->AddTexture("bullet_image", "../assets/images/bullet.png");
    NewLevel.AssetStore->AddTexture("chopper_image", "../assets/images/chopper-spritesheet.png");
    NewLevel.AssetStore->AddTexture("radar_image", "../assets/images/radar.png");
    NewLevel.AssetStore->AddTexture("tilemap_image", LevelTexture);

    NewLevel.AssetStore->AddFont("font_arial", "C:/Windows/Fonts/arial.ttf", 18);

#if 0
    LoadTileMap(&NewLevel, LevelFile, TileScale, &NewLevel.MapWidth, &NewLevel.MapHeight);
#else
    NewLevel.MapWidth  = 25 * 32 * TileScale;
    NewLevel.MapHeight = 20 * 32 * TileScale;
#endif

    entity Chopper = NewLevel.Registry->CreateEntity();
    Chopper.Registry->TagEntity(Chopper, "player");
    NewLevel.Registry->AddComponent<transformation_compontent>(Chopper, V3(100.0f, 100.0f, 4), V2(1.0f, 1.0f));
    NewLevel.Registry->AddComponent<animation_component>(Chopper, 2, 20);
    NewLevel.Registry->AddComponent<body_component>(Chopper);
    NewLevel.Registry->AddComponent<camera_component>(Chopper);
    NewLevel.Registry->AddComponent<sprite_component>(Chopper, "chopper_image", 32, 32);
    NewLevel.Registry->AddComponent<collision_box_component>(Chopper, V2(32, 32));
    NewLevel.Registry->AddComponent<keyboard_control_component>(Chopper, V2(0, -60), V2(60, 0), V2(-60, 0), V2(0, 60));
    NewLevel.Registry->AddComponent<projectile_emitter_component>(Chopper, V2(150, 150), 0, 10000, 0, true);
    NewLevel.Registry->AddComponent<health_component>(Chopper, 100);

    entity Radar = NewLevel.Registry->CreateEntity();
    NewLevel.Registry->AddComponent<transformation_compontent>(Radar, V3(ColorBuffer->Width - 74.0f, 10.0f, 10), V2(1.0f, 1.0f));
    NewLevel.Registry->AddComponent<body_component>(Radar);
    NewLevel.Registry->AddComponent<sprite_component>(Radar, "radar_image", 64, 64, true);
    NewLevel.Registry->AddComponent<animation_component>(Radar, 8, 5, true);

    entity Tank = NewLevel.Registry->CreateEntity();
    Tank.Registry->GroupEntity(Tank, "enemies");
    NewLevel.Registry->AddComponent<transformation_compontent>(Tank, V3(ColorBuffer->Width / 2, ColorBuffer->Height / 2, 3), V2(1.0f, 1.0f));
    NewLevel.Registry->AddComponent<body_component>(Tank);
    NewLevel.Registry->AddComponent<sprite_component>(Tank, "tank_image", 32, 32);
    NewLevel.Registry->AddComponent<collision_box_component>(Tank, V2(32, 32));
    NewLevel.Registry->AddComponent<projectile_emitter_component>(Tank, V2(0, -100), 2000, 5000, 5, false);
    NewLevel.Registry->AddComponent<health_component>(Tank, 5);

    return NewLevel;
}

internal void 
LoadAllLevels(game_world* World)
{
    std::vector<std::string> LevelFileNames;
    std::vector<std::string> LevelTexturesFileNames;
    LoadAllLevelsFileNames(&LevelFileNames, &LevelTexturesFileNames);
    World->LevelsCount = LevelFileNames.size();

    if(!World->Levels)
    {
        World->Levels = (game_level*)calloc(sizeof(game_level), World->LevelsCount);
    }

    for(u32 LevelNumber = 0;
        LevelNumber < World->LevelsCount;
        ++LevelNumber)
    {
        World->Levels[LevelNumber] = LoadLevel(LevelNumber, LevelFileNames[LevelNumber], LevelTexturesFileNames[LevelNumber], Camera.Scale);
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
process_input(game_world* World)
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
            World->Levels[World->ChosenLevel].EventBus->EmitEvent<key_pressed_event>(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            World->Levels[World->ChosenLevel].EventBus->EmitEvent<key_up_event>(event.key.keysym.sym);
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
    //if (DeltaTime > TimeForFrame) DeltaTime = TimeForFrame;
    PreviousFrameTime = SDL_GetTicks();

    World->Levels[World->ChosenLevel].EventBus->Reset();

    World->Levels[World->ChosenLevel].Registry->GetSystem<damage_system>().SubscribeToEvent(World->Levels[World->ChosenLevel].EventBus);
    World->Levels[World->ChosenLevel].Registry->GetSystem<keyboard_control_system>().SubscribeToEvent(World->Levels[World->ChosenLevel].EventBus);
    World->Levels[World->ChosenLevel].Registry->GetSystem<projectile_emit_system>().SubscribeToEvent(World->Levels[World->ChosenLevel].EventBus);

    World->Levels[World->ChosenLevel].Registry->GetSystem<movement_system>().Update(DeltaTime);
    World->Levels[World->ChosenLevel].Registry->Update();
    World->Levels[World->ChosenLevel].Registry->GetSystem<collision_system>().Update(World->Levels[World->ChosenLevel].EventBus);
    World->Levels[World->ChosenLevel].Registry->GetSystem<projectile_emit_system>().Update(World->Levels[World->ChosenLevel].Registry);
    World->Levels[World->ChosenLevel].Registry->GetSystem<camera_system>().Update(&Camera, World->Levels[World->ChosenLevel].MapWidth, World->Levels[World->ChosenLevel].MapHeight);
    World->Levels[World->ChosenLevel].Registry->GetSystem<projectile_life_cycle_system>().Update();
}

internal void 
render(game_world* World)
{
    RenderColorBuffer();
    ClearColorBuffer(ColorBuffer, 0xFF16161d);//0xFF056263);

    World->Levels[World->ChosenLevel].Registry->GetSystem<rendering_system>().Render(World->Levels[World->ChosenLevel].Registry, World->Levels[World->ChosenLevel].AssetStore, &Camera);
    World->Levels[World->ChosenLevel].Registry->GetSystem<rendering_text_system>().Render(World->Levels[World->ChosenLevel].AssetStore, &Camera);
    World->Levels[World->ChosenLevel].Registry->GetSystem<animation_system>().Animate();

    SDL_RenderPresent(renderer);
}

int 
main(int argc, char** argv)
{
    is_running = InitWindow();

    game_world* World = (game_world*)calloc(sizeof(game_world), 1);

    Camera.Scale = 2.0f; // Default value
    Camera.Area.Min = V2(0, 0);
    Camera.Area.Max = V2(ColorBuffer->Width, ColorBuffer->Height);
    Camera.P = V2(0, 0);

    setup(World);

    while(is_running)
    {
        process_input(World);
        update(World);
        render(World);
    }

    DestroyWindow();

    return 0;
}
