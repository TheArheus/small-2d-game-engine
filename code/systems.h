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

            Transform.Position += V3(Movement.Velocity * DeltaTime, 0.0f);
        }
    }
};

internal texture_t*
GetShiftedTexture(texture_t* Texture, v2 Shift, v2 NewDim)
{
    texture_t* NewTexture = (texture_t*)malloc(sizeof(texture_t));
    NewTexture->Width  = NewDim.x;
    NewTexture->Height = NewDim.y;
    NewTexture->Memory = (u32*)calloc(sizeof(u32), NewDim.x * NewDim.y);

    u32 ShiftX = Shift.x;
    u32 ShiftY = Shift.y;

    u32 Pitch    = Texture->Width * sizeof(u32);
    u32 NewPitch = NewTexture->Width * sizeof(u32);

    u8* InRow  = ((u8*)Texture->Memory + ShiftX*sizeof(u32) + ShiftY*Pitch);
    u8* OutRow = (u8*)NewTexture->Memory;
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
    health_component Health;
};

// TODO: Have to fix the bug with 
// the projectiles not being rendered
class rendering_system : public entity_system
{
public:
    rendering_system()
    {
        RequireComponent<transformation_compontent>();
        RequireComponent<sprite_component>();
        RequireComponent<health_component>();
    }

    void Render(std::unique_ptr<registry>& Registry, std::unique_ptr<asset_store>& AssetStore, camera* Camera)
    {
        std::vector<sortable_entities> SortedEntities;
        for(entity Entity: Entities)
        {
            sortable_entities SortableEntity;

            SortableEntity.Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
            SortableEntity.Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);
            SortableEntity.Health = Entity.Registry->GetComponent<health_component>(Entity);

            SortedEntities.emplace_back(SortableEntity);
        }

        std::sort(SortedEntities.begin(), SortedEntities.end(), [](const sortable_entities& A, const sortable_entities& B){ return A.Transform.Position.z < B.Transform.Position.z; });

        for(sortable_entities Entity : SortedEntities)
        {
            transformation_compontent Transform = Entity.Transform;
            sprite_component Sprite = Entity.Sprite;
            health_component Health = Entity.Health;

            texture_t* Texture = AssetStore->GetTexture(Sprite.AssetID);
            texture_t* SlicedTexture = GetShiftedTexture(Texture, Sprite.SpritePos, Sprite.Dims);

            v2 DrawPoint     = Transform.Position.xy - (!Sprite.IsFixed)*Camera->P;
            v2 TextureWidth  = rotate(Transform.Scale*Sprite.Dims.x*V2(1, 0), Transform.Rotation);
            v2 TextureHeight = rotate(Transform.Scale*Sprite.Dims.y*V2(0, 1), Transform.Rotation);

            //if(IsInRectangle(Camera->Area, DrawPoint))
            {
                DrawRotRect(ColorBuffer, DrawPoint, TextureWidth, TextureHeight, 0xFFFFFFFF, SlicedTexture);
            }

            v4 Color = Lerp(V4(255, 0, 0, 255), Health.HealthPercentage / 100.0f, V4(0, 255, 0, 255));
            PutText(DrawPoint + V2(TextureWidth.x/2, TextureHeight.y/2 - 25), std::to_string(Health.HealthPercentage) + "%", &AssetStore->GetFont("font_arial"), Color);
        
            free(SlicedTexture);
            free(SlicedTexture->Memory);
        }

        SortedEntities.clear();
    }
};

class rendering_text_system : public entity_system
{
public:
    rendering_text_system()
    {
        RequireComponent<text_component>();
    }

    void Render(std::unique_ptr<asset_store>& AssetStore, camera* Camera)
    {
        for(entity Entity : Entities)
        {
            text_component Text = Entity.Registry->GetComponent<text_component>(Entity);
            font_t Font = AssetStore->GetFont(Text.AssetID);

            v2 DrawPoint = Text.Position - (!Text.IsFixed)*Camera->P;
            PutText(DrawPoint, Text.Text, &Font, Text.Color);
        }
    }
};

class animation_system : public entity_system
{
public:
    animation_system()
    {
        RequireComponent<animation_component>();
        RequireComponent<sprite_component>();
    }

