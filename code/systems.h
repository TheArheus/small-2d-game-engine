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
            transformation_compontent Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
            body_component Movement = Entity.Registry->GetComponent<body_component>(Entity);

            Transform.Position += V3(Movement.Velocity * DeltaTime, 0.0f);
        }
    }
};


// TODO: need to fix pointer arithmetics here
// (but I think pointer arithmetics) 
// and maybe correct mathematics
internal texture_t
GetShiftedTexture(texture_t* Texture, v2 Shift, v2 NewDim)
{
    texture_t NewTexture;
    NewTexture.Width  = NewDim.x;
    NewTexture.Height = NewDim.y;
    NewTexture.Memory = (u32*)calloc(sizeof(u32), NewDim.x * NewDim.y);

    u32 ShiftX = Shift.x;
    u32 ShiftY = Shift.y;

    u32 Pitch    = Texture->Width * sizeof(u32);
    u32 NewPitch = NewTexture.Width * sizeof(u32);

    u8* InRow  = ((u8*)Texture->Memory + ShiftX*sizeof(u32) + ShiftY*Pitch);
    u8* OutRow = (u8*)NewTexture.Memory;
    for(u32 Y = 0;
        Y < NewDim.y;
        ++Y)
    {
        u32* InPixel  = (u32*)InRow;
        u32* OutPixel = (u32*)OutRow;
        
        for(u32 X = 0;
            X < NewDim.x;
            ++X)
        {
            *OutPixel++ = *InPixel++;
        }

        InRow  += Pitch;
        OutRow += NewPitch;
        
    }

    return NewTexture;
}

struct sortable_entities
{
    transformation_compontent Transform;
    sprite_component Sprite;
};

class rendering_system : public entity_system
{
public:
    rendering_system()
    {
        RequireComponent<transformation_compontent>();
        RequireComponent<sprite_component>();
    }

    void Render(std::unique_ptr<asset_store>& AssetStore)
    {
        std::vector<sortable_entities> SortedEntities;
        for(entity& Entity: Entities)
        {
            sortable_entities SortableEntity;

            SortableEntity.Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
            SortableEntity.Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);

            SortedEntities.emplace_back(SortableEntity);
        }

        std::sort(SortedEntities.begin(), SortedEntities.end(), [](const sortable_entities& A, const sortable_entities& B){ return A.Transform.Position.z < B.Transform.Position.z; });

        for(sortable_entities Entity : SortedEntities)
        {
            transformation_compontent Transform = Entity.Transform;
            sprite_component Sprite = Entity.Sprite;
            texture_t Texture = *AssetStore->GetTexture(Sprite.AssetID);
            texture_t SlicedTexture;

            SlicedTexture = GetShiftedTexture(&Texture, Sprite.SpritePos, Sprite.Dims);

            v2 DrawPoint     = Transform.Position.xy;
            v2 TextureWidth  = rotate(Transform.Scale*Sprite.Dims.x*V2(1, 0), Transform.Rotation);
            v2 TextureHeight = rotate(Transform.Scale*Sprite.Dims.y*V2(0, 1), Transform.Rotation);
            DrawRotRect(ColorBuffer, DrawPoint, TextureWidth, TextureHeight, 0xFFFFFFFF, &SlicedTexture);
        }
    }
};


#define SYSTEMS_H_
#endif
