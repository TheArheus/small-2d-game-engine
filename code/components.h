
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

struct sprite_component
{
    std::string AssetID;
    v2 Dims;
    v2 SpritePos;

    sprite_component(std::string AssetID_ = "", r32 Width = 0.0f, r32 Height = 0.0f, r32 SpriteX = 0.0f, r32 SpriteY = 0.0f)
    {
        AssetID = AssetID_;

        Dims.x = Width;
        Dims.y = Height;

        SpritePos.x = SpriteX;
        SpritePos.y = SpriteY;
    }
};

#define COMPONENTS_H
#endif
