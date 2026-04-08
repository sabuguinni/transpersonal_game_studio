#include "DinosaurCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

void UDinosaurCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get Mass Entity subsystem references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurCrowdSubsystem: Failed to get Mass subsystems"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Initialized successfully"));
}

void UDinosaurCrowdSubsystem::Deinitialize()
{
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CrowdUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(PerformanceOptimizationTimer);
    }

    // Clear all dinosaur zones
    DinosaurZones.Empty();
    CurrentDinosaurCount = 0;

    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Deinitialized"));
}

bool UDinosaurCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UDinosaurCrowdSubsystem::InitializeCrowdSimulation()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurCrowdSubsystem: No valid world for initialization"));
        return;
    }

    // Start periodic updates
    GetWorld()->GetTimerManager().SetTimer(
        CrowdUpdateTimer,
        this,
        &UDinosaurCrowdSubsystem::ProcessDinosaurBehaviors,
        UpdateFrequency,
        true
    );

    // Start performance optimization timer (every 5 seconds)
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceOptimizationTimer,
        this,
        &UDinosaurCrowdSubsystem::OptimizePerformance,
        5.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Crowd simulation initialized with max %d dinosaurs"), MaxDinosaurCount);
}

void UDinosaurCrowdSubsystem::SpawnDinosaurHerd(FVector Location, int32 Count, TSubclassOf<class ADinosaur> DinosaurClass)
{
    if (!MassSpawnerSubsystem || !DinosaurClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCrowdSubsystem: Cannot spawn herd - invalid spawner or class"));
        return;
    }

    // Check if we're at capacity
    if (CurrentDinosaurCount + Count > MaxDinosaurCount)
    {
        int32 AvailableSlots = MaxDinosaurCount - CurrentDinosaurCount;
        if (AvailableSlots <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCrowdSubsystem: Cannot spawn herd - at maximum capacity"));
            return;
        }
        Count = AvailableSlots;
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCrowdSubsystem: Reducing herd size to %d due to capacity limits"), Count);
    }

    // Spawn dinosaurs in a scattered formation around the location
    for (int32 i = 0; i < Count; i++)
    {
        // Create random offset within a reasonable radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(50.0f, 300.0f); // 50cm to 3m spread
        
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        // TODO: Integrate with Mass spawning system
        // For now, log the spawn request
        UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Spawning dinosaur %d at location %s"), 
               i + 1, *SpawnLocation.ToString());
    }

    CurrentDinosaurCount += Count;
    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Spawned herd of %d dinosaurs. Total count: %d"), 
           Count, CurrentDinosaurCount);
}

void UDinosaurCrowdSubsystem::RegisterDinosaurZone(FVector Center, float Radius, EDinosaurZoneType ZoneType)
{
    FDinosaurZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.ZoneType = ZoneType;
    NewZone.bActiveZone = true;

    // Set default max dinosaurs based on zone type
    switch (ZoneType)
    {
    case EDinosaurZoneType::Grazing:
        NewZone.MaxDinosaurs = 200;
        break;
    case EDinosaurZoneType::Watering:
        NewZone.MaxDinosaurs = 150;
        break;
    case EDinosaurZoneType::Nesting:
        NewZone.MaxDinosaurs = 50;
        break;
    case EDinosaurZoneType::Hunting:
        NewZone.MaxDinosaurs = 30;
        break;
    case EDinosaurZoneType::Migration:
        NewZone.MaxDinosaurs = 500;
        break;
    case EDinosaurZoneType::Shelter:
        NewZone.MaxDinosaurs = 100;
        break;
    }

    DinosaurZones.Add(NewZone);

    FString ZoneTypeName;
    switch (ZoneType)
    {
    case EDinosaurZoneType::Grazing: ZoneTypeName = TEXT("Grazing"); break;
    case EDinosaurZoneType::Watering: ZoneTypeName = TEXT("Watering"); break;
    case EDinosaurZoneType::Nesting: ZoneTypeName = TEXT("Nesting"); break;
    case EDinosaurZoneType::Hunting: ZoneTypeName = TEXT("Hunting"); break;
    case EDinosaurZoneType::Migration: ZoneTypeName = TEXT("Migration"); break;
    case EDinosaurZoneType::Shelter: ZoneTypeName = TEXT("Shelter"); break;
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Registered %s zone at %s with radius %.1f"), 
           *ZoneTypeName, *Center.ToString(), Radius);
}

