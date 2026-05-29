#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsPerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsPerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float CPUUsagePercent;

    FCore_PhysicsMetrics()
        : PhysicsFrameTime(0.0f)
        , ActiveRigidBodies(0)
        , CollisionChecks(0)
        , MemoryUsageMB(0.0f)
        , CPUUsagePercent(0.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsPerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsPerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FCore_PhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPerformanceLevel(ECore_PhysicsPerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    ECore_PhysicsPerformanceLevel GetCurrentPerformanceLevel() const;

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsSubsteps(int32 NumSubsteps);

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance", CallInEditor)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance", CallInEditor)
    void ResetMetrics();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    ECore_PhysicsPerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Current Metrics")
    FCore_PhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    float TimeSinceLastCheck;

private:
    // Internal monitoring functions
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void ApplyPerformanceSettings();
    
    // Performance data collection
    void CollectPhysicsMetrics();
    void CollectMemoryMetrics();
    void CollectCPUMetrics();

    // Optimization helpers
    void OptimizeForLowEnd();
    void OptimizeForMedium();
    void OptimizeForHighEnd();
    void OptimizeForUltra();

    // Metrics storage
    TArray<float> FrameTimeHistory;
    TArray<int32> RigidBodyHistory;
    float AverageFrameTime;
    int32 MaxHistorySize;
};