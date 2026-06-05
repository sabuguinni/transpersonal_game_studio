#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SharedTypes.h"
#include "Perf_PhysicsIntegrationMasterOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsIntegrationMetrics
{
    GENERATED_BODY()

    // Physics Integration Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float PhysicsIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float CharacterPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float CollisionValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float TerrainPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float CameraPhysicsTime;

    // Performance Counters
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionEnabledActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsBodyCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_PhysicsIntegrationMetrics()
        : PhysicsIntegrationTime(0.0f)
        , CharacterPhysicsTime(0.0f)
        , CollisionValidationTime(0.0f)
        , TerrainPhysicsTime(0.0f)
        , CameraPhysicsTime(0.0f)
        , ActivePhysicsActors(0)
        , CollisionEnabledActors(0)
        , PhysicsBodyCount(0)
        , MemoryUsageMB(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameTime60FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameTime30FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxPhysicsIntegrationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxCollisionValidationTime;

    // Optimization Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActivePhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxCollisionActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsTickOptimizationThreshold;

    // Auto-Optimization Flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bEnableAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bOptimizeCharacterPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bOptimizeCollisionDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bOptimizeTerrainPhysics;

    FPerf_PhysicsOptimizationSettings()
        : TargetFrameTime60FPS(16.67f)
        , TargetFrameTime30FPS(33.33f)
        , MaxPhysicsIntegrationTime(8.0f)
        , MaxCollisionValidationTime(4.0f)
        , MaxActivePhysicsActors(100)
        , MaxCollisionActors(500)
        , MaxMemoryUsageMB(256.0f)
        , PhysicsTickOptimizationThreshold(12.0f)
        , bEnableAutoOptimization(true)
        , bOptimizeCharacterPhysics(true)
        , bOptimizeCollisionDetection(true)
        , bOptimizeTerrainPhysics(true)
    {}
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationMasterOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationMasterOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS INTEGRATION OPTIMIZATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics Integration Optimization")
    void OptimizePhysicsIntegrationMaster();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration Optimization")
    void ValidatePhysicsIntegrationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration Optimization")
    void OptimizeCharacterPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration Optimization")
    void OptimizeCollisionValidationSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration Optimization")
    void OptimizeTerrainPhysicsIntegration();

    // === PERFORMANCE MONITORING ===

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_PhysicsIntegrationMetrics CollectPhysicsIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void GeneratePerformanceReport();

    // === AUTO-OPTIMIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void ExecuteAutoOptimizationCycle();

    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void OptimizeForTargetFramerate(float TargetFrameTime);

    // === INTEGRATION VALIDATION ===

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidatePhysicsIntegrationMasterConnection();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void TestPhysicsIntegrationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void ValidateWalkAroundMilestonePerformance();

    // === PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsIntegrationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIntegrationMasterConnected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float IntegrationValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceOptimizationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

private:
    // Internal optimization state
    float LastOptimizationTime;
    float LastMetricsUpdateTime;
    bool bOptimizationInProgress;
    
    // Performance tracking
    TArray<float> FrameTimeHistory;
    TArray<float> PhysicsTimeHistory;
    
    // Helper methods
    void CollectFrameTimeMetrics();
    void AnalyzePerformanceTrends();
    void ApplyOptimizationAdjustments();
    class ACore_PhysicsIntegrationMaster* FindPhysicsIntegrationMaster();
};