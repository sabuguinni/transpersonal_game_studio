#include "Crowd_MassEntitySubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    MassEntitySubsystem = nullptr;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeMassEntity();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem initialized with max entities: %d"), MaxCrowdEntities);
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    CleanupMassEntity();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem deinitialized"));
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassEntitySubsystem::InitializeMassEntity()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
            return;
        }
        
        // Initialize group behaviors map
        GroupBehaviors.Empty();
        ActiveCrowdEntities.Empty();
        
        UE_LOG(LogTemp, Warning, TEXT("Mass Entity framework initialized for crowd simulation"));
    }
}

void UCrowd_MassEntitySubsystem::CleanupMassEntity()
{
    DespawnAllCrowdEntities();
    
    GroupBehaviors.Empty();
    MassEntitySubsystem = nullptr;
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(const TArray<FCrowd_EntitySpawnData>& SpawnData)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available for crowd spawning"));
        return;
    }

    int32 SpawnedCount = 0;
    for (const FCrowd_EntitySpawnData& Data : SpawnData)
    {
        if (ActiveCrowdEntities.Num() >= MaxCrowdEntities)
        {
            UE_LOG(LogTemp, Warning, TEXT("Max crowd entities reached: %d"), MaxCrowdEntities);
            break;
        }

        FMassEntityHandle EntityHandle = CreateCrowdEntity(Data);
        if (EntityHandle.IsValid())
        {
            ActiveCrowdEntities.Add(EntityHandle);
            SpawnedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities. Total active: %d"), SpawnedCount, ActiveCrowdEntities.Num());
}

void UCrowd_MassEntitySubsystem::DespawnAllCrowdEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    ActiveCrowdEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("All crowd entities despawned"));
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    return ActiveCrowdEntities.Num();
}

void UCrowd_MassEntitySubsystem::SetGroupBehavior(const FCrowd_GroupBehavior& GroupBehavior)
{
    GroupBehaviors.Add(GroupBehavior.GroupID, GroupBehavior);
    
    UE_LOG(LogTemp, Warning, TEXT("Group behavior set for GroupID %d: Target(%s), Cohesion(%f)"), 
           GroupBehavior.GroupID, 
           *GroupBehavior.TargetLocation.ToString(), 
           GroupBehavior.CohesionRadius);
}

void UCrowd_MassEntitySubsystem::UpdateGroupTarget(int32 GroupID, const FVector& NewTarget)
{
    if (FCrowd_GroupBehavior* GroupBehavior = GroupBehaviors.Find(GroupID))
    {
        GroupBehavior->TargetLocation = NewTarget;
        UE_LOG(LogTemp, Log, TEXT("Updated target for GroupID %d to %s"), GroupID, *NewTarget.ToString());
    }
}

void UCrowd_MassEntitySubsystem::SetMaxCrowdEntities(int32 MaxEntities)
{
    MaxCrowdEntities = FMath::Clamp(MaxEntities, 100, 50000);
    UE_LOG(LogTemp, Warning, TEXT("Max crowd entities set to: %d"), MaxCrowdEntities);
}

void UCrowd_MassEntitySubsystem::EnableLODSystem(bool bEnable)
{
    bLODSystemEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("Crowd LOD system %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FMassEntityHandle UCrowd_MassEntitySubsystem::CreateCrowdEntity(const FCrowd_EntitySpawnData& SpawnData)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }

    // Create basic mass entity
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    if (EntityHandle.IsValid())
    {
        // Entity created successfully - components would be added here in full implementation
        UE_LOG(LogTemp, VeryVerbose, TEXT("Created crowd entity at %s"), *SpawnData.SpawnLocation.ToString());
    }
    
    return EntityHandle;
}

void UCrowd_MassEntitySubsystem::UpdateCrowdEntities(float DeltaTime)
{
    if (!MassEntitySubsystem || ActiveCrowdEntities.Num() == 0)
    {
        return;
    }

    // Mass entity update logic would go here
    // This is called by the Mass Entity framework automatically
}