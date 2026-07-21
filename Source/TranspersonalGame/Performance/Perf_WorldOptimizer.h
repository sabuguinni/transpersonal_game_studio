#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Perf_WorldOptimizer.generated.h"

/**
 * World Performance Optimizer - Global world-level performance optimization
 * Manages LOD systems, culling, streaming, and world-wide performance settings
 * Ensures 60fps on PC and 30fps on console across all world areas
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_WorldOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_WorldOptimizer();

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

public:
    // World Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    float WorldFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    float WorldFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    int32 TotalWorldActors;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    int32 CulledActors;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    float WorldMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "World Performance")
    float DrawCallCount;

    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameRatePC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameRateConsole = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxWorldActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxWorldMemoryMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxDrawCalls = 2000;

    // LOD and Culling Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableAggressiveCulling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDynamicLOD = true;

    // Optimization Status
    UPROPERTY(BlueprintReadOnly, Category = "Optimization Status")
    EPerf_OptimizationLevel CurrentWorldOptimization = EPerf_OptimizationLevel::Balanced;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization Status")
    bool bWorldPerformanceGood = true;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization Status")
    FString WorldPerformanceStatus = "Good";

    UPROPERTY(BlueprintReadOnly, Category = "Optimization Status")
    TArray<FString> WorldOptimizationWarnings;

    // World Optimization Functions
    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void UpdateWorldMetrics();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void CheckWorldPerformance();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void ApplyWorldOptimizations();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void SetWorldOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void OptimizeActorsByDistance(float PlayerX, float PlayerY, float PlayerZ);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void ApplyLODOptimizations();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void CullDistantActors();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void OptimizeDrawCalls();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    float GetCurrentFrameRate() const { return WorldFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    bool IsWorldPerformanceGood() const { return bWorldPerformanceGood; }

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void CleanupUnusedAssets();

    // Platform-specific optimizations
    UFUNCTION(BlueprintCallable, Category = "Platform Performance")
    void ApplyPCOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Platform Performance")
    void ApplyConsoleOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Platform Performance")
    bool IsRunningOnConsole() const;

private:
    // Internal tracking
    float LastOptimizationCheck = 0.0f;
    float OptimizationInterval = 2.0f; // Check every 2 seconds
    TArray<AActor*> CachedWorldActors;
    TArray<AActor*> CachedVisibleActors;
    
    // Performance monitoring
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    float LastFrameRateUpdate = 0.0f;
    
    // Optimization helpers
    void GatherWorldActors();
    void CalculateVisibilityMetrics();
    void UpdateFrameRateMetrics(float DeltaTime);
    void LogWorldPerformance();
    void ApplyPlatformSpecificSettings();
    bool ShouldCullActor(AActor* Actor, const FVector& PlayerLocation) const;
    void SetActorLODLevel(AActor* Actor, int32 LODLevel);
};