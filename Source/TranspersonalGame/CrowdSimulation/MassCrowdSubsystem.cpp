// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "MassCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplateRegistry.h"
#include "ZoneGraphSubsystem.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMassCrowd, Log, All);

// Console variables for runtime tuning
static TAutoConsoleVariable<bool> CVarCrowdDebugEnabled(
    TEXT("crowd.debug.enabled"),
    false,
    TEXT("Enable crowd debug visualization"),
    ECVF_Default
);

static TAutoConsoleVariable<int32> CVarMaxCrowdAgents(
    TEXT("crowd.max.agents"),
    50000,
    TEXT("Maximum number of crowd agents"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarCrowdLODHigh(
    TEXT("crowd.lod.high"),
    1000.0f,
    TEXT("High LOD distance for crowd agents"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarCrowdLODMedium(
    TEXT("crowd.lod.medium"),
    2500.0f,
    TEXT("Medium LOD distance for crowd agents"),
    ECVF_Default
);

static TAutoConsoleVariable<float> CVarCrowdLODLow(
    TEXT("crowd.lod.low"),
    5000.0f,
    TEXT("Low LOD distance for crowd agents"),
    ECVF_Default
);

UMassCrowdSubsystem::UMassCrowdSubsystem()
{
    // Initialize default values
    MaxTotalAgents = 50000;
    ZoneUpdateInterval = 1.0f;
    PlayerInfluenceRadius = 3000.0f;
    HighLODRadius = 1000.0f;
    MediumLODRadius = 2500.0f;
    LowLODRadius = 5000.0f;
    TargetFrameTime = 16.67f;
    MaxCrowdProcessingTime = 5.0f;
    bEnableAdaptiveLOD = true;
    bEnablePerformanceMonitoring = true;
    GlobalDensityMultiplier = 1.0f;
    NextCrowdID = 1;
    bIsInitialized = false;
    LastZoneUpdateTime = 0.0f;
    LastPerformanceUpdateTime = 0.0;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(FrameTimeHistorySize);
    for (int32 i = 0; i < FrameTimeHistorySize; ++i)
    {
        FrameTimeHistory.Add(16.67f);
    }
}

void UMassCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogMassCrowd, Log, TEXT("Initializing Mass Crowd Subsystem"));

    // Get required subsystems
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
    NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    if (!MassEntitySubsystem)
    {
        UE_LOG(LogMassCrowd, Error, TEXT("Failed to get MassEntitySubsystem"));
        return;
    }

    // Initialize default crowd zones
    FCrowdZone DefaultZone;
    DefaultZone.ZoneName = TEXT("MainArea");
    DefaultZone.ZoneCenter = FVector::ZeroVector;
    DefaultZone.ZoneRadius = 5000.0f;
    DefaultZone.MaxDensity = ECrowdDensity::Medium;
    DefaultZone.MaxAgentCount = 1000;
    DefaultZone.AllowedCrowdTypes = {ECrowdType::Herbivore, ECrowdType::Neutral};
    CrowdZones.Add(DefaultZone);

    bIsInitialized = true;
    UE_LOG(LogMassCrowd, Log, TEXT("Mass Crowd Subsystem initialized successfully"));
}

void UMassCrowdSubsystem::Deinitialize()
{
    UE_LOG(LogMassCrowd, Log, TEXT("Deinitializing Mass Crowd Subsystem"));

    // Clean up all active entities
    DespawnAllCrowds();

    // Clear references
    MassEntitySubsystem = nullptr;
    ZoneGraphSubsystem = nullptr;
    NavigationSystem = nullptr;

    bIsInitialized = false;
    Super::Deinitialize();
}

bool UMassCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UMassCrowdSubsystem::Tick(float DeltaTime)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        return;
    }

    SCOPE_CYCLE_COUNTER(STAT_MassCrowdTick);

    const double StartTime = FPlatformTime::Seconds();

    // Update performance metrics
    if (bEnablePerformanceMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
    }

    // Update crowd zones periodically
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastZoneUpdateTime >= ZoneUpdateInterval)
    {
        UpdateCrowdZones(DeltaTime);
        LastZoneUpdateTime = CurrentTime;
    }

    // Update LOD system
    if (bEnableAdaptiveLOD)
    {
        UpdateLODSystem();
    }

    // Handle player influence on crowds
    HandlePlayerInfluence();

    // Process crowd behaviors
    ProcessCrowdBehaviors(DeltaTime);

    // Track processing time
    const double EndTime = FPlatformTime::Seconds();
    const float ProcessingTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    PerformanceMetrics.CrowdProcessingTime = ProcessingTime;

    // Adaptive performance management
    if (bEnableAdaptiveLOD && ProcessingTime > MaxCrowdProcessingTime)
    {
        AdaptLODBasedOnPerformance();
    }
}

