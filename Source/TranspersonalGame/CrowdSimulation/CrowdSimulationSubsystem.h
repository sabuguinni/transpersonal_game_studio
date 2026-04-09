// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityManager.h"
#include "MassProcessingTypes.h"
#include "MassEntityFragments.h"
#include "Engine/World.h"
#include "Templates/SubclassOf.h"
#include "CrowdSimulationSubsystem.generated.h"

// Forward declarations
class UMassEntityConfigAsset;
class UMassProcessor;
class AMassSpawner;

/**
 * Dinosaur species configuration data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpeciesID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> HighDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> MediumDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> LowDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseAggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseHerdInstinct = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseTerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;

    // Physical attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseBodySize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDetectionRange = 2000.0f;

    // Population parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPopulation = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> PreferredBiomes;
};

/**
 * Spawn zone configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BiomeType = 0; // 0=Forest, 1=Plains, 2=Swamp, 3=Desert

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> AllowedSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEntitiesInZone = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActiveSpawnZone = true;
};

/**
 * Performance metrics tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSimulationMetrics
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 TotalActiveEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 HighDetailEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MediumDetailEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 LowDetailEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CulledEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AverageFrameTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float ProcessingTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RenderingTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MemoryUsageMB = 0;
};

/**
 * Main subsystem for managing crowd simulation of dinosaurs
 * Handles up to 50,000 simultaneous entities using Mass Entity framework
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Main simulation control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    // Entity spawning and management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(int32 SpeciesID, FVector Location, int32 HerdSize = 10, float SpreadRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnSolitaryDinosaur(int32 SpeciesID, FVector Location, float TerritoryRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnEntitiesInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllEntities();

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void AddSpawnZone(const FDinosaurSpawnZone& SpawnZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveSpawnZone(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateSpawnZones();

    // Species configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterDinosaurSpecies(const FDinosaurSpeciesConfig& SpeciesConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FDinosaurSpeciesConfig GetSpeciesConfig(int32 SpeciesID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FDinosaurSpeciesConfig> GetAllSpeciesConfigs() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowdSimulationMetrics GetSimulationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPerformanceTarget(float TargetFrameTime, int32 MaxEntities);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetQualityLevel(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DrawDebugInfo(bool bShowHerds, bool bShowNavigation, bool bShowLOD);

    // Event handling
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEntitySpawned, int32, SpeciesID, FVector, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEntityDespawned, int32, SpeciesID, FVector, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, float, FrameTime);

    UPROPERTY(BlueprintAssignable)
    FOnEntitySpawned OnEntitySpawned;

    UPROPERTY(BlueprintAssignable)
    FOnEntityDespawned OnEntityDespawned;

    UPROPERTY(BlueprintAssignable)
    FOnPerformanceWarning OnPerformanceWarning;

protected:
    // Internal systems
    void InitializeMassEntitySystem();
    void InitializeProcessors();
    void InitializeDefaultSpecies();
    void InitializeSpawnZones();

    // Update loops
    void UpdateSimulation(float DeltaTime);
    void UpdatePerformanceMetrics();
    void UpdateQualityScaling();

    // Entity management
    FMassEntityHandle CreateDinosaurEntity(int32 SpeciesID, FVector Location, FRotator Rotation);
    void DestroyDinosaurEntity(FMassEntityHandle EntityHandle);
    void UpdateEntityLOD(FMassEntityHandle EntityHandle, float DistanceToPlayer);

    // Spawn zone processing
    void ProcessSpawnZones(float DeltaTime);
    void SpawnEntitiesInZone(const FDinosaurSpawnZone& Zone, float DeltaTime);
    void CullEntitiesInZone(const FDinosaurSpawnZone& Zone);

    // Performance optimization
    void OptimizePerformance();
    void BalanceEntityCounts();
    void AdjustUpdateFrequencies();

private:
    // Core systems
    UPROPERTY()
    TObjectPtr<UMassEntityManager> EntityManager;

    UPROPERTY()
    TArray<TObjectPtr<UMassProcessor>> Processors;

    // Configuration data
    UPROPERTY(EditAnywhere, Category = "Configuration")
    TMap<int32, FDinosaurSpeciesConfig> SpeciesConfigs;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    TArray<FDinosaurSpawnZone> SpawnZones;

    // Runtime state
    UPROPERTY(VisibleAnywhere, Category = "Runtime")
    bool bIsSimulationActive = false;

    UPROPERTY(VisibleAnywhere, Category = "Runtime")
    bool bIsSimulationPaused = false;

    UPROPERTY(VisibleAnywhere, Category = "Runtime")
    FCrowdSimulationMetrics CurrentMetrics;

    // Performance settings
    UPROPERTY(EditAnywhere, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxActiveEntities = 50000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 CurrentQualityLevel = 2; // High by default

    UPROPERTY(EditAnywhere, Category = "Performance")
    float PerformanceScalingFactor = 0.8f;

    // Debug settings
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugVisualizationEnabled = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowHerdDebug = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowNavigationDebug = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowLODDebug = false;

    // Timing
    float LastUpdateTime = 0.0f;
    float AccumulatedDeltaTime = 0.0f;
    int32 FrameCounter = 0;

    // Entity tracking
    TMap<FMassEntityHandle, int32> EntityToSpeciesMap;
    TMap<int32, TArray<FMassEntityHandle>> SpeciesToEntitiesMap;
    TArray<FMassEntityHandle> ActiveEntities;

    // Herd tracking
    TMap<int32, TArray<FMassEntityHandle>> HerdMembership;
    int32 NextHerdID = 1;
};