void UDinosaurCrowdSubsystem::SetCrowdDensity(float DensityMultiplier)
{
    DensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 2.0f);
    
    int32 NewMaxCount = FMath::RoundToInt(50000 * DensityMultiplier);
    MaxDinosaurCount = NewMaxCount;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Crowd density set to %.2f (max dinosaurs: %d)"), 
           DensityMultiplier, MaxDinosaurCount);
}

int32 UDinosaurCrowdSubsystem::GetActiveDinosaurCount() const
{
    return CurrentDinosaurCount;
}

void UDinosaurCrowdSubsystem::TriggerHerdBehavior(FVector ThreatLocation, float ThreatRadius, EDinosaurThreatLevel ThreatLevel)
{
    FString ThreatLevelName;
    switch (ThreatLevel)
    {
    case EDinosaurThreatLevel::Low: ThreatLevelName = TEXT("Low"); break;
    case EDinosaurThreatLevel::Medium: ThreatLevelName = TEXT("Medium"); break;
    case EDinosaurThreatLevel::High: ThreatLevelName = TEXT("High"); break;
    case EDinosaurThreatLevel::Extreme: ThreatLevelName = TEXT("Extreme"); break;
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Triggering %s threat response at %s (radius: %.1f)"), 
           *ThreatLevelName, *ThreatLocation.ToString(), ThreatRadius);

    // TODO: Implement Mass Entity query to find all dinosaurs within threat radius
    // TODO: Apply appropriate behavioral response based on threat level
    // For now, just log the event
}

void UDinosaurCrowdSubsystem::UpdateGrazingAreas(const TArray<FVector>& NewGrazingLocations)
{
    // Remove existing grazing zones
    DinosaurZones.RemoveAll([](const FDinosaurZone& Zone) {
        return Zone.ZoneType == EDinosaurZoneType::Grazing;
    });

    // Add new grazing zones
    for (const FVector& Location : NewGrazingLocations)
    {
        RegisterDinosaurZone(Location, 800.0f, EDinosaurZoneType::Grazing);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Updated %d grazing areas"), NewGrazingLocations.Num());
}

void UDinosaurCrowdSubsystem::UpdateCrowdLOD()
{
    // TODO: Implement LOD system based on distance from player
    // Near LOD: Full behavior simulation
    // Medium LOD: Simplified behavior
    // Far LOD: Position updates only
}

void UDinosaurCrowdSubsystem::ProcessDinosaurBehaviors()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Track frame time for performance monitoring
    float StartTime = FPlatformTime::Seconds();

    // TODO: Process Mass Entity queries for dinosaur behaviors
    // - Update grazing behaviors
    // - Process herd movement
    // - Handle predator-prey interactions
    // - Update zone occupancy

    UpdateCrowdLOD();

    float EndTime = FPlatformTime::Seconds();
    CurrentFrameTime = EndTime - StartTime;

    // Log performance warning if frame time is too high
    if (CurrentFrameTime > 0.016f) // 60 FPS threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCrowdSubsystem: High frame time detected: %.4f seconds"), CurrentFrameTime);
    }
}

void UDinosaurCrowdSubsystem::OptimizePerformance()
{
    // Performance optimization logic
    if (CurrentFrameTime > 0.020f) // If below 50 FPS
    {
        // Reduce crowd density
        float ReductionFactor = 0.9f;
        int32 NewMaxCount = FMath::RoundToInt(MaxDinosaurCount * ReductionFactor);
        
        if (NewMaxCount < CurrentDinosaurCount)
        {
            // TODO: Remove furthest dinosaurs from player
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCrowdSubsystem: Reducing crowd size for performance (from %d to %d)"), 
                   CurrentDinosaurCount, NewMaxCount);
        }
        
        MaxDinosaurCount = NewMaxCount;
    }
    else if (CurrentFrameTime < 0.010f && MaxDinosaurCount < 50000) // If above 100 FPS and below max
    {
        // Increase crowd density
        float IncreaseFactor = 1.05f;
        MaxDinosaurCount = FMath::Min(50000, FMath::RoundToInt(MaxDinosaurCount * IncreaseFactor));
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurCrowdSubsystem: Performance good, increased max crowd size to %d"), MaxDinosaurCount);
    }
}