    void Animate()
    {
        for(entity Entity : Entities)
        {
            animation_component& Animation = Entity.Registry->GetComponent<animation_component>(Entity);
            sprite_component& Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);

            Animation.CurrentFrame = ((SDL_GetTicks() - Animation.StartTime) * Animation.FrameRateSpeed / 1000) % Animation.FramesCount;

            Sprite.SpritePos.x = Animation.CurrentFrame * Sprite.Dims.x;
        }
    }
};

class collision_system : public entity_system
{
public:
    collision_system()
    {
        RequireComponent<collision_box_component>();
        RequireComponent<transformation_compontent>();
    }

    void Update(std::unique_ptr<event_bus>& EventBus)
    {
        for(entity A : Entities)
        {
            for(entity B : Entities)
            {
                if(A.EntityID != B.EntityID)
                {
                    collision_box_component CollisionA = A.Registry->GetComponent<collision_box_component>(A);
                    transformation_compontent TransformA = A.Registry->GetComponent<transformation_compontent>(A);

                    collision_box_component CollisionB = B.Registry->GetComponent<collision_box_component>(B);
                    transformation_compontent TransformB = B.Registry->GetComponent<transformation_compontent>(B);

                    v2 PosA = TransformA.Position.xy + CollisionA.Offset;
                    v2 PosB = TransformB.Position.xy + CollisionB.Offset;

                    b32 AreCollided = (((PosA.x) < (PosB.x + CollisionB.Dims.x)) && 
                                       ((PosA.y) < (PosB.y + CollisionB.Dims.y)) &&
                                       ((PosA.x + CollisionA.Dims.x) > (PosB.x)) &&
                                       ((PosA.y + CollisionA.Dims.y) > (PosB.y)));

                    if(AreCollided)
                    {
                        MessageLog("Entities " + std::to_string(A.EntityID) + " and " + std::to_string(B.EntityID) + " are collided");

                        EventBus->EmitEvent<collision_event>(A, B);
                    }
                }
            }
        }
    }
};

class damage_system : public entity_system
{
public:
    damage_system()
    {
        RequireComponent<collision_box_component>();
    }

    void SubscribeToEvent(std::unique_ptr<event_bus>& EventBus)
    {
        EventBus->SubscribeToEvent<collision_event>(this, &damage_system::OnCollision);
    }

    void OnCollision(collision_event& Event)
    {
        entity A = Event.A;
        entity B = Event.B;

        if(A.Registry->EntityBelongsToGroup(A, "projectiles") && B.Registry->EntityHasTag(B, "player"))
        {
            OnProjectileHitsPlayer(A, B);
        }

        if(B.Registry->EntityBelongsToGroup(B, "projectiles") && A.Registry->EntityHasTag(B, "player"))
        {
            OnProjectileHitsPlayer(B, A);
        }

        if(A.Registry->EntityBelongsToGroup(A, "projectiles") && B.Registry->EntityBelongsToGroup(B, "enemies"))
        {
            OnProjectileHitsEnemy(A, B);
        }

        if(B.Registry->EntityBelongsToGroup(B, "projectiles") && A.Registry->EntityBelongsToGroup(A, "enemies"))
        {
            OnProjectileHitsEnemy(B, A);
        }
    }

    void OnProjectileHitsPlayer(entity Projectile, entity Player)
    {
        const projectile_component ProjectileComp = Projectile.Registry->GetComponent<projectile_component>(Projectile);

        if(!ProjectileComp.IsFriendly)
        {
            health_component& Health = Player.Registry->GetComponent<health_component>(Player);

            Health.HealthPercentage -= ProjectileComp.HitDamage;

            if(Health.HealthPercentage <= 0)
            {
                Player.Registry->KillEntity(Player);
            }

            Projectile.Registry->KillEntity(Projectile);
        }
    }

    void OnProjectileHitsEnemy(entity Projectile, entity Enemy)
    {
        const projectile_component ProjectileComp = Projectile.Registry->GetComponent<projectile_component>(Projectile);

        if(ProjectileComp.IsFriendly)
        {
            health_component& Health = Enemy.Registry->GetComponent<health_component>(Enemy);

            Health.HealthPercentage -= ProjectileComp.HitDamage;

            if(Health.HealthPercentage <= 0)
            {
                Enemy.Registry->KillEntity(Enemy);
            }

            Projectile.Registry->KillEntity(Projectile);
        }
    }
};