bool UMassCrowdSubsystem::IsTickable() const
{
    return bIsInitialized && !IsTemplate();
}

TStatId UMassCrowdSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UMassCrowdSubsystem, STATGROUP_Tickables);
}

int32 UMassCrowdSubsystem::SpawnCrowd(const FCrowdSpawnParameters& SpawnParams)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogMassCrowd, Warning, TEXT("Cannot spawn crowd - subsystem not initialized"));
        return -1;
    }

    // Check if we're at capacity
    if (ActiveCrowdEntities.Num() >= MaxTotalAgents)
    {
        UE_LOG(LogMassCrowd, Warning, TEXT("Cannot spawn crowd - at maximum capacity (%d)"), MaxTotalAgents);
        return -1;
    }

    // Calculate actual spawn count based on density
    int32 SpawnCount = FMath::RandRange(SpawnParams.MinAgents, SpawnParams.MaxAgents);
    SpawnCount = FMath::RoundToInt(SpawnCount * GlobalDensityMultiplier);
    SpawnCount = FMath::Min(SpawnCount, MaxTotalAgents - ActiveCrowdEntities.Num());

    if (SpawnCount <= 0)
    {
        return -1;
    }

    const int32 CrowdID = NextCrowdID++;
    int32 SpawnedCount = 0;

    // Spawn entities in a pattern around the spawn center
    for (int32 i = 0; i < SpawnCount; ++i)
    {
        // Generate spawn location
        const float Angle = (2.0f * PI * i) / SpawnCount;
        const float Distance = FMath::RandRange(0.0f, SpawnParams.SpawnRadius);
        const FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        const FVector SpawnLocation = SpawnParams.SpawnCenter + Offset;

        // Create entity
        FMassEntityHandle EntityHandle = CreateCrowdEntity(SpawnParams, SpawnLocation);
        if (EntityHandle.IsValid())
        {
            ActiveCrowdEntities.Add(EntityHandle);
            SpawnedCount++;
        }
    }

    UE_LOG(LogMassCrowd, Log, TEXT("Spawned crowd %d with %d agents (requested %d)"), 
           CrowdID, SpawnedCount, SpawnCount);

    return CrowdID;
}

void UMassCrowdSubsystem::DespawnCrowd(int32 CrowdID)
{
    // For now, despawn all entities (would need better tracking for specific crowd IDs)
    DespawnAllCrowds();
}

void UMassCrowdSubsystem::DespawnAllCrowds()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    UE_LOG(LogMassCrowd, Log, TEXT("Despawning all crowds (%d entities)"), ActiveCrowdEntities.Num());

    // Destroy all active entities
    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            DestroyEntity(EntityHandle);
        }
    }

    ActiveCrowdEntities.Empty();
    PerformanceMetrics.TotalActiveAgents = 0;
}

int32 UMassCrowdSubsystem::AddCrowdZone(const FCrowdZone& NewZone)
{
    const int32 ZoneIndex = CrowdZones.Add(NewZone);
    UE_LOG(LogMassCrowd, Log, TEXT("Added crowd zone '%s' at index %d"), *NewZone.ZoneName, ZoneIndex);
    return ZoneIndex;
}

void UMassCrowdSubsystem::RemoveCrowdZone(int32 ZoneIndex)
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        UE_LOG(LogMassCrowd, Log, TEXT("Removed crowd zone '%s'"), *CrowdZones[ZoneIndex].ZoneName);
        CrowdZones.RemoveAt(ZoneIndex);
    }
}

void UMassCrowdSubsystem::UpdateCrowdZone(int32 ZoneIndex, const FCrowdZone& UpdatedZone)
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        CrowdZones[ZoneIndex] = UpdatedZone;
    }
}

FCrowdZone UMassCrowdSubsystem::GetCrowdZone(int32 ZoneIndex) const
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        return CrowdZones[ZoneIndex];
    }
    return FCrowdZone();
}

void UMassCrowdSubsystem::SetGlobalDensityMultiplier(float Multiplier)
{
    GlobalDensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    UE_LOG(LogMassCrowd, Log, TEXT("Set global density multiplier to %f"), GlobalDensityMultiplier);
}

void UMassCrowdSubsystem::SetZoneDensity(int32 ZoneIndex, ECrowdDensity NewDensity)
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        CrowdZones[ZoneIndex].MaxDensity = NewDensity;
        UE_LOG(LogMassCrowd, Log, TEXT("Set zone %d density to %d"), ZoneIndex, (int32)NewDensity);
    }
}

