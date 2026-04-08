#include "MassCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassAgentComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UMassCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MassEntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Initialized for prehistoric world simulation"));
}

void UMassCrowdSubsystem::Deinitialize()
{
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CrowdUpdateTimer);
        World->GetTimerManager().ClearTimer(MigrationUpdateTimer);
        World->GetTimerManager().ClearTimer(DensityAnalysisTimer);
    }

    Super::Deinitialize();
}

bool UMassCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UMassCrowdSubsystem::InitializeCrowdSystem()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: MassEntitySubsystem not available"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Initialize crowd density grid
    CrowdDensityGrid.Empty();
    
    // Set up periodic updates
    World->GetTimerManager().SetTimer(
        CrowdUpdateTimer,
        this,
        &UMassCrowdSubsystem::UpdateCrowdDensityGrid,
        CrowdUpdateFrequency,
        true
    );

    World->GetTimerManager().SetTimer(
        MigrationUpdateTimer,
        this,
        &UMassCrowdSubsystem::ProcessMigrationLogic,
        5.0f, // Check migration every 5 seconds
        true
    );

    World->GetTimerManager().SetTimer(
        DensityAnalysisTimer,
        this,
        &UMassCrowdSubsystem::HandlePredatorPreyInteractions,
        2.0f, // Check predator-prey interactions every 2 seconds
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Crowd system initialized with %d max agents"), MaxSimultaneousAgents);
}

void UMassCrowdSubsystem::SpawnDinosaurHerd(const FVector& Location, int32 HerdSize, TSubclassOf<class ADinosaurAgent> DinosaurClass)
{
    if (HerdSize <= 0 || HerdSize > 1000) // Reasonable limits per herd
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Invalid herd size %d"), HerdSize);
        return;
    }

    // Spawn herd in formation around the central location
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Create circular formation with some randomness
        float Angle = (2.0f * PI * i) / HerdSize;
        float Distance = FMath::RandRange(50.0f, 200.0f);
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector SpawnLocation = Location + SpawnOffset;
        SpawnMassAgents(SpawnLocation, 1, TEXT("DinosaurHerbivore"));
    }

    // Update density grid
    FVector GridKey = GetGridKey(Location);
    CrowdDensityGrid.FindOrAdd(GridKey) += HerdSize;

    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawned dinosaur herd of %d at %s"), HerdSize, *Location.ToString());
}

void UMassCrowdSubsystem::SpawnHumanTribe(const FVector& Location, int32 TribeSize, float TerritoryRadius)
{
    if (TribeSize <= 0 || TribeSize > 200) // Prehistoric tribes were typically 20-150 people
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Invalid tribe size %d"), TribeSize);
        return;
    }

    // Spawn tribe members in small family groups
    int32 GroupSize = FMath::Clamp(TribeSize / 4, 3, 8); // 3-8 people per family group
    int32 NumGroups = FMath::CeilToInt(float(TribeSize) / GroupSize);

    for (int32 GroupIndex = 0; GroupIndex < NumGroups; GroupIndex++)
    {
        // Position family groups around the territory
        float Angle = (2.0f * PI * GroupIndex) / NumGroups;
        float Distance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
        FVector GroupCenter = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        // Spawn individual tribe members in this group
        int32 ActualGroupSize = FMath::Min(GroupSize, TribeSize - (GroupIndex * GroupSize));
        for (int32 i = 0; i < ActualGroupSize; i++)
        {
            FVector MemberOffset = FVector(
                FMath::RandRange(-20.0f, 20.0f),
                FMath::RandRange(-20.0f, 20.0f),
                0.0f
            );
            SpawnMassAgents(GroupCenter + MemberOffset, 1, TEXT("HumanTribal"));
        }
    }

    // Update density grid
    FVector GridKey = GetGridKey(Location);
    CrowdDensityGrid.FindOrAdd(GridKey) += TribeSize;

    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawned human tribe of %d in %d groups at %s"), 
           TribeSize, NumGroups, *Location.ToString());
}

