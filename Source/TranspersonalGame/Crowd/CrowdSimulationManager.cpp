#include "CrowdSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for crowd management
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntitySystem();
    
    // Start with some initial herds
    if (CrowdSpawnConfigs.Num() > 0)
    {
        for (const FCrowdSpawnParameters& Config : CrowdSpawnConfigs)
        {
            // Spawn initial herds at random locations around the map
            FVector SpawnLocation = GetActorLocation() + FMath::VRand() * Config.SpawnRadius;
            SpawnCrowd(Config, SpawnLocation);
        }
    }
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCrowdBehaviors(DeltaTime);
    ProcessEnvironmentalInfluences();
    ManageLODSystem();
}

void ACrowdSimulationManager::InitializeMassEntitySystem()
{
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: Failed to get MassEntitySubsystem"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Mass Entity System initialized"));
}

void ACrowdSimulationManager::SpawnCrowd(const FCrowdSpawnParameters& SpawnParams, const FVector& Location)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: MassEntitySubsystem not available"));
        return;
    }
    
    // Calculate actual group size
    int32 GroupSize = FMath::RandRange(SpawnParams.MinGroupSize, SpawnParams.MaxGroupSize);
    
    // Check if we would exceed max agents
    if (ActiveCrowdEntities.Num() + GroupSize > MaxSimultaneousAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Cannot spawn crowd - would exceed max agents limit"));
        return;
    }
    
    // Spawn entities in formation based on crowd type
    for (int32 i = 0; i < GroupSize; i++)
    {
        FVector SpawnOffset;
        
        switch (SpawnParams.CrowdType)
        {
            case ECrowdType::HerbivoreHerd:
                // Clustered formation for protection
                SpawnOffset = FMath::VRand() * SpawnParams.CohesionRadius * 0.5f;
                break;
                
            case ECrowdType::PredatorPack:
                // Loose formation for hunting
                SpawnOffset = FMath::VRand() * SpawnParams.CohesionRadius * 0.8f;
                break;
                
            case ECrowdType::FlyingSwarm:
                // 3D formation with vertical spread
                SpawnOffset = FMath::VRand() * SpawnParams.CohesionRadius;
                SpawnOffset.Z = FMath::RandRange(-200.0f, 500.0f);
                break;
                
            default:
                SpawnOffset = FMath::VRand() * SpawnParams.CohesionRadius;
                break;
        }
        
        FVector FinalSpawnLocation = Location + SpawnOffset;
        
        // Create Mass Entity (placeholder - actual implementation would use Mass spawning system)
        FMassEntityHandle EntityHandle; // This would be properly created via Mass system
        ActiveCrowdEntities.Add(EntityHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d agents of type %d at location %s"), 
           GroupSize, (int32)SpawnParams.CrowdType, *Location.ToString());
}

void ACrowdSimulationManager::TriggerMassFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    // This would trigger a mass flee behavior in all crowds within the threat radius
    // Implementation would use Mass Entity queries to find affected entities
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Mass flee response triggered at %s with radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);
    
    // Pseudo-code for Mass Entity implementation:
    // 1. Query all entities within ThreatRadius of ThreatLocation
    // 2. Set their behavior state to Fleeing
    // 3. Calculate flee direction away from ThreatLocation
    // 4. Increase movement speed and cohesion for panic behavior
}

void ACrowdSimulationManager::StartSeasonalMigration(ECrowdType CrowdType, const FVector& TargetLocation)
{
    // Trigger migration behavior for all crowds of the specified type
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Starting seasonal migration for type %d to %s"), 
           (int32)CrowdType, *TargetLocation.ToString());
    
    // Implementation would:
    // 1. Find all entities of the specified CrowdType
    // 2. Set their behavior state to Migrating
    // 3. Set TargetLocation as their long-term movement goal
    // 4. Adjust movement parameters for long-distance travel
}

void ACrowdSimulationManager::UpdateCrowdBehaviors(float DeltaTime)
{
    LastBehaviorUpdateTime += DeltaTime;
    
    if (LastBehaviorUpdateTime >= BehaviorUpdateInterval)
    {
        LastBehaviorUpdateTime = 0.0f;
        
        // Update crowd behaviors at reduced frequency for performance
        // This is where the main crowd logic would run:
        // 1. Boids algorithm (cohesion, separation, alignment)
        // 2. Environmental response (water seeking, food seeking)
        // 3. Predator-prey interactions
        // 4. State transitions (grazing -> fleeing -> grazing)
    }
}

void ACrowdSimulationManager::ProcessEnvironmentalInfluences()
{
    // Process environmental factors that influence crowd behavior
    
    // Water sources - attract thirsty herds
    for (AActor* WaterSource : WaterSources)
    {
        if (WaterSource)
        {
            // Attract nearby crowds to water source during certain times
            // Implementation would check time of day and crowd thirst levels
        }
    }
    
    // Feeding areas - attract herbivores
    for (AActor* FeedingArea : FeedingAreas)
    {
        if (FeedingArea)
        {
            // Attract herbivore crowds to feeding areas
            // Consider food availability and competition
        }
    }
    
    // Resting areas - attract tired crowds
    for (AActor* RestingArea : RestingAreas)
    {
        if (RestingArea)
        {
            // Attract crowds during rest periods (night, midday heat)
        }
    }
}

void ACrowdSimulationManager::ManageLODSystem()
{
    // Implement 3-tier LOD system for performance
    // This would be integrated with the Mass Entity LOD system
    
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Categorize crowds by distance from player
        // LOD 0 (0-1000m): Full simulation with all behaviors
        // LOD 1 (1000-3000m): Simplified simulation, reduced update frequency
        // LOD 2 (3000-8000m): Visual representation only, minimal logic
        // LOD 3 (8000m+): Culled completely
        
        // Implementation would update Mass Entity LOD levels based on distance
    }
}

int32 ACrowdSimulationManager::GetActiveCrowdCount() const
{
    return ActiveCrowdEntities.Num();
}

TArray<FVector> ACrowdSimulationManager::GetNearestCrowdPositions(const FVector& QueryLocation, float Radius) const
{
    TArray<FVector> NearbyPositions;
    
    // Implementation would query Mass Entity system for entities within radius
    // and return their positions
    
    return NearbyPositions;
}