
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

struct pool_{};

template<typename T>
struct pool : pool_
{
    std::vector<T> Data;

    pool(i32 Size = 100)
    {
        Data.resize(Size);
    }

    ~pool() = default;

    T& operator[](u32 Index)
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

    std::vector<std::shared_ptr<pool_>> ComponentsPool;
    std::vector<signature> EntityComponentSignature;

    system_pool Systems;

public:
    registry() = default;
    entity CreateEntity();
    void Update();
    void AddEntityToSystems(entity Entity);

    template<typename tcomponent, typename ...targs> void AddComponent(entity Entity, targs&& ...Args);
    template<typename tcomponent> void RemoveComponent(entity Entity);
    template<typename tcomponent> b32 HasComponent(entity Entity);
    template<typename tcomponent> tcomponent GetComponent(entity Entity);

    template<typename tsystem, typename ...targs> void AddSystem(targs&& ...Args);
    template<typename tsystem> void RemoveSystem();
    template<typename tsystem> b32 HasSystem() const;
    template<typename tsystem> tsystem& GetSystem() const;
};

#endif
