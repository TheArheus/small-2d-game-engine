
#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

const u32 MaxComponents = 32;
typedef std::bitset<MaxComponents> signature;

class registry;
struct entity
{
    i32 EntityID;
    registry* Registry;

    bool operator==(const entity& B) const { return EntityID == B.EntityID; }
    bool operator!=(const entity& B) const { return EntityID != B.EntityID; }
    bool operator<(const entity& B)  const { return EntityID < B.EntityID; }
    bool operator>(const entity& B)  const { return EntityID > B.EntityID; }
};

struct component_
{
protected:
    static u32 NextID;
};

template<typename T>
struct component : public component_
{
    static u32 GetID()
    { 
        static u32 Result = NextID++; 
        return Result;
    }
};

struct entity_system 
{
private:
    signature ComponentSignature;

public:
    entity_system()  = default;
    ~entity_system() = default;

    std::vector<entity> Entities;

    const signature& GetComponentSignature() const;

    void AddEntity(entity Entity);
    void RemoveEntity(entity Entity);

    template<typename T> void RequireComponent();
};

struct pool_
{
    virtual ~pool_() = default;
    virtual void RemoveEntityFromPool(i32 EntityID) = 0;
};

template<typename tobject>
struct pool : pool_
{
    i32 Size;
    std::vector<tobject> Data;

    std::unordered_map<i32, i32> EntityIdToIndex;
    std::unordered_map<i32, i32> IndexToEntityId;

    pool(i32 Capacity = 100)
    {
        Size = 0;
        Data.resize(Capacity);
    }

    ~pool() = default;

    void Clear()
    {
        Data.clear();
        EntityIdToIndex.clear();
        IndexToEntityId.clear();
        Size = 0;
    }

    void Set(tobject Object, i32 EntityID)
    {
        if(EntityIdToIndex.find(EntityID) != EntityIdToIndex.end())
        {
            i32 Index = EntityIdToIndex[EntityID];
            Data[Index] = Object;
        }
        else
        {
            i32 Index = Size;
            EntityIdToIndex.emplace(EntityID, Index);
            IndexToEntityId.emplace(Index, EntityID);

            if(Index >= Data.capacity())
            {
                Data.resize(Size * 2);
            }

            Data[Index] = Object;
            Size++;
        }
    }

    void Remove(i32 EntityID)
    {
        i32 IndexOfRemoved = EntityIdToIndex[EntityID];
        i32 IndexOfLast = Size - 1;
        Data[IndexOfRemoved] = Data[IndexOfLast];

        i32 EntityIdOfLastElement = IndexToEntityId[IndexOfLast];
        EntityIdToIndex[EntityIdOfLastElement] = IndexOfRemoved;
        IndexToEntityId[IndexOfRemoved] = EntityIdOfLastElement;

        EntityIdToIndex.erase(EntityID);
        IndexToEntityId.erase(IndexOfLast);

        Size--;
    }

    void RemoveEntityFromPool(i32 EntityID) override
    {
        if(EntityIdToIndex.find(EntityID) != EntityIdToIndex.end())
        {
            Remove(EntityID);
        }
    }

    tobject& Get(i32 EntityID)
    {
        i32 Index = EntityIdToIndex[EntityID];
        return static_cast<tobject&>(Data[Index]);
    }

    tobject& operator[](u32 Index)
    {
        return Data[Index];
    }
};


typedef std::unordered_map<std::type_index, std::shared_ptr<entity_system>> system_pool;
class registry
{
private:
    i32 EntityCount = 0;
    std::set<entity> HotEntities;
    std::set<entity> ColdEntities;
    std::deque<i32> FreedIDs;

    std::unordered_map<std::string, entity> EntityPerTag;
    std::unordered_map<i32, std::string> TagPerEntity;

    std::unordered_map<std::string, std::set<entity>> EntitiesPerGroup;
    std::unordered_map<i32, std::string> GroupPerEntity;

    std::vector<std::shared_ptr<pool_>> ComponentsPool;
    std::vector<signature> EntityComponentSignature;

    system_pool Systems;

public:
    registry() = default;
    entity CreateEntity();
    void Update();
    void AddEntityToSystems(entity Entity);
    void RemoveEntityFromSystems(entity Entity);
    void KillEntity(entity Entity);

    void TagEntity(entity Entity, const std::string& Tag);
    b32 EntityHasTag(entity Entity, const std::string& Tag) const;
    entity GetEntityByTag(const std::string& Tag) const;
    void RemoveEntityTag(entity Entity);

    void GroupEntity(entity Entity, const std::string& Group);
    b32 EntityBelongsToGroup(entity Entity, const std::string& Group) const;
    std::vector<entity> GetEntitiesByGroup(const std::string& Group) const;
    void RemoveEntityGroup(entity Entity);

    template<typename tcomponent, typename ...targs> void AddComponent(entity Entity, targs&& ...Args);
    template<typename tcomponent> void RemoveComponent(entity Entity);
    template<typename tcomponent> b32 HasComponent(entity Entity);
    template<typename tcomponent> tcomponent& GetComponent(entity Entity);

    template<typename tsystem, typename ...targs> void AddSystem(targs&& ...Args);
    template<typename tsystem> void RemoveSystem();
    template<typename tsystem> b32 HasSystem() const;
    template<typename tsystem> tsystem& GetSystem() const;
};

#endif
