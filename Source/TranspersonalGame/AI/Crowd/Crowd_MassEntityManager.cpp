#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Default herd configurations
    FCrowd_HerdData DefaultHerbivoreHerd;
    DefaultHerbivoreHerd.EntityCount = 500;
    DefaultHerbivoreHerd.MovementSpeed = 120.0f;
    DefaultHerbivoreHerd.FlockingRadius = 800.0f;
    DefaultHerbivoreHerd.BehaviorType = ECrowd_BehaviorType::Grazing;
    HerdConfigurations.Add(DefaultHerbivoreHerd);
    
    FCrowd_HerdData DefaultPredatorPack;
    DefaultPredatorPack.EntityCount = 50;
    DefaultPredatorPack.MovementSpeed = 250.0f;
    DefaultPredatorPack.FlockingRadius = 300.0f;
    DefaultPredatorPack.BehaviorType = ECrowd_BehaviorType::Hunting;
    HerdConfigurations.Add(DefaultPredatorPack);
    
    // LOD settings for performance
    LODSettings.HighDetailDistance = 1000.0f;
    LODSettings.MediumDetailDistance = 2500.0f;
    LODSettings.LowDetailDistance = 5000.0f;
    LODSettings.MaxHighDetailEntities = 200;
    LODSettings.MaxMediumDetailEntities = 1000;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntitySystem();
    
    // Spawn default herds
    for (const FCrowd_HerdData& HerdData : HerdConfigurations)
    {
        SpawnHerd(HerdData);
    }
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateCrowdBehavior(DeltaTime);
        UpdateLODSystem();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager: No valid world"));
        return;
    }
    
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager: Failed to get MassEntitySubsystem"));
        return;
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager: Failed to get MassSpawnerSubsystem"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager: Mass Entity system initialized"));
}

void ACrowd_MassEntityManager::SpawnHerd(const FCrowd_HerdData& HerdData)
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager: Mass systems not initialized"));
        return;
    }
    
    if (CurrentEntityCount + HerdData.EntityCount > MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager: Entity limit reached, cannot spawn herd"));
        return;
    }
    
    // Create entities in a circular formation around center location
    for (int32 i = 0; i < HerdData.EntityCount; i++)
    {
        float Angle = (2.0f * PI * i) / HerdData.EntityCount;
        float Distance = FMath::RandRange(50.0f, HerdData.FlockingRadius);
        
        FVector SpawnLocation = HerdData.CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Create entity through Mass Entity system
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        if (EntityHandle.IsValid())
        {
            ActiveEntities.Add(EntityHandle);
            CurrentEntityCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager: Spawned herd with %d entities. Total: %d"), 
           HerdData.EntityCount, CurrentEntityCount);
}

void ACrowd_MassEntityManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Update flocking behavior for all active entities
    if (bEnableFlocking)
    {
        // Implement basic flocking rules: separation, alignment, cohesion
        for (int32 i = 0; i < ActiveEntities.Num(); i++)
        {
            if (!ActiveEntities[i].IsValid())
            {
                ActiveEntities.RemoveAt(i);
                i--;
                CurrentEntityCount--;
                continue;
            }
            
            // Basic flocking behavior would be implemented here
            // For now, we just validate entity existence
        }
    }
    
    // Update avoidance behavior
    if (bEnableAvoidance)
    {
        // Implement obstacle avoidance and inter-entity collision avoidance
    }
}

void ACrowd_MassEntityManager::SetHerdDestination(int32 HerdIndex, FVector NewDestination)
{
    if (HerdIndex >= 0 && HerdIndex < HerdConfigurations.Num())
    {
        HerdConfigurations[HerdIndex].CenterLocation = NewDestination;
        UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager: Set herd %d destination to %s"), 
               HerdIndex, *NewDestination.ToString());
    }
}

void ACrowd_MassEntityManager::UpdateLODSystem()
{
    // Implement LOD system based on distance from player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 HighDetailCount = 0;
    int32 MediumDetailCount = 0;
    
    for (const FMassEntityHandle& EntityHandle : ActiveEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        // Calculate distance to player (simplified - would need actual entity positions)
        float Distance = FVector::Dist(PlayerLocation, GetActorLocation());
        
        if (Distance <= LODSettings.HighDetailDistance && HighDetailCount < LODSettings.MaxHighDetailEntities)
        {
            // Set high detail LOD
            HighDetailCount++;
        }
        else if (Distance <= LODSettings.MediumDetailDistance && MediumDetailCount < LODSettings.MaxMediumDetailEntities)
        {
            // Set medium detail LOD
            MediumDetailCount++;
        }
        else
        {
            // Set low detail LOD or cull
        }
    }
}

void ACrowd_MassEntityManager::ClearAllHerds()
{
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& EntityHandle : ActiveEntities)
        {
            if (EntityHandle.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
    }
    
    ActiveEntities.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager: Cleared all herds"));
}

void ACrowd_MassEntityManager::DebugSpawnTestHerd()
{
    FCrowd_HerdData TestHerd;
    TestHerd.EntityCount = 100;
    TestHerd.MovementSpeed = 200.0f;
    TestHerd.FlockingRadius = 400.0f;
    TestHerd.CenterLocation = GetActorLocation() + FVector(500.0f, 0.0f, 0.0f);
    TestHerd.BehaviorType = ECrowd_BehaviorType::Migrating;
    
    SpawnHerd(TestHerd);
}

void ACrowd_MassEntityManager::DebugShowHerdStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CROWD SYSTEM STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Entities: %d / %d"), CurrentEntityCount, MaxSimultaneousEntities);
    UE_LOG(LogTemp, Warning, TEXT("Configured Herds: %d"), HerdConfigurations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Update Frequency: %.2f"), UpdateFrequency);
    UE_LOG(LogTemp, Warning, TEXT("Flocking Enabled: %s"), bEnableFlocking ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Avoidance Enabled: %s"), bEnableAvoidance ? TEXT("Yes") : TEXT("No"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Crowd System: %d/%d entities active"), CurrentEntityCount, MaxSimultaneousEntities));
    }
}