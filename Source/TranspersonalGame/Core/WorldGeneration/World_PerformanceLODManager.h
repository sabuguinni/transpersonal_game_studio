#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "World_PerformanceLODManager.generated.h"

// Forward declarations
class AProceduralWorldGenerator;
class UWorld_BiomeManager;
class UWorld_TerrainGenerator;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float NearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MidDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float FarDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxVegetationInstances = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxRockInstances = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeLODData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome LOD")
    EBiomeType BiomeType = EBiomeType::TropicalRainforest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome LOD")
    int32 ActiveVegetationCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome LOD")
    int32 ActiveRockCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome LOD")
    float AverageFrameTime = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome LOD")
    bool bIsOptimized = false;
};

UENUM(BlueprintType)
enum class EWorld_LODLevel : uint8
{
    Ultra = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Minimal = 4
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(WorldGeneration), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_PerformanceLODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_PerformanceLODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core LOD Management
    UFUNCTION(BlueprintCallable, Category = "World LOD")
    void UpdateLODLevels(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World LOD")
    void SetGlobalLODLevel(EWorld_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "World LOD")
    EWorld_LODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void MonitorFramePerformance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    bool IsPerformanceOptimal() const;

    // Biome-Specific LOD
    UFUNCTION(BlueprintCallable, Category = "Biome LOD")
    void UpdateBiomeLOD(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Biome LOD")
    FWorld_BiomeLODData GetBiomeLODData(EBiomeType BiomeType) const;

    // Vegetation Management
    UFUNCTION(BlueprintCallable, Category = "Vegetation LOD")
    void OptimizeVegetationLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Vegetation LOD")
    void CullDistantVegetation(float CullingDistance);

    // Terrain LOD
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void UpdateTerrainLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void SetTerrainDetailLevel(int32 DetailLevel);

    // Integration with Performance Systems
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void IntegrateWithSurvivalPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void SetPerformanceBudget(float MaxFrameTimeMs);

protected:
    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    FWorld_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    EWorld_LODLevel CurrentLODLevel = EWorld_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bAutoLODEnabled = true;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    float AverageFrameTime = 16.67f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    TArray<float> FrameTimeHistory;

    // Biome LOD Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Performance")
    TMap<EBiomeType, FWorld_BiomeLODData> BiomeLODMap;

    // Component References
    UPROPERTY()
    AProceduralWorldGenerator* WorldGenerator;

    UPROPERTY()
    UWorld_BiomeManager* BiomeManager;

    UPROPERTY()
    UWorld_TerrainGenerator* TerrainGenerator;

private:
    // Internal LOD Management
    void UpdateVegetationLOD(const FVector& PlayerLocation);
    void UpdateRockLOD(const FVector& PlayerLocation);
    void UpdateWaterLOD(const FVector& PlayerLocation);
    
    // Performance Analysis
    void AnalyzePerformanceMetrics();
    void AdjustLODBasedOnPerformance();
    
    // Distance Calculations
    float CalculateDistanceToPlayer(const FVector& Location) const;
    EWorld_LODLevel DetermineLODLevel(float Distance) const;
    
    // Frame Time Tracking
    void UpdateFrameTimeHistory(float DeltaTime);
    float CalculateAverageFrameTime() const;
    
    // LOD Application
    void ApplyLODToVegetation(EWorld_LODLevel LODLevel);
    void ApplyLODToTerrain(EWorld_LODLevel LODLevel);
    void ApplyLODToBiomes(EWorld_LODLevel LODLevel);

    // Cached Player Location
    FVector LastPlayerLocation = FVector::ZeroVector;
    float LastUpdateTime = 0.0f;
    
    // Performance Thresholds
    static constexpr float PERFORMANCE_GOOD_THRESHOLD = 16.0f; // < 16ms = good
    static constexpr float PERFORMANCE_BAD_THRESHOLD = 20.0f;  // > 20ms = bad
    static constexpr int32 FRAME_HISTORY_SIZE = 60; // 1 second at 60fps
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_PerformanceLODSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World LOD Subsystem")
    static UWorld_PerformanceLODSubsystem* Get(const UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "World LOD Subsystem")
    void RegisterLODManager(UWorld_PerformanceLODManager* LODManager);

    UFUNCTION(BlueprintCallable, Category = "World LOD Subsystem")
    void UnregisterLODManager(UWorld_PerformanceLODManager* LODManager);

    UFUNCTION(BlueprintCallable, Category = "World LOD Subsystem")
    void UpdateAllLODManagers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World LOD Subsystem")
    void SetGlobalPerformanceTarget(float TargetFrameTimeMs);

protected:
    UPROPERTY()
    TArray<UWorld_PerformanceLODManager*> RegisteredLODManagers;

    UPROPERTY()
    float GlobalPerformanceTarget = 16.67f;

    UPROPERTY()
    bool bGlobalOptimizationEnabled = true;
};

#include "World_PerformanceLODManager.generated.h"