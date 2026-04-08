#include "EcosystemCrowdManager.h"
#include "MassEntitySubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void UEcosystemCrowdManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Initializing prehistoric ecosystem simulation"));
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSimulationSubsystem = GetWorld()->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem || !MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemCrowdManager: Failed to get Mass Entity subsystems"));
        return;
    }
    
    // Initialize ecosystem update timers
    GetWorld()->GetTimerManager().SetTimer(
        EcosystemUpdateTimer,
        this,
        &UEcosystemCrowdManager::UpdateEcosystemBalance,
        5.0f, // Update every 5 seconds
        true
    );
    
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceOptimizationTimer,
        this,
        &UEcosystemCrowdManager::OptimizeLODLevels,
        1.0f, // Optimize every second
        true
    );
}

void UEcosystemCrowdManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EcosystemUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceOptimizationTimer);
    }
    
    // Clean up entity arrays
    HerbivoreEntities.Empty();
    PredatorEntities.Empty();
    AmbientEntities.Empty();
    
    Super::Deinitialize();
}

bool UEcosystemCrowdManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UEcosystemCrowdManager::InitializeEcosystem(const FVector& WorldCenter, float WorldRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Initializing ecosystem at %s with radius %f"), 
           *WorldCenter.ToString(), WorldRadius);
    
    // Define grazing zones (herbivore gathering areas)
    GrazingZones.Empty();
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (2.0f * PI * i) / 8.0f;
        FVector GrazingLocation = WorldCenter + FVector(
            FMath::Cos(Angle) * WorldRadius * 0.6f,
            FMath::Sin(Angle) * WorldRadius * 0.6f,
            0.0f
        );
        GrazingZones.Add(GrazingLocation);
    }
    
    // Define water sources (critical gathering points)
    WaterSources.Empty();
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (2.0f * PI * i) / 4.0f;
        FVector WaterLocation = WorldCenter + FVector(
            FMath::Cos(Angle) * WorldRadius * 0.3f,
            FMath::Sin(Angle) * WorldRadius * 0.3f,
            0.0f
        );
        WaterSources.Add(WaterLocation);
    }
    
    // Define predator territories (overlapping with grazing zones for hunting)
    PredatorTerritories.Empty();
    for (int32 i = 0; i < 6; i++)
    {
        float Angle = (2.0f * PI * i) / 6.0f;
        FVector TerritoryLocation = WorldCenter + FVector(
            FMath::Cos(Angle) * WorldRadius * 0.8f,
            FMath::Sin(Angle) * WorldRadius * 0.8f,
            0.0f
        );
        PredatorTerritories.Add(TerritoryLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Created %d grazing zones, %d water sources, %d predator territories"),
           GrazingZones.Num(), WaterSources.Num(), PredatorTerritories.Num());
}

void UEcosystemCrowdManager::SpawnHerbivoreHerd(const FVector& Location, int32 HerdSize, TSubclassOf<class ADinosaur> DinosaurClass)
{
    if (!MassEntitySubsystem || CurrentPopulation + HerdSize > MaxPopulation)
    {
        UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Cannot spawn herbivore herd - population limit reached"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawning herbivore herd of %d at %s"), 
           HerdSize, *Location.ToString());
    
    // Create Mass entities for herbivore herd
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Random position within herd formation
        FVector HerdOffset = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        
        FVector SpawnLocation = Location + HerdOffset;
        
        // Create entity through Mass Entity system
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        
        if (EntityHandle.IsValid())
        {
            HerbivoreEntities.Add(EntityHandle);
            CurrentPopulation++;
            
            // Add transform fragment
            FTransform EntityTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
            // Note: Actual fragment addition would require proper Mass Entity archetype setup
            // This is a simplified version for demonstration
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawned %d herbivores. Current population: %d"), 
           HerdSize, CurrentPopulation);
}

void UEcosystemCrowdManager::SpawnPredatorPack(const FVector& Location, int32 PackSize, TSubclassOf<class ADinosaur> DinosaurClass)
{
    if (!MassEntitySubsystem || CurrentPopulation + PackSize > MaxPopulation)
    {
        UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Cannot spawn predator pack - population limit reached"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawning predator pack of %d at %s"), 
           PackSize, *Location.ToString());
    
    // Create Mass entities for predator pack
    for (int32 i = 0; i < PackSize; i++)
    {
        // Random position within pack formation
        FVector PackOffset = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
        
        FVector SpawnLocation = Location + PackOffset;
        
        // Create entity through Mass Entity system
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        
        if (EntityHandle.IsValid())
        {
            PredatorEntities.Add(EntityHandle);
            CurrentPopulation++;
            
            // Add transform fragment
            FTransform EntityTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
            // Note: Actual fragment addition would require proper Mass Entity archetype setup
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawned %d predators. Current population: %d"), 
           PackSize, CurrentPopulation);
}

