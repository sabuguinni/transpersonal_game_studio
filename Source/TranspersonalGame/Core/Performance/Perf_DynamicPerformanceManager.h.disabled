#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Perf_DynamicPerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra     UMETA(DisplayName = "Ultra"),
    High      UMETA(DisplayName = "High"), 
    Medium    UMETA(DisplayName = "Medium"),
    Low       UMETA(DisplayName = "Low"),
    Potato    UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        GPUFrameTime = 8.0f;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxViewDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxPhysicsObjects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ShadowResolutionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bOptimizePhysics = true;

    FPerf_OptimizationSettings()
    {
        MaxViewDistance = 15000.0f;
        LODDistanceScale = 1.0f;
        MaxPhysicsObjects = 100;
        ShadowResolutionScale = 1.0f;
        bEnableDistanceCulling = true;
        bOptimizePhysics = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_DynamicPerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_DynamicPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Dynamic optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeScene();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeLighting();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyLODOptimizations();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageMB() const;

    // Console commands
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ExecutePerformanceCommand(const FString& Command);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceLogging = true;

private:
    float LastUpdateTime = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    
    TArray<float> FrameTimeHistory;
    static const int32 MaxFrameHistorySize = 60;

    void ApplyPerformanceSettings();
    void UpdateFrameTimeHistory(float DeltaTime);
    float CalculateAverageFrameTime() const;
    void CheckPerformanceThresholds();
    void AutoOptimizeIfNeeded();
};