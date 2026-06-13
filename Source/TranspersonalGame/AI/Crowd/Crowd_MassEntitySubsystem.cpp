#include "Crowd_MassEntitySubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassArchetypeTypes.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeMassEntity();
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    DespawnAllCrowdEntities();
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::InitializeMassEntity()
{
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (MassEntitySubsystem)
    {
        CreateCrowdArchetype();
        bIsInitialized = true;
    }
}

void UCrowd_MassEntitySubsystem::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create archetype for crowd entities
    FMassArchetypeCreationParams CreationParams;
    CreationParams.ChunkMemorySize = 16 * 1024; // 16KB chunks
    
    // Add basic components for crowd entities
    // Note: In a real implementation, you would add specific Mass components here
    // such as FMassTransformFragment, FMassVelocityFragment, etc.
    
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(CreationParams);
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(const FVector& Location, int32 Count)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }

    // Clamp count to max entities
    Count = FMath::Min(Count, CrowdConfig.MaxEntities - ActiveCrowdEntities.Num());

    for (int32 i = 0; i < Count; i++)
    {
        // Generate random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, CrowdConfig.SpawnRadius);
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        SpawnEntityAtLocation(SpawnLocation);
    }
}

void UCrowd_MassEntitySubsystem::SpawnEntityAtLocation(const FVector& Location)
{
    if (!MassEntitySubsystem || !CrowdArchetype.IsValid())
    {
        return;
    }

    // Spawn entity using Mass Entity system
    FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity(CrowdArchetype);
    if (NewEntity.IsValid())
    {
        ActiveCrowdEntities.Add(NewEntity);
        
        // Set initial transform (simplified - in real implementation would use MassTransformFragment)
        // This is a placeholder for the actual Mass Entity transform setting
    }
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
}

void UCrowd_MassEntitySubsystem::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Calculate distance to player (simplified)
            // In real implementation, would get entity position from MassTransformFragment
            float Distance = 1000.0f; // Placeholder distance
            UpdateEntityLOD(EntityHandle, Distance);
        }
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD(const FMassEntityHandle& EntityHandle, float DistanceToPlayer)
{
    // Determine LOD level based on distance
    int32 LODLevel = 0;
    if (DistanceToPlayer > CrowdConfig.LODDistance3)
    {
        LODLevel = 3; // Lowest detail
    }
    else if (DistanceToPlayer > CrowdConfig.LODDistance2)
    {
        LODLevel = 2;
    }
    else if (DistanceToPlayer > CrowdConfig.LODDistance1)
    {
        LODLevel = 1;
    }
    else
    {
        LODLevel = 0; // Highest detail
    }

    // Apply LOD settings (placeholder - would set actual Mass components)
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    return ActiveCrowdEntities.Num();
}

void UCrowd_MassEntitySubsystem::SetCrowdEntityConfig(const FCrowd_MassEntityConfig& NewConfig)
{
    CrowdConfig = NewConfig;
}