int32 UMassCrowdSubsystem::GetTotalActiveAgents() const
{
    return ActiveCrowdEntities.Num();
}

int32 UMassCrowdSubsystem::GetZoneAgentCount(int32 ZoneIndex) const
{
    if (CrowdZones.IsValidIndex(ZoneIndex))
    {
        return CrowdZones[ZoneIndex].CurrentAgentCount;
    }
    return 0;
}

void UMassCrowdSubsystem::SetCrowdBehavior(ECrowdType CrowdType, ECrowdBehavior NewBehavior)
{
    // Implementation would update behavior for all entities of the specified type
    UE_LOG(LogMassCrowd, Log, TEXT("Setting behavior %d for crowd type %d"), (int32)NewBehavior, (int32)CrowdType);
}

void UMassCrowdSubsystem::TriggerCrowdEvent(const FGameplayTag& EventTag, const FVector& Location, float Radius)
{
    UE_LOG(LogMassCrowd, Log, TEXT("Triggering crowd event '%s' at %s with radius %f"), 
           *EventTag.ToString(), *Location.ToString(), Radius);

    // Implementation would affect all entities within the radius
    // Could trigger flee, investigate, or other behaviors
}

void UMassCrowdSubsystem::SetCrowdFleeTarget(const FVector& ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogMassCrowd, Log, TEXT("Setting crowd flee target at %s with radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);

    // Implementation would make entities flee from the threat location
}

void UMassCrowdSubsystem::OptimizePerformance()
{
    UE_LOG(LogMassCrowd, Log, TEXT("Optimizing crowd performance"));

    // Cull distant agents
    CullDistantAgents();

    // Balance zone densities
    BalanceZoneDensities();

    // Update LOD distances based on performance
    AdaptLODBasedOnPerformance();
}

void UMassCrowdSubsystem::SetLODDistances(float HighLOD, float MediumLOD, float LowLOD)
{
    HighLODRadius = HighLOD;
    MediumLODRadius = MediumLOD;
    LowLODRadius = LowLOD;

    UE_LOG(LogMassCrowd, Log, TEXT("Set LOD distances - High: %f, Medium: %f, Low: %f"), 
           HighLOD, MediumLOD, LowLOD);
}

void UMassCrowdSubsystem::ToggleCrowdDebugDisplay(bool bEnabled)
{
    bDebugDisplayEnabled = bEnabled;
    CVarCrowdDebugEnabled->Set(bEnabled);
}

void UMassCrowdSubsystem::DrawCrowdZones(bool bEnabled)
{
    bZoneVisualizationEnabled = bEnabled;

    if (bEnabled && GetWorld())
    {
        for (int32 i = 0; i < CrowdZones.Num(); ++i)
        {
            const FCrowdZone& Zone = CrowdZones[i];
            const FColor ZoneColor = Zone.bIsActive ? FColor::Green : FColor::Red;
            
            DrawDebugSphere(GetWorld(), Zone.ZoneCenter, Zone.ZoneRadius, 32, ZoneColor, false, 5.0f, 0, 2.0f);
            DrawDebugString(GetWorld(), Zone.ZoneCenter + FVector(0, 0, Zone.ZoneRadius + 100), 
                          FString::Printf(TEXT("%s (%d/%d)"), *Zone.ZoneName, Zone.CurrentAgentCount, Zone.MaxAgentCount),
                          nullptr, ZoneColor, 5.0f);
        }
    }
}

void UMassCrowdSubsystem::ShowPerformanceStats(bool bEnabled)
{
    bPerformanceStatsEnabled = bEnabled;
}

void UMassCrowdSubsystem::UpdateCrowdZones(float DeltaTime)
{
    // Get player location for zone updates
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    // Update each zone
    for (FCrowdZone& Zone : CrowdZones)
    {
        // Update player distance
        Zone.PlayerDistance = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
        Zone.bPlayerNearby = Zone.PlayerDistance <= PlayerInfluenceRadius;

        // Update zone activity based on player proximity
        Zone.bIsActive = Zone.bPlayerNearby || Zone.PlayerDistance <= Zone.ZoneRadius * 2.0f;

        // Count agents in zone (simplified - would need spatial tracking)
        // Zone.CurrentAgentCount = CountAgentsInZone(Zone);
    }

    PerformanceMetrics.ActiveZones = CrowdZones.Num();
}

void UMassCrowdSubsystem::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time history
    const float CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimeHistory.RemoveAt(0);
    FrameTimeHistory.Add(CurrentFrameTime);

    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    PerformanceMetrics.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();

    // Update agent counts
    PerformanceMetrics.TotalActiveAgents = ActiveCrowdEntities.Num();

    // Estimate memory usage (simplified)
    PerformanceMetrics.MemoryUsageMB = (ActiveCrowdEntities.Num() * sizeof(FMassEntityHandle)) / (1024.0f * 1024.0f);

    // Update LOD counts (would need actual LOD tracking)
    // This is a simplified estimation
    const int32 TotalAgents = PerformanceMetrics.TotalActiveAgents;
    PerformanceMetrics.HighLODAgents = FMath::RoundToInt(TotalAgents * 0.1f);
    PerformanceMetrics.MediumLODAgents = FMath::RoundToInt(TotalAgents * 0.3f);
    PerformanceMetrics.LowLODAgents = FMath::RoundToInt(TotalAgents * 0.4f);
    PerformanceMetrics.OffLODAgents = TotalAgents - PerformanceMetrics.HighLODAgents - 
                                     PerformanceMetrics.MediumLODAgents - PerformanceMetrics.LowLODAgents;
}