void UMassCrowdSubsystem::SpawnPredatorPack(const FVector& Location, int32 PackSize, TSubclassOf<class APredatorAgent> PredatorClass)
{
    if (PackSize <= 0 || PackSize > 50) // Predator packs are typically small
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Invalid pack size %d"), PackSize);
        return;
    }

    // Spawn predators in loose formation
    for (int32 i = 0; i < PackSize; i++)
    {
        FVector SpawnOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        
        SpawnMassAgents(Location + SpawnOffset, 1, TEXT("PredatorPack"));
    }

    // Update density grid
    FVector GridKey = GetGridKey(Location);
    CrowdDensityGrid.FindOrAdd(GridKey) += PackSize;

    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawned predator pack of %d at %s"), PackSize, *Location.ToString());
}

void UMassCrowdSubsystem::TriggerSeasonalMigration(const FVector& FromRegion, const FVector& ToRegion, float MigrationSpeed)
{
    // Find all herds in the FromRegion
    float SearchRadius = 5000.0f;
    
    // This would integrate with Mass Entity queries to find relevant agents
    // For now, we'll broadcast the migration event
    OnMigrationStarted.Broadcast(FMath::RandRange(1000, 9999), FromRegion, ToRegion);
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Triggered seasonal migration from %s to %s"), 
           *FromRegion.ToString(), *ToRegion.ToString());
}

void UMassCrowdSubsystem::SetWaterSourceLocations(const TArray<FVector>& WaterSources)
{
    WaterSourceLocations = WaterSources;
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Updated water source locations - %d sources"), 
           WaterSources.Num());
}

void UMassCrowdSubsystem::SetFoodSourceDensity(const FVector& Location, float Radius, float FoodDensity)
{
    // This would update the Mass Entity system's environmental data
    // For now, we'll log the food source registration
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Set food density %.2f at %s (radius %.0f)"), 
           FoodDensity, *Location.ToString(), Radius);
}

float UMassCrowdSubsystem::GetCrowdDensityAtLocation(const FVector& Location, float Radius) const
{
    float TotalDensity = 0.0f;
    int32 CellsChecked = 0;

    // Check multiple grid cells within the radius
    int32 CellsToCheck = FMath::CeilToInt(Radius / GridCellSize);
    
    for (int32 X = -CellsToCheck; X <= CellsToCheck; X++)
    {
        for (int32 Y = -CellsToCheck; Y <= CellsToCheck; Y++)
        {
            FVector CellLocation = Location + FVector(X * GridCellSize, Y * GridCellSize, 0.0f);
            FVector GridKey = GetGridKey(CellLocation);
            
            if (const float* Density = CrowdDensityGrid.Find(GridKey))
            {
                TotalDensity += *Density;
                CellsChecked++;
            }
        }
    }

    return CellsChecked > 0 ? TotalDensity / CellsChecked : 0.0f;
}

TArray<FVector> UMassCrowdSubsystem::GetHighTrafficAreas(float MinDensity) const
{
    TArray<FVector> HighTrafficAreas;
    
    for (const auto& DensityPair : CrowdDensityGrid)
    {
        if (DensityPair.Value >= MinDensity)
        {
            HighTrafficAreas.Add(DensityPair.Key);
        }
    }
    
    return HighTrafficAreas;
}

FVector UMassCrowdSubsystem::GetNearestSafeZone(const FVector& FromLocation, float SearchRadius) const
{
    FVector SafestLocation = FromLocation;
    float LowestDensity = GetCrowdDensityAtLocation(FromLocation, 500.0f);
    
    // Sample locations in a grid pattern to find the safest area
    int32 SampleCount = 16;
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        FVector SampleLocation = FromLocation + FVector(
            FMath::Cos(Angle) * SearchRadius,
            FMath::Sin(Angle) * SearchRadius,
            0.0f
        );
        
        float SampleDensity = GetCrowdDensityAtLocation(SampleLocation, 500.0f);
        if (SampleDensity < LowestDensity)
        {
            LowestDensity = SampleDensity;
            SafestLocation = SampleLocation;
        }
    }
    
    return SafestLocation;
}

