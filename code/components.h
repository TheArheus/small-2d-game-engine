
#ifndef COMPONENTS_H

struct transformation_compontent
{
    v3 Position;
    v2 Scale;
    r32 Rotation;

    transformation_compontent(v3 Position_ = V3(0, 0, 0), v2 Scale_ = V2(0, 0), r32 Rotation_ = 0)
    {
        Position = Position_;
        Scale = Scale_;
        Rotation = Rotation_;
    }
};

struct body_component
{
    v2 Velocity;

    body_component(v2 Velocity_ = V2(0, 0))
    {
        Velocity = Velocity_;
    }
};

struct text_component
{
    v2 Position;
    std::string Text;
    std::string AssetID;
    v4 Color;
    b32 IsFixed;

    text_component(std::string AssetID_ = "", std::string Text_ = "", v2 Position_ = V2(0, 0), v4 Color_ = V4(0), b32 IsFixed_ = false)
    {
        Position = Position_;
        Text = Text_;
        AssetID = AssetID_;
        Color = Color_;
        IsFixed = IsFixed_;
    }
};

struct health_component
{
    i32 HealthPercentage;
    health_component(i32 HealthPercentage_ = 100)
    {
        HealthPercentage = HealthPercentage_;
    }
};

struct projectile_emitter_component
{
    v2  Velocity;
    i32 RepeatFrequency;
    i32 ProjectileDuration;
    i32 LastEmissionTime;
    i32 HitDamage;
    b32 IsFriendly;

    projectile_emitter_component(v2 Velocity_ = V2(0, 0), i32 RepeatFrequency_ = 0, i32 ProjectileDuration_ = 10000, i32 HitDamage_ = 10, i32 IsFriendly_ = false)
    {
        Velocity = Velocity_;
        RepeatFrequency = RepeatFrequency_;
        ProjectileDuration = ProjectileDuration_;
        HitDamage = HitDamage_;
        IsFriendly = IsFriendly_;
        LastEmissionTime = SDL_GetTicks();
    }
};

struct projectile_component
{
    b32 IsFriendly;
    i32 HitDamage;
    i32 Duration;
    i32 StartTime;

    projectile_component(b32 IsFriendly_ = false, i32 HitDamage_ = 0, i32 Duration_ = 0)
    {
        IsFriendly = IsFriendly_;
        HitDamage = HitDamage_;
        Duration = Duration_;
        StartTime = SDL_GetTicks();
    }
};

struct sprite_component
{
    std::string AssetID;
    v2 Dims;
    v2 SpritePos;
    b32 IsFixed;

    sprite_component(std::string AssetID_ = "", r32 Width = 0.0f, r32 Height = 0.0f, b32 IsFixed_ = false, r32 SpriteX = 0.0f, r32 SpriteY = 0.0f)
    {
        AssetID = AssetID_;

        Dims.x = Width;
        Dims.y = Height;

        SpritePos.x = SpriteX;
        SpritePos.y = SpriteY;

        IsFixed = IsFixed_;
    }
};

struct animation_component
{
    i32 FramesCount;
    i32 CurrentFrame;
    i32 FrameRateSpeed;
    i32 StartTime;
    b32 IsLoop;

    animation_component(i32 FramesCount_ = 1, i32 FrameRateSpeed_ = 1, b32 IsLoop_ = true)
    {
        FramesCount = FramesCount_;
        CurrentFrame = 1;
        FrameRateSpeed = FrameRateSpeed_;
        IsLoop = IsLoop_;
        StartTime = SDL_GetTicks();
    }
};

struct collision_box_component
{
    v2 Dims;
    v2 Offset;

    collision_box_component(v2 Dims_ = V2(0, 0), v2 Offset_ = V2(0, 0))
    {
        Dims = Dims_;
        Offset = Offset_;
    }
};

struct keyboard_control_component
{
    v2 dP_Up;
    v2 dP_Right;
    v2 dP_Left;
    v2 dP_Down;

    keyboard_control_component(v2 Up = V2(0, 0), v2 Right = V2(0, 0), v2 Left = V2(0, 0), v2 Down = V2(0, 0))
    {
        dP_Up = Up;
        dP_Right = Right;
        dP_Left = Left;
        dP_Down = Down;
    }
};

struct camera_component
{
    camera_component() = default;
};

#define COMPONENTS_H
#endif
