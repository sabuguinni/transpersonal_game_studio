#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassDinosaurFragments.h"
#include "MassAvoidanceFragments.h"
#include "MassNavigationFragments.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationCore.generated.h"

/**
 * CROWD SIMULATION CORE SYSTEM
 * 
 * Manages up to 50,000 simultaneous dinosaur agents using Mass Entity Framework
 * Creates emergent ecosystem behaviors through individual agent interactions
 * 
 * Core Philosophy:
 * - Each dinosaur lives its own life independent of the player
 * - Behaviors emerge from simple rules, not scripted events
 * - The world feels alive even when the player isn't watching
 * 
 * Performance Targets:
 * - 50,000 agents at 60fps on high-end PC
 * - 25,000 agents at 30fps on console
 * - Scalable LOD system based on distance to player
 * 
 * Agent #13 — Crowd & Traffic Simulation
 * Transpersonal Game Studio — March 2026
 */

UENUM(BlueprintType)
enum class ECrowdDensityLevel : uint8
{
    Sparse = 0,     // 100-500 agents per km²
    Light,          // 500-1500 agents per km²
    Medium,         // 1500-5000 agents per km²
    Dense,          // 5000-15000 agents per km²
    Massive         // 15000+ agents per km² (migration events)
};

UENUM(BlueprintType)
enum class EEcosystemEvent : uint8
{
    None = 0,
    Migration,      // Seasonal movement of herds
    Stampede,       // Panic response to predators
    Feeding,        // Gathering at food sources
    Watering,       // Gathering at water sources
    Mating,         // Breeding season behaviors
    Storm,          // Weather-driven shelter seeking
    Drought,        // Resource scarcity response
    Predation       // Active hunting events
};

UENUM(BlueprintType)
enum class ECrowdLODLevel : uint8
{
    FullSimulation = 0,     // Full AI, physics, animation (0-500m from player)
    ReducedSimulation,      // Simplified AI, basic physics (500-2000m)
    BasicBehavior,          // State machine only (2000-5000m)
    StaticRepresentation,   // No AI, visual only (5000-10000m)
    Culled                  // Not rendered or simulated (10000m+)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdDensityZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdDensityLevel DensityLevel = ECrowdDensityLevel::Medium;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PreferredSpecies;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResourceAbundance = 1.0f; // Affects carrying capacity
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWaterSource = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSafeZone = true; // Low predator activity
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalModifier = 1.0f; // Changes with time
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RouteWidth = 2000.0f; // Corridor width
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExpectedHerdSize = 100;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 300.0f; // cm/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTiming = 0.0f; // 0-1, when in year this occurs
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActiveAgents = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 FullSimulationAgents = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 ReducedSimulationAgents = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 BasicBehaviorAgents = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 StaticAgents = 0;
    
    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float CrowdSimulationCost = 0.0f; // ms per frame
    
    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage = 0.0f; // MB
    
    UPROPERTY(BlueprintReadOnly)
    bool bPerformanceWarning = false;
};

/**
 * Core crowd simulation manager
 * Orchestrates all crowd behaviors and manages performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core simulation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxSimultaneousAgents = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SimulationRadius = 20000.0f; // cm from player
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableAdaptiveLOD = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableEcosystemEvents = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float EcosystemEventFrequency = 0.1f; // Events per minute
    
    // Density zones configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density Management")
    TArray<FCrowdDensityZone> DensityZones;
    
    // Migration system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float GlobalMigrationScale = 1.0f;
    
    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCrowdPerformanceMetrics PerformanceMetrics;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps target
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoAdjustQuality = true;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdInZone(const FCrowdDensityZone& Zone);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerEcosystemEvent(EEcosystemEvent EventType, FVector Location, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartMigration(int32 RouteIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODLevels();
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCrowdPerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }
    
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

private:
    // Internal systems
    void UpdateDensityZones(float DeltaTime);
    void ProcessEcosystemEvents(float DeltaTime);
    void ManageMigrations(float DeltaTime);
    void OptimizePerformance(float DeltaTime);
    void UpdatePerformanceMetrics(float DeltaTime);
    
    // LOD management
    void UpdateAgentLOD(FMassEntityHandle Entity, float DistanceToPlayer);
    ECrowdLODLevel CalculateLODLevel(float DistanceToPlayer) const;
    
    // Event system
    void ProcessMigrationEvent(FVector Location, float Intensity);
    void ProcessStampedeEvent(FVector Location, float Intensity);
    void ProcessFeedingEvent(FVector Location, float Intensity);
    void ProcessWateringEvent(FVector Location, float Intensity);
    
    // Performance optimization
    void AdaptiveQualityAdjustment();
    void CullDistantAgents();
    void BalanceSimulationLoad();

private:
    // Runtime state
    float CurrentSimulationTime = 0.0f;
    float LastEventTime = 0.0f;
    int32 CurrentQualityLevel = 2; // Default to High
    bool bSimulationInitialized = false;
    
    // Performance tracking
    TArray<float> FrameTimeHistory;
    float PerformanceUpdateTimer = 0.0f;
    
    // Active events
    TArray<EEcosystemEvent> ActiveEvents;
    TMap<EEcosystemEvent, float> EventTimers;
    
    // Cached references
    UPROPERTY()
    class UMassDinosaurSubsystem* MassSubsystem;
    
    UPROPERTY()
    class APawn* PlayerPawn;
};

/**
 * Specialized processor for crowd density management
 * Handles dynamic spawning/despawning based on player proximity and performance
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdDensityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassCrowdDensityProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery DensityQuery;
    
    UPROPERTY(EditAnywhere, Category = "Density Management")
    float DensityUpdateInterval = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Density Management")
    float SpawnRadius = 15000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Density Management")
    float DespawnRadius = 25000.0f;
    
    float LastDensityUpdate = 0.0f;
};

/**
 * Processor for ecosystem-wide events (migrations, stampedes, etc.)
 * Creates emergent large-scale behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UMassEcosystemEventProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassEcosystemEventProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EventQuery;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Events")
    float EventInfluenceRadius = 10000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Events")
    float StampedeSpeed = 1500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Events")
    float MigrationSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Events")
    float EventDecayRate = 0.1f; // Events fade over time
};

/**
 * Fragment for crowd-specific data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassCrowdFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdLODLevel LODLevel = ECrowdLODLevel::FullSimulation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInDensityZone = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DensityZoneID = -1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocalDensity = 0.0f; // Agents per m²
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectedByEvent = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEcosystemEvent CurrentEvent = EEcosystemEvent::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventInfluence = 0.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EventTarget = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastLODUpdate = 0.0f;
};