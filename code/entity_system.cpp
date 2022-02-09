
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
    i32 NewEntityID;

    if(FreedIDs.empty())
    {
        NewEntityID = EntityCount++;
        if(NewEntityID >= EntityComponentSignature.size())
        {
            EntityComponentSignature.resize(NewEntityID + 1);
        }
    }
    else
    {
        NewEntityID = FreedIDs.front();
        FreedIDs.pop_front();
    }

    NewEntity.EntityID = NewEntityID;
    NewEntity.Registry = this;
    HotEntities.insert(NewEntity);

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
RemoveEntityFromSystems(entity Entity)
{
    for(auto& System : Systems)
    {
        System.second->RemoveEntity(Entity);
    }
}

void registry::
KillEntity(entity Entity)
{
    ColdEntities.insert(Entity);
    MessageLog("Entity #" + std::to_string(Entity.EntityID) + " was killed");
}

void registry::
Update()
{
    for(entity Entity : HotEntities)
    {
        AddEntityToSystems(Entity);
    }

    HotEntities.clear();

    for(entity Entity : ColdEntities)
    {
        RemoveEntityFromSystems(Entity);
        EntityComponentSignature[Entity.EntityID].reset();

        for(auto Pool : ComponentsPool)
        {
            if(Pool) Pool->RemoveEntityFromPool(Entity.EntityID);
        }

        FreedIDs.push_back(Entity.EntityID);

        RemoveEntityTag(Entity);
        RemoveEntityGroup(Entity);
    }

    ColdEntities.clear();
}

void registry::
TagEntity(entity Entity, const std::string& Tag)
{
    EntityPerTag.emplace(Tag, Entity);
    TagPerEntity.emplace(Entity.EntityID, Tag);
}

b32 registry:: 
EntityHasTag(entity Entity, const std::string& Tag) const
{
    b32 Result = (TagPerEntity.find(Entity.EntityID) != TagPerEntity.end()) && 
                 (EntityPerTag.find(Tag)->second == Entity);

    return Result;
}

entity registry:: 
GetEntityByTag(const std::string& Tag) const
{
    return EntityPerTag.at(Tag);
}

void registry:: 
RemoveEntityTag(entity Entity)
{
    auto TaggedEntity = TagPerEntity.find(Entity.EntityID);
    if(TaggedEntity != TagPerEntity.end())
    {
        auto Tag = TaggedEntity->second;
        EntityPerTag.erase(Tag);
        TagPerEntity.erase(TaggedEntity);
    }
}

void registry:: 
GroupEntity(entity Entity, const std::string& Group)
{
    EntitiesPerGroup.emplace(Group, std::set<entity>());
    EntitiesPerGroup[Group].emplace(Entity);
    GroupPerEntity.emplace(Entity.EntityID, Group);
}

b32 registry:: 
EntityBelongsToGroup(entity Entity, const std::string& Group) const
{
    if(EntitiesPerGroup.find(Group) == EntitiesPerGroup.end()) return false;

    std::set<entity> GroupEntities = EntitiesPerGroup.at(Group);
    return GroupEntities.find(Entity) != GroupEntities.end();
}

std::vector<entity> registry:: 
GetEntitiesByGroup(const std::string& Group) const
{
    auto& SetOfEntities = EntitiesPerGroup.at(Group);
    return std::vector<entity>(SetOfEntities.begin(), SetOfEntities.end());
}

void registry:: 
RemoveEntityGroup(entity Entity)
{
    auto GroupedEntity = GroupPerEntity.find(Entity.EntityID);
    if(GroupedEntity != GroupPerEntity.end())
    {
        auto Group = EntitiesPerGroup.find(GroupedEntity->second);
        if(Group != EntitiesPerGroup.end())
        {
            auto EntityInGroup = Group->second.find(Entity);
            if(EntityInGroup != Group->second.end())
            {
                Group->second.erase(EntityInGroup);
            }
        }

        GroupPerEntity.erase(GroupedEntity);
    }
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

    tcomponent NewComponent(std::forward<targs>(Args)...);

    ExistComponentPool->Set(NewComponent, EntityID);
    EntityComponentSignature[EntityID].set(ComponentID);

    //MessageLog("Component ID = " + std::to_string(ComponentID) + " --> POOL SIZE: " + std::to_string(ExistComponentPool->Size));
}

template<typename tcomponent>
void registry::
RemoveComponent(entity Entity)
{
    const i32 ComponentID = component<tcomponent>::GetID();
    const i32 EntityID = Entity.EntityID;

    std::shared_ptr<pool<tcomponent>> ExistComponentPool = std::static_pointer_cast<pool<tcomponent>>(ComponentsPool[ComponentID]);

    ExistComponentPool->Remove(EntityID);
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

    return ComponentPool->Get(EntityID);
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