void UEcosystemCrowdManager::SpawnAmbientWildlife(const FVector& Location, float Radius, int32 Count)
{
    if (!MassEntitySubsystem || CurrentPopulation + Count > MaxPopulation)
    {
        UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Cannot spawn ambient wildlife - population limit reached"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawning %d ambient wildlife around %s"), 
           Count, *Location.ToString());
    
    // Create Mass entities for ambient wildlife (small creatures, birds, etc.)
    for (int32 i = 0; i < Count; i++)
    {
        // Random position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius);
        
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(0.0f, 100.0f) // Some height variation for flying creatures
        );
        
        // Create entity through Mass Entity system
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        
        if (EntityHandle.IsValid())
        {
            AmbientEntities.Add(EntityHandle);
            CurrentPopulation++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Spawned %d ambient creatures. Current population: %d"), 
           Count, CurrentPopulation);
}

void UEcosystemCrowdManager::CullExcessPopulation()
{
    if (CurrentPopulation <= MaxPopulation)
    {
        return;
    }
    
    int32 ExcessCount = CurrentPopulation - MaxPopulation;
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Culling %d excess entities"), ExcessCount);
    
    // Prioritize culling ambient wildlife first, then distant entities
    int32 CulledCount = 0;
    
    // Cull ambient wildlife first
    for (int32 i = AmbientEntities.Num() - 1; i >= 0 && CulledCount < ExcessCount; i--)
    {
        if (AmbientEntities[i].IsValid())
        {
            MassEntitySubsystem->DestroyEntity(AmbientEntities[i]);
            AmbientEntities.RemoveAt(i);
            CulledCount++;
            CurrentPopulation--;
        }
    }
    
    // If still excess, cull distant herbivores
    if (CulledCount < ExcessCount)
    {
        // Implementation would check distance from player and cull furthest entities
        // Simplified for demonstration
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Culled %d entities. New population: %d"), 
           CulledCount, CurrentPopulation);
}

float UEcosystemCrowdManager::GetSimulationPerformance() const
{
    // Return performance metric (entities per millisecond or similar)
    // This would be calculated based on actual frame time measurements
    if (CurrentPopulation > 0)
    {
        return static_cast<float>(CurrentPopulation) / FMath::Max(GetWorld()->GetDeltaSeconds() * 1000.0f, 1.0f);
    }
    return 0.0f;
}

void UEcosystemCrowdManager::SetLODDistances(float Near, float Medium, float Far)
{
    NearLODDistance = Near;
    MediumLODDistance = Medium;
    FarLODDistance = Far;
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemCrowdManager: Updated LOD distances - Near: %f, Medium: %f, Far: %f"), 
           Near, Medium, Far);
}

void UEcosystemCrowdManager::UpdateEcosystemBalance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Process migration patterns
    ProcessMigrationPatterns();
    
    // Handle predator-prey interactions
    HandlePredatorPreyInteractions();
    
    // Debug visualization in development builds
    #if WITH_EDITOR
    if (GrazingZones.Num() > 0)
    {
        for (const FVector& Zone : GrazingZones)
        {
            DrawDebugSphere(GetWorld(), Zone, 300.0f, 12, FColor::Green, false, 5.0f);
        }
    }
    
    if (WaterSources.Num() > 0)
    {
        for (const FVector& Water : WaterSources)
        {
            DrawDebugSphere(GetWorld(), Water, 200.0f, 12, FColor::Blue, false, 5.0f);
        }
    }
    
    if (PredatorTerritories.Num() > 0)
    {
        for (const FVector& Territory : PredatorTerritories)
        {
            DrawDebugSphere(GetWorld(), Territory, 400.0f, 12, FColor::Red, false, 5.0f);
        }
    }
    #endif
}

void UEcosystemCrowdManager::ProcessMigrationPatterns()
{
    // Implement seasonal migration patterns
    // Herbivores move between grazing zones
    // Predators follow prey
    // This would involve updating Mass Entity movement targets
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("EcosystemCrowdManager: Processing migration patterns for %d entities"), 
           CurrentPopulation);
}

void UEcosystemCrowdManager::HandlePredatorPreyInteractions()
{
    // Implement predator-prey dynamics
    // Predators hunt herbivores
    // Herbivores flee from predators
    // Population balance through natural selection
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("EcosystemCrowdManager: Processing predator-prey interactions"));
}

void UEcosystemCrowdManager::OptimizeLODLevels()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Process LOD for all entities based on distance from player
    // Near LOD: Full detail, all behaviors
    // Medium LOD: Reduced detail, simplified behaviors
    // Far LOD: Minimal detail, basic movement only
    
    // This would involve updating Mass Entity LOD components
    // Implementation simplified for demonstration
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("EcosystemCrowdManager: Optimizing LOD levels for %d entities"), 
           CurrentPopulation);
}

void UEcosystemCrowdManager::CullDistantEntities()
{
    // Remove entities that are too far from player to matter
    // This helps maintain performance by keeping population within reasonable bounds
    
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float CullDistance = FarLODDistance * 2.0f; // Cull beyond far LOD distance
    
    // Implementation would check entity distances and remove those beyond cull distance
    // Simplified for demonstration
}