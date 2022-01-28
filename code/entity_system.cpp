
u32 component_::NextID = 0;

template<typename tcomponent> 
void entity_system::
RequireComponent()
{
    ComponentSignature.set(component<tcomponent>::GetID());
}

void entity_system::
AddEntity(entity Entity)
{
    Entities.push_back(Entity);
}

void entity_system::
RemoveEntity(entity Entity)
{
    Entities.erase(std::remove_if(Entities.begin(), Entities.end(), [&Entity](entity OtherEntity){ return OtherEntity.EntityID == Entity.EntityID; }), Entities.end());
}

const signature& entity_system::
GetComponentSignature() const
{
    return ComponentSignature; 
}

entity registry::
CreateEntity()
{
    entity NewEntity;

    NewEntity.EntityID = EntityCount++;
    NewEntity.Registry = this;
    HotEntities.insert(NewEntity);

    if(NewEntity.EntityID >= EntityComponentSignature.size())
    {
        EntityComponentSignature.resize(NewEntity.EntityID + 1);
    }

    MessageLog("Entity created with ID = " + std::to_string(NewEntity.EntityID));

    return NewEntity;
}

void registry::
AddEntityToSystems(entity Entity)
{
    const i32 EntityID = Entity.EntityID;
    const signature& EntitySignature = EntityComponentSignature[EntityID];

    for(auto& System : Systems)
    {
        const signature& SystemSignature = System.second->GetComponentSignature();

        bool IsEntityNeeded = (EntitySignature & SystemSignature) == SystemSignature;
        if(IsEntityNeeded)
        {
            System.second->AddEntity(Entity);
        }
    }
}

void registry::
Update()
{
    for(entity Entity : HotEntities)
    {
        AddEntityToSystems(Entity);
    }

    HotEntities.clear();
}

template<typename tcomponent, typename ...targs> 
void registry::
AddComponent(entity Entity, targs&& ...Args)
{
    const i32 ComponentID = component<tcomponent>::GetID();
    const i32 EntityID = Entity.EntityID;

    if(ComponentID >= ComponentsPool.size())
    {
        ComponentsPool.resize(ComponentID + 1, nullptr);
    }

    if(!ComponentsPool[ComponentID])
    {
        std::shared_ptr<pool<tcomponent>> NewComponentPool(new pool<tcomponent>());
        ComponentsPool[ComponentID] = NewComponentPool;
    }

    std::shared_ptr<pool<tcomponent>> ExistComponentPool = std::static_pointer_cast<pool<tcomponent>>(ComponentsPool[ComponentID]);
    if(EntityID >= ExistComponentPool->Data.size())
    {
        ExistComponentPool->Data.resize(EntityCount);
    }

    tcomponent NewComponent(std::forward<targs>(Args)...);

    ExistComponentPool->Data[EntityID] = NewComponent;
    EntityComponentSignature[EntityID].set(ComponentID);
}

template<typename tcomponent>
void registry::
RemoveComponent(entity Entity)
{
    const i32 ComponentID = component<tcomponent>::GetID();
    const i32 EntityID = Entity.EntityID;

    EntityComponentSignature[EntityID].set(ComponentID, false);
}

template<typename tcomponent>
b32 registry::
HasComponent(entity Entity)
{
    const i32 ComponentID = component<tcomponent>::GetID();
    const i32 EntityID = Entity.EntityID;

    b32 Result = EntityComponentSignature[EntityID].test(ComponentID);
    return Result;
}

template<typename tcomponent> 
tcomponent& registry::
GetComponent(entity Entity)
{
    const i32 EntityID = Entity.EntityID;
    const i32 ComponentID = component<tcomponent>::GetID();

    std::shared_ptr<pool<tcomponent>> ComponentPool = std::static_pointer_cast<pool<tcomponent>>(ComponentsPool[ComponentID]);
    tcomponent Result = ComponentPool->Data[EntityID];

    return Result;
}

template<typename tsystem, typename ...targs> 
void registry::
AddSystem(targs&& ...Args)
{
    std::shared_ptr<tsystem> NewSystem = std::make_shared<tsystem>(std::forward<targs>(Args)...);
    Systems.insert(std::make_pair(std::type_index(typeid(tsystem)), NewSystem));
}

template<typename tsystem> 
void registry::
RemoveSystem()
{
    auto System = Systems.find(std::type_index(typeid(tsystem)));
    Systems.erase(System);
}

template<typename tsystem> 
b32 registry::
HasSystem() const
{
    b32 Reuslt = Systems.find(std::type_index(typeid(tsystem))) != Systems.end();
    return Result;
}

template<typename tsystem> 
tsystem& registry::
GetSystem() const
{
    auto System = Systems.find(std::type_index(typeid(tsystem)));
    return *(std::static_pointer_cast<tsystem>(System->second));
}

