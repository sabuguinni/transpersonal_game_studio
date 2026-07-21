#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perf_OptimizationManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance"),
    Dynamic     UMETA(DisplayName = "Dynamic Optimization")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDebrisObjects = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f;

    FPerf_PerformanceBudget()
    {
        MaxActors = 8000;
        MaxDinosaurs = 150;
        MaxDebrisObjects = 500;
        TargetFrameRate = 60.0f;
        CullingDistance = 10000.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_OptimizationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ZoneRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_OptimizationLevel OptimizationLevel = EPerf_OptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableCulling = true;

    FPerf_OptimizationZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        OptimizationLevel = EPerf_OptimizationLevel::Medium;
        bEnableLODOptimization = true;
        bEnableCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_OptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_OptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Performance Budget Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    FPerf_PerformanceBudget CurrentBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    TArray<FPerf_OptimizationZone> OptimizationZones;

    // Frame Rate Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float CurrentFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float FrameRateThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bEnableFrameRateMonitoring = true;

    // LOD Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance1 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistance3 = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDynamicLOD = true;

    // Culling System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    // Memory Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryUsageMB = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bEnableGarbageCollection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float GCInterval = 30.0f;

public:
    // Performance Monitoring Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const { return CurrentFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimal() const;

    // Budget Management Functions
    UFUNCTION(BlueprintCallable, Category = "Budget")
    void SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget);

    UFUNCTION(BlueprintCallable, Category = "Budget")
    FPerf_PerformanceBudget GetCurrentBudget() const { return CurrentBudget; }

    UFUNCTION(BlueprintCallable, Category = "Budget")
    bool IsWithinBudget() const;

    // LOD Management Functions
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistances(float Distance1, float Distance2, float Distance3);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    int32 GetLODLevel(float Distance) const;

    // Culling Functions
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetCullingDistance(float NewDistance);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    bool ShouldCullActor(AActor* Actor) const;

    // Optimization Zone Functions
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void AddOptimizationZone(const FPerf_OptimizationZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void RemoveOptimizationZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    EPerf_OptimizationLevel GetOptimizationLevelAtLocation(const FVector& Location) const;

    // Memory Management Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    int32 GetCurrentMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsMemoryUsageOptimal() const;

    // Debugging Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPerformanceStats();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogOptimizationZones();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestPerformanceBudget();

private:
    // Internal timer handles
    FTimerHandle FrameRateTimerHandle;
    FTimerHandle LODUpdateTimerHandle;
    FTimerHandle CullingUpdateTimerHandle;
    FTimerHandle GCTimerHandle;

    // Internal functions
    void InitializeOptimizationSystem();
    void UpdatePerformanceMetrics();
    void OptimizeBasedOnFrameRate();
    void CleanupUnusedObjects();
};