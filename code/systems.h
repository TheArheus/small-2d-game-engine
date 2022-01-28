#ifndef SYSTEMS_H_

class movement_system : public entity_system
{
public:
    movement_system()
    {
        RequireComponent<transformation_compontent>();
        RequireComponent<body_component>();
    }

    void Update(r32 DeltaTime)
    {
        for(entity& Entity : Entities)
        {
            transformation_compontent& Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
            body_component& Movement = Entity.Registry->GetComponent<body_component>(Entity);

            Transform.Position += Movement.Velocity * DeltaTime;
        }
    }
};


class rendering_system : public entity_system
{
public:
    rendering_system()
    {
        RequireComponent<transformation_compontent>();
        RequireComponent<sprite_component>();
    }

    void Render()
    {
        for(entity& Entity : Entities)
        {
            transformation_compontent& Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
            sprite_component& Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);

            v2 StartP = Transform.Position;
            DrawRect(ColorBuffer, StartP, StartP + Sprite.Dims, 0xFFFFFFFF);
        }
    }
};


#define SYSTEMS_H_
#endif
