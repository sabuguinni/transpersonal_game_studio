// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "MassMovementTypes.h"
#include "MassNavigationTypes.h"
#include "MassLODTypes.h"
#include "MassRepresentationTypes.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "Templates/SubclassOf.h"
#include "MassCrowdSubsystem.generated.h"

class UMassEntityConfigAsset;
class AMassSpawner;
class UMassProcessor;
class UZoneGraphSubsystem;
class UNavigationSystemV1;

UENUM(BlueprintType)
enum class ECrowdType : uint8
{
    Herbivore,      // Peaceful grazing dinosaurs
    Carnivore,      // Hunting predators
    Scavenger,      // Opportunistic feeders
    Territorial,    // Area defenders
    Migratory,      // Seasonal movers
    Pack,           // Group hunters
    Solitary,       // Lone creatures
    Neutral         // Non-aggressive
};

UENUM(BlueprintType)
enum class ECrowdBehavior : uint8
{
    Wandering,      // Random movement
    Grazing,        // Feeding behavior
    Hunting,        // Predatory behavior
    Fleeing,        // Escape behavior
    Socializing,    // Group interaction
    Resting,        // Idle state
    Migrating,      // Long-distance movement
    Territorial,    // Area defense
    Following,      // Leader following
    Investigating   // Curiosity behavior
};

UENUM(BlueprintType)
enum class ECrowdDensity : uint8
{
    Sparse,         // 1-10 agents per area
    Low,            // 10-50 agents per area
    Medium,         // 50-200 agents per area
    High,           // 200-1000 agents per area
    Dense,          // 1000-5000 agents per area
    Massive         // 5000+ agents per area
};

USTRUCT(BlueprintType)
struct FCrowdSpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowdType CrowdType = ECrowdType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowdBehavior DefaultBehavior = ECrowdBehavior::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowdDensity Density = ECrowdDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MinAgents = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxAgents = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TArray<FGameplayTag> BehaviorTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SightRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bUseZoneGraph = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bEnableAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bEnableLOD = true;

    FCrowdSpawnParameters()
    {
        CrowdType = ECrowdType::Herbivore;
        DefaultBehavior = ECrowdBehavior::Wandering;
        Density = ECrowdDensity::Medium;
        MinAgents = 10;
        MaxAgents = 100;
        SpawnRadius = 1000.0f;
        SpawnCenter = FVector::ZeroVector;
        MovementSpeed = 300.0f;
        AvoidanceRadius = 100.0f;
        SightRange = 800.0f;
        bUseZoneGraph = true;
        bEnableAvoidance = true;
        bEnableLOD = true;
    }
};

USTRUCT(BlueprintType)
struct FCrowdZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ECrowdDensity MaxDensity = ECrowdDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<ECrowdType> AllowedCrowdTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<FGameplayTag> ZoneTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 CurrentAgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxAgentCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bPlayerNearby = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float PlayerDistance = 0.0f;

    FCrowdZone()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 2000.0f;
        MaxDensity = ECrowdDensity::Medium;
        CurrentAgentCount = 0;
        MaxAgentCount = 500;
        bIsActive = true;
        bPlayerNearby = false;
        PlayerDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCrowdPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 HighLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MediumLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LowLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OffLODAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CrowdProcessingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveZones = 0;

    FCrowdPerformanceMetrics()
    {
        TotalActiveAgents = 0;
        HighLODAgents = 0;
        MediumLODAgents = 0;
        LowLODAgents = 0;
        OffLODAgents = 0;
        AverageFrameTime = 0.0f;
        CrowdProcessingTime = 0.0f;
        MemoryUsageMB = 0.0f;
        ActiveZones = 0;
    }
};

/**
 * Mass Crowd Subsystem - Manages large-scale crowd simulation using UE5 Mass Entity framework
 * Handles up to 50,000 simultaneous agents with LOD system and performance optimization
 * Integrates with Zone Graph for navigation and provides dynamic density management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMassCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UMassCrowdSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Tick interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;

protected:
    // Core Mass Entity Components
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UZoneGraphSubsystem* ZoneGraphSubsystem;

    UPROPERTY()
    UNavigationSystemV1* NavigationSystem;

    // Crowd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    TArray<FCrowdZone> CrowdZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    int32 MaxTotalAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float ZoneUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float PlayerInfluenceRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float HighLODRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float MediumLODRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Management")
    float LowLODRadius = 5000.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCrowdProcessingTime = 5.0f; // 5ms max per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring = true;

    // Entity Configuration Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    TMap<ECrowdType, TSoftObjectPtr<UMassEntityConfigAsset>> CrowdTypeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Configuration")
    TArray<TObjectPtr<UMassProcessor>> CrowdProcessors;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    FCrowdPerformanceMetrics PerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    TArray<FMassEntityHandle> ActiveCrowdEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float LastZoneUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bIsInitialized = false;

public:
    // Crowd Spawning
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 SpawnCrowd(const FCrowdSpawnParameters& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowd(int32 CrowdID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowds();

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 AddCrowdZone(const FCrowdZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveCrowdZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdZone(int32 ZoneIndex, const FCrowdZone& UpdatedZone);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    FCrowdZone GetCrowdZone(int32 ZoneIndex) const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    TArray<FCrowdZone> GetAllCrowdZones() const { return CrowdZones; }

    // Density Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalDensityMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetZoneDensity(int32 ZoneIndex, ECrowdDensity NewDensity);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetZoneAgentCount(int32 ZoneIndex) const;

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehavior(ECrowdType CrowdType, ECrowdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerCrowdEvent(const FGameplayTag& EventTag, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdFleeTarget(const FVector& ThreatLocation, float ThreatRadius);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OptimizePerformance();

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    FCrowdPerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float HighLOD, float MediumLOD, float LowLOD);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleCrowdDebugDisplay(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawCrowdZones(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowPerformanceStats(bool bEnabled);

protected:
    // Internal Management
    void UpdateCrowdZones(float DeltaTime);
    void UpdatePerformanceMetrics(float DeltaTime);
    void UpdateLODSystem();
    void ProcessCrowdBehaviors(float DeltaTime);
    void HandlePlayerInfluence();

    // Zone Helpers
    FCrowdZone* FindNearestZone(const FVector& Location);
    bool IsLocationInZone(const FVector& Location, const FCrowdZone& Zone) const;
    void BalanceZoneDensities();

    // Performance Helpers
    void AdaptLODBasedOnPerformance();
    void CullDistantAgents();
    void UpdateEntityLOD(const FMassEntityHandle& EntityHandle, float DistanceToPlayer);

    // Mass Entity Helpers
    FMassEntityHandle CreateCrowdEntity(const FCrowdSpawnParameters& SpawnParams, const FVector& SpawnLocation);
    void ConfigureEntityComponents(const FMassEntityHandle& EntityHandle, const FCrowdSpawnParameters& SpawnParams);
    void DestroyEntity(const FMassEntityHandle& EntityHandle);

private:
    // Internal state
    TArray<int32> AvailableCrowdIDs;
    int32 NextCrowdID = 1;
    float GlobalDensityMultiplier = 1.0f;
    bool bDebugDisplayEnabled = false;
    bool bZoneVisualizationEnabled = false;
    bool bPerformanceStatsEnabled = false;

    // Performance tracking
    double LastPerformanceUpdateTime = 0.0;
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameTimeHistorySize = 60;
};