class keyboard_control_system : public entity_system
{
public:
    keyboard_control_system()
    {
        RequireComponent<keyboard_control_component>();
        RequireComponent<sprite_component>();
        RequireComponent<body_component>();
    }

    void SubscribeToEvent(std::unique_ptr<event_bus>& EventBus)
    {
        EventBus->SubscribeToEvent<key_pressed_event>(this, &keyboard_control_system::OnKeyPressed);
        EventBus->SubscribeToEvent<key_up_event>(this, &keyboard_control_system::OnKeyReleased);
    }

    void OnKeyPressed(key_pressed_event& Event)
    {
        for(entity& Entity : Entities)
        {
            keyboard_control_component Control = Entity.Registry->GetComponent<keyboard_control_component>(Entity);
            sprite_component& Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);
            body_component& Body = Entity.Registry->GetComponent<body_component>(Entity);

            switch(Event.Symbol)
            {
                case SDLK_UP:
                case SDLK_w:
                {
                    Body.Velocity = Control.dP_Up;
                    Sprite.SpritePos.y = Sprite.Dims.y * 0;
                } break;

                case SDLK_RIGHT:
                case SDLK_d:
                {
                    Body.Velocity = Control.dP_Right;
                    Sprite.SpritePos.y = Sprite.Dims.y * 1;
                } break;

                case SDLK_DOWN:
                case SDLK_s:
                {
                    Body.Velocity = Control.dP_Down;
                    Sprite.SpritePos.y = Sprite.Dims.y * 2;
                } break;

                case SDLK_LEFT:
                case SDLK_a:
                {
                    Body.Velocity = Control.dP_Left;
                    Sprite.SpritePos.y = Sprite.Dims.y * 3;
                } break;
            }
        }
    }


    void OnKeyReleased(key_up_event& Event)
    {
        for(entity& Entity : Entities)
        {
            body_component& Body = Entity.Registry->GetComponent<body_component>(Entity);

            Body.Velocity = V2(0, 0);
        }
    }
};


class camera_system : public entity_system
{
public:
    camera_system()
    {
        RequireComponent<camera_component>();
        RequireComponent<transformation_compontent>();
    }

    void Update(camera* Camera, i32 MapWidth, i32 MapHeight)
    {
        for(entity Entity : Entities)
        {
            transformation_compontent Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);

            if(Transform.Position.x + GetDim(Camera->Area).x / 2 < MapWidth)
            {
                Camera->P.x = Transform.Position.x - ColorBuffer->Width / 2;
            }

            if(Transform.Position.y + GetDim(Camera->Area).y / 2 < MapHeight)
            {
                Camera->P.y = Transform.Position.y - ColorBuffer->Height / 2;
            }

            Camera->P.x = Camera->P.x < 0 ? 0 : Camera->P.x;
            Camera->P.y = Camera->P.y < 0 ? 0 : Camera->P.y;
            Camera->P.x = (Camera->P.x + GetDim(Camera->Area).x > MapWidth)  ? MapWidth  - GetDim(Camera->Area).x : Camera->P.x;
            Camera->P.y = (Camera->P.y + GetDim(Camera->Area).y > MapHeight) ? MapHeight - GetDim(Camera->Area).y : Camera->P.y;
        }
    }
};

void ShootProjectile()
{
}

class projectile_emit_system : public entity_system
{
public:
    projectile_emit_system()
    {
        RequireComponent<projectile_emitter_component>();
        RequireComponent<transformation_compontent>();
    }

    void SubscribeToEvent(std::unique_ptr<event_bus>& EventBus)
    {
        EventBus->SubscribeToEvent<key_pressed_event>(this, &projectile_emit_system::OnKeyPressed);
        EventBus->SubscribeToEvent<key_up_event>(this, &projectile_emit_system::OnKeyReleased);
    }

