
#ifndef COMPONENTS_H

struct transformation_compontent
{
    v2 Position;
    v2 Scale;
    r32 Rotation;

    transformation_compontent(v2 Position_ = V2(0, 0), v2 Scale_ = V2(0, 0), r32 Rotation_ = 0)
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
    v2 Dims;

    sprite_component(r32 Width = 0.0f, r32 Height = 0.0f)
    {
        Dims.x = Width;
        Dims.y = Height;
    }
};

#define COMPONENTS_H
#endif
