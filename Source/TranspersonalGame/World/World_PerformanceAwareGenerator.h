#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Foliage/FoliageInstancedStaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_PerformanceAwareGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_PerformanceLevel : uint8
{
    Ultra      UMETA(DisplayName = "Ultra - 60+ FPS"),
    High       UMETA(DisplayName = "High - 45-60 FPS"),
    Medium     UMETA(DisplayName = "Medium - 30-45 FPS"),
    Low        UMETA(DisplayName = "Low - 20-30 FPS"),
    Minimal    UMETA(DisplayName = "Minimal - <20 FPS")
};

USTRUCT(BlueprintType)
struct FWorld_BiomePerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationInstances = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxRockInstances = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VegetationDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TerrainDetailLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurInstances = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WaterQualityLevel = 1.0f;

    FWorld_BiomePerformanceSettings()
    {
        MaxVegetationInstances = 10000;
        MaxRockInstances = 5000;
        VegetationDensityMultiplier = 1.0f;
        TerrainDetailLevel = 1.0f;
        MaxDinosaurInstances = 50;
        WaterQualityLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VegetationInstanceCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EWorld_PerformanceLevel CurrentPerformanceLevel = EWorld_PerformanceLevel::Ultra;

    FWorld_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        TotalActorsInWorld = 0;
        VegetationInstanceCount = 0;
        DinosaurCount = 0;
        MemoryUsageMB = 0.0f;
        CurrentPerformanceLevel = EWorld_PerformanceLevel::Ultra;
    }
};

/**
 * Performance-aware world generation component that dynamically adjusts world complexity
 * based on current frame rate and system performance to maintain 60fps PC / 30fps console targets
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_PerformanceAwareGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_PerformanceAwareGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EWorld_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorld_PerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    // Biome generation with performance awareness
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeWithPerformanceConstraints(EBiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void OptimizeExistingBiome(EBiomeType BiomeType, FVector Location, float Radius);

    // Vegetation management
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnPerformanceAwareVegetation(EBiomeType BiomeType, FVector Location, int32 MaxInstances);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CullDistantVegetation(float MaxDistance);

    // Dinosaur spawning with performance limits
    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    void SpawnDinosaursWithPerformanceLimits(EBiomeType BiomeType, FVector Location, int32 MaxCount);

    // Terrain optimization
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void OptimizeTerrainLOD(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void AdjustTerrainComplexity(EWorld_PerformanceLevel TargetLevel);

    // Water system optimization
    UFUNCTION(BlueprintCallable, Category = "Water")
    void OptimizeWaterQuality(EWorld_PerformanceLevel PerformanceLevel);

    // Performance settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EWorld_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorld_BiomePerformanceSettings GetBiomePerformanceSettings(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBiomePerformanceSettings(EBiomeType BiomeType, const FWorld_BiomePerformanceSettings& Settings);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void AnalyzeWorldPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GeneratePerformanceReport();

protected:
    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    EWorld_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FWorld_PerformanceMetrics PerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<EBiomeType, FWorld_BiomePerformanceSettings> BiomePerformanceSettings;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 1.0f;

    // Biome coordinates (from brain memory)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TMap<EBiomeType, FVector> BiomeLocations;

    // Performance optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float VegetationCullDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxTotalVegetationInstances = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxTotalDinosaurs = 200;

private:
    // Internal tracking
    float PerformanceUpdateTimer;
    TArray<float> FPSHistory;
    int32 MaxFPSHistorySize = 60; // 1 minute at 1fps sampling

    // Helper functions
    void InitializeBiomeLocations();
    void InitializePerformanceSettings();
    EWorld_PerformanceLevel CalculatePerformanceLevel(float CurrentFPS, float AverageFPS) const;
    void ApplyPerformanceOptimizations(EWorld_PerformanceLevel NewLevel);
    void CollectWorldStatistics();
    FVector GetBiomeLocation(EBiomeType BiomeType) const;
    void LogPerformanceMetrics() const;
};