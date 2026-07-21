#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Perf_BiomePhysicsOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalPhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float BiomeTransitionCost = 0.0f;

    FPerf_BiomePhysicsMetrics()
    {
        TotalPhysicsActors = 0;
        ActivePhysicsActors = 0;
        PhysicsUpdateTime = 0.0f;
        MemoryUsageMB = 0.0f;
        CurrentBiome = EBiomeType::Savanna;
        BiomeTransitionCost = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomeOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxPhysicsActorsPerBiome = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float BiomeTransitionBlendTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptivePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsQualityScale = 1.0f;

    FPerf_BiomeOptimizationSettings()
    {
        MaxPhysicsActorsPerBiome = 200;
        PhysicsLODDistance = 5000.0f;
        BiomeTransitionBlendTime = 2.0f;
        bEnableAdaptivePhysics = true;
        TargetFrameTime = 16.67f;
        PhysicsQualityScale = 1.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_BiomePhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_BiomePhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_BiomePhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePhysics(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_BiomeOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsQuality(float QualityScale);

    // Biome-specific optimization
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OptimizeSavannaPhysics();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OptimizeSwampPhysics();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OptimizeForestPhysics();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OptimizeDesertPhysics();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void OptimizeMountainPhysics();

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float AnalyzeBiomePerformance(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_BiomeOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_BiomePhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsOptimizationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastOptimizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType LastOptimizedBiome = EBiomeType::Savanna;

private:
    // Internal optimization methods
    void UpdateMetrics();
    void ApplyPhysicsLOD();
    void OptimizePhysicsActorsInRange(const FVector& Center, float Radius);
    void AdjustPhysicsQualityForPerformance();
    void CleanupInactivePhysicsActors();
    
    // Performance tracking
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;
    TArray<float> FrameTimeHistory;
    
    // Actor tracking
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TMap<EBiomeType, int32> BiomeActorCounts;
};