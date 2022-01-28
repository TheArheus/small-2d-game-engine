#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include <string.h>
#include "debug.h"
#include "entity_system.cpp"

bool is_running;

i32 PreviousFrameTime = 0;
r32 DeltaTime = 0;
r32 TimeForFrame = 0;
r32 dtForFrame = 0;

static void 
setup(std::unique_ptr<registry>& Registry)
{
    Registry->AddSystem<movement_system>();
    Registry->AddSystem<rendering_system>();

    ColorBuffer->Memory = (u32*)malloc(sizeof(u32)*ColorBuffer->Width*ColorBuffer->Height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                ColorBuffer->Width, ColorBuffer->Height);

    entity Tank = Registry->CreateEntity();

    Registry->AddComponent<transformation_compontent>(Tank, V2(10.0f, 10.0f), V2(1.0f, 1.0f), 0.0f);
    Registry->AddComponent<body_component>(Tank);
    Registry->AddComponent<sprite_component>(Tank, 10, 10);

}

static void 
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

static void 
update(std::unique_ptr<registry>& Registry)
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

    Registry->Update();
    Registry->GetSystem<movement_system>().Update(DeltaTime);
}

static void 
render(std::unique_ptr<registry>& Registry)
{
    RenderColorBuffer();
    ClearColorBuffer(ColorBuffer, 0xFF056263);

    Registry->GetSystem<rendering_system>().Render();

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv)
{
    is_running = InitWindow();
    std::unique_ptr<registry> Registry = std::make_unique<registry>();

    setup(Registry);

    while(is_running)
    {
        process_input();
        update(Registry);
        render(Registry);
    }

    DestroyWindow();

    return 0;
}