void UMassCrowdSubsystem::RegisterPredatorThreat(const FVector& ThreatLocation, float ThreatRadius, float ThreatLevel)
{
    // This would trigger Mass Entity behavior changes for nearby herbivores
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Predator threat registered at %s (level %.2f, radius %.0f)"), 
           *ThreatLocation.ToString(), ThreatLevel, ThreatRadius);
    
    // Trigger herd scatter if threat level is high
    if (ThreatLevel > 0.7f)
    {
        TriggerHerdScatter(ThreatLocation, ThreatRadius);
    }
}

void UMassCrowdSubsystem::TriggerHerdScatter(const FVector& ThreatLocation, float ScatterRadius)
{
    // This would send scatter commands to all herbivores within the radius
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: HERD SCATTER triggered at %s (radius %.0f)"), 
           *ThreatLocation.ToString(), ScatterRadius);
}

void UMassCrowdSubsystem::SetPlayerStealthLevel(float StealthLevel)
{
    CurrentPlayerStealthLevel = FMath::Clamp(StealthLevel, 0.0f, 1.0f);
    PlayerDetectionRadius = FMath::Lerp(1500.0f, 200.0f, CurrentPlayerStealthLevel);
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Player stealth level set to %.2f (detection radius %.0f)"), 
           CurrentPlayerStealthLevel, PlayerDetectionRadius);
}

bool UMassCrowdSubsystem::IsPlayerDetectedByCrowd(const FVector& PlayerLocation) const
{
    float CrowdDensity = GetCrowdDensityAtLocation(PlayerLocation, PlayerDetectionRadius);
    float DetectionThreshold = FMath::Lerp(1.0f, 10.0f, 1.0f - CurrentPlayerStealthLevel);
    
    return CrowdDensity > DetectionThreshold;
}

void UMassCrowdSubsystem::UpdateCrowdDensityGrid()
{
    // Decay existing density values over time
    for (auto& DensityPair : CrowdDensityGrid)
    {
        DensityPair.Value *= 0.95f; // 5% decay per update
        
        if (DensityPair.Value < 0.1f)
        {
            DensityPair.Value = 0.0f;
        }
    }
    
    // This would query Mass Entity system for current agent positions
    // and update the density grid accordingly
}

void UMassCrowdSubsystem::ProcessMigrationLogic()
{
    // Check if any herds should start migrating based on:
    // - Season changes
    // - Food/water availability
    // - Population pressure
    
    // This would integrate with the game's time/season system
    UE_LOG(LogTemp, VeryVerbose, TEXT("MassCrowdSubsystem: Processing migration logic"));
}

void UMassCrowdSubsystem::HandlePredatorPreyInteractions()
{
    // Analyze crowd density to determine predator-prey dynamics
    // High herbivore density attracts predators
    // Predator presence causes herbivore scatter
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("MassCrowdSubsystem: Processing predator-prey interactions"));
}

FVector UMassCrowdSubsystem::GetGridKey(const FVector& WorldLocation) const
{
    return FVector(
        FMath::FloorToInt(WorldLocation.X / GridCellSize) * GridCellSize,
        FMath::FloorToInt(WorldLocation.Y / GridCellSize) * GridCellSize,
        0.0f
    );
}

void UMassCrowdSubsystem::SpawnMassAgents(const FVector& Location, int32 Count, const FString& AgentType)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // This would use Mass Entity spawning system
    // For now, we'll log the spawn request
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawning %d %s agents at %s"), 
           Count, *AgentType, *Location.ToString());
}