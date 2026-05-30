#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/LandscapeProxy.h"
#include "Perf_TerrainPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_TerrainQuality : uint8
{
    Ultra UMETA(DisplayName = "Ultra"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low"),
    Potato UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct FPerf_TerrainLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    int32 MaxLODLevel = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    float ComponentScreenSizeToUseSubSections = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    float LOD0ScreenSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    float LOD0Distribution = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain LOD")
    float LODBias = 0.0f;
};

USTRUCT(BlueprintType)
struct FPerf_TerrainCullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Culling")
    float MaxDrawDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Culling")
    float OcclusionCullingThreshold = 0.1f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_TerrainPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_TerrainQuality TerrainQuality = EPerf_TerrainQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_TerrainLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_TerrainCullingSettings CullingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float TerrainRenderTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    int32 VisibleTerrainComponents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float TerrainMemoryUsageMB = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void OptimizeTerrainForTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void SetTerrainQuality(EPerf_TerrainQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void ApplyLODSettings(const FPerf_TerrainLODSettings& NewLODSettings);

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void ApplyCullingSettings(const FPerf_TerrainCullingSettings& NewCullingSettings);

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void UpdateTerrainPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    float GetTerrainRenderCost() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void OptimizeTerrainMaterials();

    UFUNCTION(BlueprintCallable, Category = "Terrain Performance")
    void EnableDynamicLODScaling(bool bEnable);

private:
    UPROPERTY()
    TArray<ALandscapeProxy*> CachedLandscapeActors;

    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 1.0f;
    bool bDynamicLODEnabled = true;

    void FindAllLandscapeActors();
    void ApplyQualitySettings(EPerf_TerrainQuality Quality);
    void UpdateLODBasedOnPerformance();
    void MonitorTerrainMemoryUsage();
    void OptimizeTerrainTessellation();
};