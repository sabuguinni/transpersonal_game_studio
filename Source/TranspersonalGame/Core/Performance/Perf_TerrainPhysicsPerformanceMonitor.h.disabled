#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_TerrainPhysicsPerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainCollisionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainRaycastTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TerrainCollisionQueries;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsStepTime;

    FPerf_TerrainPhysicsMetrics()
    {
        TerrainCollisionTime = 0.0f;
        ActivePhysicsBodies = 0;
        TerrainRaycastTime = 0.0f;
        TerrainCollisionQueries = 0;
        AverageFrameTime = 0.0f;
        PhysicsStepTime = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_TerrainPhysicsOptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainPhysicsPerformanceAlert, const FPerf_TerrainPhysicsMetrics&, Metrics);

/**
 * Monitors and optimizes terrain physics performance in real-time
 * Integrates with Core_TerrainPhysicsSystem for seamless performance management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_TerrainPhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnTerrainPhysicsPerformanceAlert OnPerformanceAlert;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_TerrainPhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAcceptablePhysicsTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval;

private:
    FTimerHandle MonitoringTimer;
    
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void ApplyOptimizations();
    void OptimizePhysicsBodies();
    void OptimizeCollisionQueries();
    void AdjustPhysicsSettings();
    
    // Performance tracking
    TArray<float> FrameTimeHistory;
    TArray<float> PhysicsTimeHistory;
    int32 MetricsHistorySize;
    
    // Optimization state
    bool bOptimizationsApplied;
    float LastOptimizationTime;
    float OptimizationCooldown;
};