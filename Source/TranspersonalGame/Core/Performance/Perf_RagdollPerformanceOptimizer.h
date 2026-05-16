#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perf_RagdollPerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    float EstimatedCPUCost = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    float EstimatedMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    bool bOptimizationNeeded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    float AverageDistanceToPlayer = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxActiveRagdolls = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency = 0.5f;
};

UENUM(BlueprintType)
enum class EPerf_RagdollLODLevel : uint8
{
    HighDetail     UMETA(DisplayName = "High Detail"),
    MediumDetail   UMETA(DisplayName = "Medium Detail"),
    LowDetail      UMETA(DisplayName = "Low Detail"),
    Culled         UMETA(DisplayName = "Culled")
};

/**
 * Performance optimizer specifically for ragdoll physics systems
 * Manages LOD levels, culling, and performance metrics for ragdoll actors
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_RagdollPerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_RagdollPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    FPerf_RagdollMetrics AnalyzeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void OptimizeRagdollActors();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void UpdateRagdollLOD();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll LOD")
    EPerf_RagdollLODLevel CalculateRagdollLOD(AActor* RagdollActor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll LOD")
    void ApplyRagdollLOD(AActor* RagdollActor, EPerf_RagdollLODLevel LODLevel);

    // Culling and Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void CullDistantRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void LimitActiveRagdolls();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetRagdollFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugRagdollPerformance();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_RagdollLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableDistanceCulling = true;

    // Runtime Data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    FPerf_RagdollMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<TWeakObjectPtr<AActor>> TrackedRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    float LastOptimizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    float AverageFrameTime = 0.0f;

private:
    // Internal Methods
    void FindRagdollActors();
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    AActor* GetPlayerActor();
    
    // Timer Handles
    FTimerHandle OptimizationTimerHandle;
    FTimerHandle MetricsUpdateTimerHandle;

    // Performance Tracking
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameTimeHistory = 60;
    float LastFrameTime = 0.0f;
};