void UMassCrowdSubsystem::UpdateLODSystem()
{
    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    // Update LOD for all entities based on distance to player
    for (const FMassEntityHandle& EntityHandle : ActiveCrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            // Calculate distance to player (would need entity position component)
            // UpdateEntityLOD(EntityHandle, DistanceToPlayer);
        }
    }
}

void UMassCrowdSubsystem::ProcessCrowdBehaviors(float DeltaTime)
{
    // This would be handled by Mass processors in a real implementation
    // Here we just track that behaviors are being processed
}

void UMassCrowdSubsystem::HandlePlayerInfluence()
{
    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    // Apply player influence to nearby entities
    // This would modify entity behaviors based on player proximity
}

FCrowdZone* UMassCrowdSubsystem::FindNearestZone(const FVector& Location)
{
    FCrowdZone* NearestZone = nullptr;
    float NearestDistance = MAX_FLT;

    for (FCrowdZone& Zone : CrowdZones)
    {
        const float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestZone = &Zone;
        }
    }

    return NearestZone;
}

bool UMassCrowdSubsystem::IsLocationInZone(const FVector& Location, const FCrowdZone& Zone) const
{
    const float Distance = FVector::Dist(Location, Zone.ZoneCenter);
    return Distance <= Zone.ZoneRadius;
}

void UMassCrowdSubsystem::BalanceZoneDensities()
{
    // Redistribute agents between zones to balance density
    // This would be a complex algorithm to move entities between zones
}

void UMassCrowdSubsystem::AdaptLODBasedOnPerformance()
{
    // If performance is poor, reduce LOD distances
    if (PerformanceMetrics.AverageFrameTime > TargetFrameTime * 1.2f)
    {
        HighLODRadius *= 0.9f;
        MediumLODRadius *= 0.9f;
        LowLODRadius *= 0.9f;
        
        UE_LOG(LogMassCrowd, Log, TEXT("Reducing LOD distances due to performance"));
    }
    // If performance is good, gradually increase LOD distances
    else if (PerformanceMetrics.AverageFrameTime < TargetFrameTime * 0.8f)
    {
        HighLODRadius *= 1.01f;
        MediumLODRadius *= 1.01f;
        LowLODRadius *= 1.01f;
    }
}

void UMassCrowdSubsystem::CullDistantAgents()
{
    // Remove agents that are too far from the player
    // This would help maintain performance
}

void UMassCrowdSubsystem::UpdateEntityLOD(const FMassEntityHandle& EntityHandle, float DistanceToPlayer)
{
    // Update the LOD level of a specific entity based on distance
    // This would modify the entity's representation and update frequency
}

FMassEntityHandle UMassCrowdSubsystem::CreateCrowdEntity(const FCrowdSpawnParameters& SpawnParams, const FVector& SpawnLocation)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }

    // Create a new Mass entity
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    if (EntityHandle.IsValid())
    {
        // Configure entity components based on spawn parameters
        ConfigureEntityComponents(EntityHandle, SpawnParams);
    }

    return EntityHandle;
}

void UMassCrowdSubsystem::ConfigureEntityComponents(const FMassEntityHandle& EntityHandle, const FCrowdSpawnParameters& SpawnParams)
{
    // This would add and configure all necessary components for the entity
    // Including transform, movement, behavior, representation, etc.
}

void UMassCrowdSubsystem::DestroyEntity(const FMassEntityHandle& EntityHandle)
{
    if (EntityHandle.IsValid() && MassEntitySubsystem)
    {
        MassEntitySubsystem->DestroyEntity(EntityHandle);
    }
}