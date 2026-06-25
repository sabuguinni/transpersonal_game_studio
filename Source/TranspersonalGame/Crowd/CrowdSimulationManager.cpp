#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // update at 2Hz for performance

    MaxActiveAgents     = 50;
    CrowdUpdateInterval = 1.0f;
    ActiveAgentCount    = 0;
    TimeSinceLastUpdate = 0.f;

    // Seed default groups matching the MinPlayableMap population
    // Triceratops herd — grazing near (3200, 2000)
    FCrowd_DinoGroup TrikeHerd;
    TrikeHerd.GroupID       = TEXT("TrikeHerd_North");
    TrikeHerd.Role          = ECrowd_DinoRole::HerdMember;
    TrikeHerd.HomeLocation  = FVector(3200.f, 2000.f, 400.f);
    TrikeHerd.WanderRadius  = 1500.f;
    TrikeHerd.GroupSize     = 4;
    TrikeHerd.bIsActive     = true;
    RegisteredGroups.Add(TrikeHerd);

    // Brachiosaurus pair — slow wanderers near (4200, 2350)
    FCrowd_DinoGroup BrachioGroup;
    BrachioGroup.GroupID      = TEXT("BrachioGroup_East");
    BrachioGroup.Role         = ECrowd_DinoRole::SolitaryWanderer;
    BrachioGroup.HomeLocation = FVector(4200.f, 2350.f, 400.f);
    BrachioGroup.WanderRadius = 2000.f;
    BrachioGroup.GroupSize    = 2;
    BrachioGroup.bIsActive    = true;
    RegisteredGroups.Add(BrachioGroup);

    // Parasaurolophus herd — near (2800, 3300)
    FCrowd_DinoGroup ParaHerd;
    ParaHerd.GroupID      = TEXT("ParaHerd_South");
    ParaHerd.Role         = ECrowd_DinoRole::HerdMember;
    ParaHerd.HomeLocation = FVector(2800.f, 3300.f, 400.f);
    ParaHerd.WanderRadius = 1200.f;
    ParaHerd.GroupSize    = 3;
    ParaHerd.bIsActive    = true;
    RegisteredGroups.Add(ParaHerd);

    // Raptor pack — predators near (1900, 3650)
    FCrowd_DinoGroup RaptorPack;
    RaptorPack.GroupID      = TEXT("RaptorPack_West");
    RaptorPack.Role         = ECrowd_DinoRole::PackPredator;
    RaptorPack.HomeLocation = FVector(1900.f, 3650.f, 400.f);
    RaptorPack.WanderRadius = 3000.f;
    RaptorPack.GroupSize    = 4;
    RaptorPack.bIsActive    = true;
    RegisteredGroups.Add(RaptorPack);

    // Protoceratops herd — small background crowd near (1000, 4150)
    FCrowd_DinoGroup ProtoHerd;
    ProtoHerd.GroupID      = TEXT("ProtoHerd_Far");
    ProtoHerd.Role         = ECrowd_DinoRole::HerdMember;
    ProtoHerd.HomeLocation = FVector(1000.f, 4150.f, 400.f);
    ProtoHerd.WanderRadius = 800.f;
    ProtoHerd.GroupSize    = 4;
    ProtoHerd.bIsActive    = true;
    RegisteredGroups.Add(ProtoHerd);
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Count initial active agents
    ActiveAgentCount = 0;
    for (const FCrowd_DinoGroup& Group : RegisteredGroups)
    {
        if (Group.bIsActive)
        {
            ActiveAgentCount += Group.GroupSize;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSimMgr] Initialized — %d groups, %d agents"),
           RegisteredGroups.Num(), ActiveAgentCount);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate < CrowdUpdateInterval)
    {
        return;
    }
    TimeSinceLastUpdate = 0.f;

    // LOD budget enforcement: deactivate groups beyond MaxActiveAgents
    int32 RunningCount = 0;
    for (FCrowd_DinoGroup& Group : RegisteredGroups)
    {
        if (!Group.bIsActive) continue;
        RunningCount += Group.GroupSize;
        if (RunningCount > MaxActiveAgents)
        {
            Group.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSimMgr] LOD budget exceeded — deactivating group: %s"),
                   *Group.GroupID);
        }
    }

    ActiveAgentCount = FMath::Min(RunningCount, MaxActiveAgents);
}

void ACrowdSimulationManager::RegisterGroup(const FCrowd_DinoGroup& NewGroup)
{
    // Prevent duplicate IDs
    for (const FCrowd_DinoGroup& Existing : RegisteredGroups)
    {
        if (Existing.GroupID == NewGroup.GroupID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSimMgr] Group already registered: %s"), *NewGroup.GroupID);
            return;
        }
    }
    RegisteredGroups.Add(NewGroup);
    if (NewGroup.bIsActive)
    {
        ActiveAgentCount += NewGroup.GroupSize;
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimMgr] Registered group: %s (size=%d)"),
           *NewGroup.GroupID, NewGroup.GroupSize);
}

void ACrowdSimulationManager::UnregisterGroup(const FString& GroupID)
{
    for (int32 i = RegisteredGroups.Num() - 1; i >= 0; --i)
    {
        if (RegisteredGroups[i].GroupID == GroupID)
        {
            if (RegisteredGroups[i].bIsActive)
            {
                ActiveAgentCount -= RegisteredGroups[i].GroupSize;
                ActiveAgentCount  = FMath::Max(0, ActiveAgentCount);
            }
            RegisteredGroups.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("[CrowdSimMgr] Unregistered group: %s"), *GroupID);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[CrowdSimMgr] Group not found for removal: %s"), *GroupID);
}

TArray<FCrowd_DinoGroup> ACrowdSimulationManager::GetGroupsByRole(ECrowd_DinoRole Role) const
{
    TArray<FCrowd_DinoGroup> Result;
    for (const FCrowd_DinoGroup& Group : RegisteredGroups)
    {
        if (Group.Role == Role)
        {
            Result.Add(Group);
        }
    }
    return Result;
}

void ACrowdSimulationManager::TriggerHerdPanic(const FVector& ThreatLocation, float PanicRadius)
{
    for (FCrowd_DinoGroup& Group : RegisteredGroups)
    {
        if (!Group.bIsActive) continue;
        if (Group.Role == ECrowd_DinoRole::PackPredator) continue; // predators don't panic

        float Distance = FVector::Dist(Group.HomeLocation, ThreatLocation);
        if (Distance <= PanicRadius)
        {
            // Expand wander radius — herd scatters
            Group.WanderRadius *= 2.f;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSimMgr] PANIC triggered for group %s (dist=%.0f, radius=%.0f)"),
                   *Group.GroupID, Distance, PanicRadius);
        }
    }
}