    void OnKeyPressed(key_pressed_event& Event)
    {
        if(Event.Symbol == SDLK_SPACE)
        {
            for(entity Entity : Entities)
            {
                if(Entity.Registry->EntityHasTag(Entity, "player"))
                {
                    const projectile_emitter_component ProjectileEmitter = Entity.Registry->GetComponent<projectile_emitter_component>(Entity);
                    const transformation_compontent Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);
                    const body_component Movement = Entity.Registry->GetComponent<body_component>(Entity);

                    v2 ProjectilePos = Transform.Position.xy;
                    if(Entity.Registry->HasComponent<sprite_component>(Entity))
                    {
                        sprite_component Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);
                        ProjectilePos += (Transform.Scale * Sprite.Dims / 2);
                    }

                    v2 ProjectileVelocity = ProjectileEmitter.Velocity;
                    v2 Direction = {};
                    if(Movement.Velocity.x > 0) Direction.x =  1;
                    if(Movement.Velocity.x < 0) Direction.x = -1;
                    if(Movement.Velocity.y > 0) Direction.y =  1;
                    if(Movement.Velocity.y < 0) Direction.y = -1;
                    ProjectileVelocity = ProjectileEmitter.Velocity * Direction;

                    entity NewProjectileEntity = Entity.Registry->CreateEntity();
                    NewProjectileEntity.Registry->GroupEntity(NewProjectileEntity, "projectiles");
                    NewProjectileEntity.Registry->AddComponent<transformation_compontent>(NewProjectileEntity, V3(ProjectilePos, 4.0f), V2(1, 1), 0);
                    NewProjectileEntity.Registry->AddComponent<body_component>(NewProjectileEntity, ProjectileVelocity);
                    NewProjectileEntity.Registry->AddComponent<collision_box_component>(NewProjectileEntity, V2(4, 4));
                    NewProjectileEntity.Registry->AddComponent<sprite_component>(NewProjectileEntity, "bullet_image", 4, 4);
                    NewProjectileEntity.Registry->AddComponent<projectile_component>(NewProjectileEntity, ProjectileEmitter.IsFriendly, ProjectileEmitter.HitDamage, ProjectileEmitter.ProjectileDuration);
                }
            }
        }
    }
    
    void OnKeyReleased(key_up_event& Event)
    {
        // TODO: Have to make this to stop creating new entities
        // while pushing "space" key, as OnKeyPressed function
        // just creates bunch of the new projectiles
        // instead of creating one entity once space key pressed
    }

    void Update(std::unique_ptr<registry>& Registry)
    {
        for(entity& Entity : Entities)
        {
            projectile_emitter_component& ProjectileEmitter = Entity.Registry->GetComponent<projectile_emitter_component>(Entity);
            transformation_compontent Transform = Entity.Registry->GetComponent<transformation_compontent>(Entity);

            if(ProjectileEmitter.RepeatFrequency == 0) continue;

            if((SDL_GetTicks() - ProjectileEmitter.LastEmissionTime) > ProjectileEmitter.RepeatFrequency)
            {
                v2 ProjectilePos = Transform.Position.xy;
                if(Entity.Registry->HasComponent<sprite_component>(Entity))
                {
                    sprite_component Sprite = Entity.Registry->GetComponent<sprite_component>(Entity);
                    ProjectilePos += (Transform.Scale * Sprite.Dims / 2);
                }

                entity NewProjectileEntity = Entity.Registry->CreateEntity();
                NewProjectileEntity.Registry->GroupEntity(NewProjectileEntity, "projectiles");
                Entity.Registry->AddComponent<transformation_compontent>(NewProjectileEntity, V3(ProjectilePos, 4.0f), V2(1, 1), 0);
                Entity.Registry->AddComponent<body_component>(NewProjectileEntity, ProjectileEmitter.Velocity);
                Entity.Registry->AddComponent<collision_box_component>(NewProjectileEntity, V2(4, 4));
                Entity.Registry->AddComponent<sprite_component>(NewProjectileEntity, "bullet_image", 4, 4);
                Entity.Registry->AddComponent<projectile_component>(NewProjectileEntity, ProjectileEmitter.IsFriendly, ProjectileEmitter.HitDamage, ProjectileEmitter.ProjectileDuration);

                ProjectileEmitter.LastEmissionTime = SDL_GetTicks();
            }
        }
    }
};

class projectile_life_cycle_system : public entity_system
{
public:
    projectile_life_cycle_system()
    {
        RequireComponent<projectile_component>();
    }

    void Update()
    {
        for(entity& Entity : Entities)
        {
            projectile_component& Projectile = Entity.Registry->GetComponent<projectile_component>(Entity);

            if((SDL_GetTicks() - Projectile.StartTime) > Projectile.Duration)
            {
                Entity.Registry->KillEntity(Entity);
            }
        }
    }
};

#define SYSTEMS_H_
#endif
