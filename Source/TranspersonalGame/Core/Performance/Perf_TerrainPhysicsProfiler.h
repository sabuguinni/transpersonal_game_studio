#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Perf_TerrainPhysicsProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDeformationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeformationMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionQueries;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurfaceInteractionCost;

    FPerf_TerrainPhysicsMetrics()
    {
        CollisionComplexity = 0.0f;
        ActiveDeformationPoints = 0;
        DeformationMemoryUsage = 0.0f;
        PhysicsUpdateTime = 0.0f;
        CollisionQueries = 0;
        SurfaceInteractionCost = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float SurfaceFriction;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float CollisionCost;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    int32 MaxDeformationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float OptimalLODDistance;

    FPerf_BiomePhysicsProfile()
    {
        BiomeName = TEXT("Unknown");
        SurfaceFriction = 0.8f;
        CollisionCost = 1.0f;
        MaxDeformationPoints = 64;
        OptimalLODDistance = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_TerrainPhysicsProfiler : public AActor
{
    GENERATED_BODY()

public:
    APerf_TerrainPhysicsProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bEnableProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    int32 MaxSamplesPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Profiles")
    TArray<FPerf_BiomePhysicsProfile> BiomeProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeImpact;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryFootprint;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OptimizationLevel;

private:
    float LastProfilingTime;
    TArray<float> FrameTimeSamples;
    TArray<ALandscape*> TrackedLandscapes;
    
    void InitializeBiomeProfiles();
    void UpdateTerrainPhysicsMetrics();
    void AnalyzeCollisionComplexity();
    void MonitorDeformationPerformance();
    void OptimizePhysicsSettings();

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetBiomePerformanceScore(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugTerrainPhysicsPerformance();
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsProfilerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsProfilerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bAutoProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfileRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Local Metrics")
    FPerf_TerrainPhysicsMetrics LocalMetrics;

private:
    void ProfileLocalTerrainPhysics();
    void AnalyzeNearbyCollision();

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetLocalMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfileRadius(float Radius);
};