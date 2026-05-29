#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassLODFragments.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsInitialized = false;
    CurrentEntityCount = 0;
    LastUpdateTime = 0.0f;
    
    // Initialize default config
    CrowdConfig = FCrowd_EntityConfig();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem Initialized"));
    
    InitializeMassFramework();
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    ClearAllEntities();
    
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
    
    Super::Deinitialize();
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassEntitySubsystem::InitializeMassFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Subsystem: No valid world"));
        return;
    }
    
    // Get Mass subsystems
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Subsystem: Failed to get MassEntitySubsystem"));
        return;
    }
    
    CreateEntityArchetype();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Framework initialized for Crowd Simulation"));
}

void UCrowd_MassEntitySubsystem::CreateEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create archetype with basic fragments
    FMassArchetypeCreationParams CreationParams;
    CreationParams.FragmentList.Add<FMassTransformFragment>();
    CreationParams.FragmentList.Add<FMassVelocityFragment>();
    CreationParams.FragmentList.Add<FMassRepresentationFragment>();
    CreationParams.FragmentList.Add<FMassViewerInfoFragment>();
    
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(CreationParams);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd archetype created"));
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(const FVector& Center, int32 Count, ECrowd_BiomeType BiomeType)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Subsystem not initialized"));
        return;
    }
    
    if (CurrentEntityCount + Count > CrowdConfig.MaxEntities)
    {
        Count = FMath::Max(0, CrowdConfig.MaxEntities - CurrentEntityCount);
        UE_LOG(LogTemp, Warning, TEXT("Limiting spawn count to %d to stay within MaxEntities"), Count);
    }
    
    TArray<FMassEntityHandle> NewEntities;
    NewEntities.Reserve(Count);
    
    for (int32 i = 0; i < Count; i++)
    {
        // Random position within spawn radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-CrowdConfig.SpawnRadius, CrowdConfig.SpawnRadius),
            FMath::RandRange(-CrowdConfig.SpawnRadius, CrowdConfig.SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = Center + RandomOffset;
        
        // Create entity
        FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity(CrowdArchetype);
        
        if (Entity.IsValid())
        {
            // Set transform
            FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
            TransformFragment.SetTransform(FTransform(SpawnLocation));
            
            // Set initial velocity
            FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(Entity);
            VelocityFragment.Value = FVector::ZeroVector;
            
            NewEntities.Add(Entity);
            ActiveEntities.Add(Entity);
        }
    }
    
    CurrentEntityCount += NewEntities.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities at %s (Total: %d)"), 
           NewEntities.Num(), *Center.ToString(), CurrentEntityCount);
}

void UCrowd_MassEntitySubsystem::UpdateCrowdMovement(float DeltaTime)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update every 0.1 seconds for performance
    if (LastUpdateTime < 0.1f)
    {
        return;
    }
    
    LastUpdateTime = 0.0f;
    
    // Get player position for LOD calculations
    FVector PlayerLocation = FVector::ZeroVector;
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->GetPawn())
        {
            PlayerLocation = PC->GetPawn()->GetActorLocation();
        }
    }
    
    // Update entities
    for (int32 i = ActiveEntities.Num() - 1; i >= 0; i--)
    {
        FMassEntityHandle& Entity = ActiveEntities[i];
        
        if (!Entity.IsValid())
        {
            ActiveEntities.RemoveAtSwap(i);
            CurrentEntityCount--;
            continue;
        }
        
        // Update LOD based on distance to player
        UpdateEntityLOD(Entity, FVector::Dist(PlayerLocation, 
            MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity).GetTransform().GetLocation()));
        
        // Simple movement behavior
        FMassTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
        FMassVelocityFragment& VelocityFragment = MassEntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(Entity);
        
        FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
        
        // Random movement target
        if (VelocityFragment.Value.IsNearlyZero())
        {
            FVector Target = GetRandomMovementTarget(CurrentLocation);
            FVector Direction = (Target - CurrentLocation).GetSafeNormal();
            VelocityFragment.Value = Direction * CrowdConfig.MovementSpeed;
        }
        
        // Apply movement
        FVector NewLocation = CurrentLocation + (VelocityFragment.Value * DeltaTime);
        TransformFragment.SetTransform(FTransform(NewLocation));
        
        // Randomly change direction occasionally
        if (FMath::RandRange(0.0f, 1.0f) < 0.01f)
        {
            VelocityFragment.Value = FVector::ZeroVector;
        }
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD(const FMassEntityHandle& Entity, float DistanceToPlayer)
{
    if (!Entity.IsValid() || !MassEntitySubsystem)
    {
        return;
    }
    
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
    
    // Apply LOD settings (simplified for now)
    // In a full implementation, this would affect rendering, update frequency, etc.
}

FVector UCrowd_MassEntitySubsystem::GetRandomMovementTarget(const FVector& CurrentPosition)
{
    float RandomRadius = FMath::RandRange(500.0f, 2000.0f);
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
    
    return CurrentPosition + Offset;
}

void UCrowd_MassEntitySubsystem::SetCrowdConfig(const FCrowd_EntityConfig& NewConfig)
{
    CrowdConfig = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("Crowd config updated: MaxEntities=%d, SpawnRadius=%.1f"), 
           CrowdConfig.MaxEntities, CrowdConfig.SpawnRadius);
}

int32 UCrowd_MassEntitySubsystem::GetActiveEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassEntitySubsystem::ClearAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    for (const FMassEntityHandle& Entity : ActiveEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    ActiveEntities.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("All crowd entities cleared"));
}

void UCrowd_MassEntitySubsystem::SetLODDistances(float LOD1, float LOD2, float LOD3)
{
    CrowdConfig.LODDistance1 = LOD1;
    CrowdConfig.LODDistance2 = LOD2;
    CrowdConfig.LODDistance3 = LOD3;
    
    UE_LOG(LogTemp, Warning, TEXT("LOD distances updated: %.1f, %.1f, %.1f"), LOD1, LOD2, LOD3);
}