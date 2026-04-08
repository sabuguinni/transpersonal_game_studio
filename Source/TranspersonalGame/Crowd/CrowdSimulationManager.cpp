#include "CrowdSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized with Mass Entity Framework"));
}

void UCrowdSimulationManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
    }
    
    Super::Deinitialize();
}

void UCrowdSimulationManager::InitializeCrowdSystem()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: No valid world found"));
        return;
    }

    // Setup update timer
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimer,
        this,
        &UCrowdSimulationManager::UpdateCrowdDensity,
        UpdateFrequency,
        true
    );

    // Initialize default density zones
    FCrowdDensityZone ForestZone;
    ForestZone.Center = FVector(0, 0, 0);
    ForestZone.Radius = 10000.0f;
    ForestZone.MaxEntities = 500;
    ForestZone.AllowedSpecies = {"Parasaurolophus", "Triceratops", "Compsognathus"};
    ForestZone.SpawnProbability = 0.8f;
    RegisterDensityZone(ForestZone);

    FCrowdDensityZone RiverZone;
    RiverZone.Center = FVector(5000, 0, 0);
    RiverZone.Radius = 3000.0f;
    RiverZone.MaxEntities = 200;
    RiverZone.AllowedSpecies = {"Parasaurolophus", "Brachiosaurus"};
    RiverZone.SpawnProbability = 0.6f;
    RegisterDensityZone(RiverZone);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: System initialized with %d density zones"), DensityZones.Num());
}

void UCrowdSimulationManager::SpawnDinosaurHerd(const FString& SpeciesName, const FVector& Location, int32 Count)
{
    if (!MassSpawnerSubsystem || !DinosaurSpeciesTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Missing required subsystems or data"));
        return;
    }

    // Get species data
    FDinosaurSpeciesData* SpeciesData = DinosaurSpeciesTable->FindRow<FDinosaurSpeciesData>(*SpeciesName, TEXT(""));
    if (!SpeciesData)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Species %s not found in data table"), *SpeciesName);
        return;
    }

    // Check entity limits
    if (CurrentEntityCount + Count > MaxGlobalEntities)
    {
        int32 AllowedCount = FMath::Max(0, MaxGlobalEntities - CurrentEntityCount);
        Count = AllowedCount;
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Entity limit reached, spawning only %d entities"), Count);
    }

    // Spawn entities in formation
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );

        // TODO: Integrate with Mass Entity spawning system
        // This would use MassSpawnerSubsystem to create entities with proper fragments
        
        CurrentEntityCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d %s at %s"), Count, *SpeciesName, *Location.ToString());
}

void UCrowdSimulationManager::RegisterDensityZone(const FCrowdDensityZone& Zone)
{
    DensityZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered density zone at %s with radius %f"), 
           *Zone.Center.ToString(), Zone.Radius);
}

void UCrowdSimulationManager::UpdateCrowdDensity(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    UpdatePlayerProximity();
    ManageEntityLOD();
    ProcessEmergentBehaviors();

    // Manage density zones
    for (const FCrowdDensityZone& Zone : DensityZones)
    {
        int32 EntitiesInZone = GetEntityCountInRadius(Zone.Center, Zone.Radius);
        
        if (EntitiesInZone < Zone.MaxEntities * 0.7f) // Spawn when below 70% capacity
        {
            // Randomly select species and spawn
            if (Zone.AllowedSpecies.Num() > 0 && FMath::RandRange(0.0f, 1.0f) < Zone.SpawnProbability * DeltaTime)
            {
                FString RandomSpecies = Zone.AllowedSpecies[FMath::RandRange(0, Zone.AllowedSpecies.Num() - 1)];
                FVector RandomLocation = Zone.Center + FVector(
                    FMath::RandRange(-Zone.Radius * 0.8f, Zone.Radius * 0.8f),
                    FMath::RandRange(-Zone.Radius * 0.8f, Zone.Radius * 0.8f),
                    0.0f
                );
                
                SpawnDinosaurHerd(RandomSpecies, RandomLocation, FMath::RandRange(1, 5));
            }
        }
    }
}

int32 UCrowdSimulationManager::GetEntityCountInRadius(const FVector& Location, float Radius) const
{
    // TODO: Implement actual entity counting using Mass Entity queries
    // This would query the MassEntitySubsystem for entities within the radius
    return FMath::RandRange(50, 150); // Placeholder
}

void UCrowdSimulationManager::UpdatePlayerProximity()
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // TODO: Implement player proximity effects
    // - Increase entity detail within PlayerInfluenceRadius
    // - Trigger flee/alert behaviors for nearby entities
    // - Adjust spawning rates based on player presence
}

void UCrowdSimulationManager::ManageEntityLOD()
{
    // TODO: Implement LOD management
    // - Reduce update frequency for distant entities
    // - Switch to simplified behaviors for far entities
    // - Cull entities beyond maximum distance
}

void UCrowdSimulationManager::ProcessEmergentBehaviors()
{
    // TODO: Implement emergent behavior processing
    // - Predator-prey interactions
    // - Herd formation and movement
    // - Territory establishment
    // - Feeding and resting cycles
}