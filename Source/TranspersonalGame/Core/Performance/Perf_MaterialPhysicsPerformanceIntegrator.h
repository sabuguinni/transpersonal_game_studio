#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodyInstance.h"
#include "CollisionQueryParams.h"
#include "../SharedTypes.h"
#include "Perf_MaterialPhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MaterialPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PhysicsSimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MaterialInteractionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float FrameTimeImpact;

    FPerf_MaterialPhysicsMetrics()
        : CollisionDetectionTime(0.0f)
        , PhysicsSimulationTime(0.0f)
        , MaterialInteractionTime(0.0f)
        , ActivePhysicsObjects(0)
        , CollisionChecksPerFrame(0)
        , FrameTimeImpact(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Physics")
    EBiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Physics")
    FPerf_MaterialPhysicsMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Physics")
    float OptimalLODDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Physics")
    int32 MaxPhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Physics")
    bool bRequiresOptimization;

    FPerf_BiomePhysicsProfile()
        : BiomeType(EBiomeType::Savana)
        , OptimalLODDistance(5000.0f)
        , MaxPhysicsObjects(100)
        , bRequiresOptimization(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_MaterialPhysicsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_MaterialPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    FPerf_MaterialPhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    TArray<FPerf_BiomePhysicsProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float PerformanceThreshold;

private:
    float LastUpdateTime;
    TArray<AActor*> MonitoredActors;
    TMap<EBiomeType, int32> BiomePhysicsObjectCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_MaterialPhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void OptimizePhysicsForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void SetLODDistanceForBiome(EBiomeType BiomeType, float Distance);

    UFUNCTION(CallInEditor, Category = "Performance Testing")
    void RunPerformanceTest();

    UFUNCTION(CallInEditor, Category = "Performance Testing")
    void GeneratePerformanceReport();

protected:
    void UpdateMetrics();
    void AnalyzeBiomePerformance();
    void ApplyOptimizations();
    void CollectPhysicsData();
    void UpdateBiomeProfiles();
    EBiomeType GetBiomeTypeAtLocation(const FVector& Location) const;
    void OptimizePhysicsObjects(const TArray<AActor*>& Actors);
    void AdjustLODSettings(AActor* Actor, float Distance);
    bool ShouldOptimizeActor(AActor* Actor, float Distance) const;
};