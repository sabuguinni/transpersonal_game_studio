#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "PhysicsOptimizationComponent.generated.h"

class UPhysicsSystemManager;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EPhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Aggressive  UMETA(DisplayName = "Aggressive Optimization")
};

UENUM(BlueprintType)
enum class EPhysicsLODState : uint8
{
    FullDetail      UMETA(DisplayName = "Full Detail"),
    ReducedDetail   UMETA(DisplayName = "Reduced Detail"),
    SimplifiedPhysics UMETA(DisplayName = "Simplified Physics"),
    StaticOnly      UMETA(DisplayName = "Static Only"),
    Disabled        UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float DistanceThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    EPhysicsLODState LODState = EPhysicsLODState::FullDetail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CollisionSimplificationFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bDisableComplexCollision = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bReduceTickFrequency = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float TickFrequencyMultiplier = 1.0f;

    FPhysicsLODSettings()
    {
        DistanceThreshold = 1000.0f;
        LODState = EPhysicsLODState::FullDetail;
        CollisionSimplificationFactor = 1.0f;
        bDisableComplexCollision = false;
        bReduceTickFrequency = false;
        TickFrequencyMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FPhysicsOptimizationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 SleepingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 OptimizedBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CollisionQueryTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CollisionQueries = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsage = 0.0f;

    FPhysicsOptimizationMetrics()
    {
        ActivePhysicsBodies = 0;
        SleepingBodies = 0;
        OptimizedBodies = 0;
        PhysicsUpdateTime = 0.0f;
        CollisionQueryTime = 0.0f;
        CollisionQueries = 0;
        MemoryUsage = 0.0f;
    }
};

/**
 * Component responsible for optimizing physics performance through LOD systems,
 * distance-based culling, and adaptive quality settings.
 * Ensures 60fps on PC and 30fps on console by managing physics complexity.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsOptimizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsOptimizationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPhysicsOptimizationLevel OptimizationLevel = EPhysicsOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceBasedLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePerformanceBasedLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableMemoryOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsTime = 8.0f; // Max ms per frame for physics

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<FPhysicsLODSettings> LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODUpdateFrequency = 0.5f; // Update LOD twice per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HysteresisDistance = 50.0f; // Prevent LOD thrashing

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    FPhysicsOptimizationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableMetricsLogging = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MetricsUpdateFrequency = 1.0f;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ForceUpdateLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    EPhysicsLODState GetCurrentLODState() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FPhysicsOptimizationMetrics GetOptimizationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnableOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetLODDistanceOverride(float Distance, EPhysicsLODState LODState);

private:
    // Internal state
    UPROPERTY()
    TWeakObjectPtr<UPhysicsSystemManager> PhysicsManager;

    EPhysicsLODState CurrentLODState = EPhysicsLODState::FullDetail;
    EPhysicsLODState PreviousLODState = EPhysicsLODState::FullDetail;
    
    float LastLODUpdateTime = 0.0f;
    float LastMetricsUpdateTime = 0.0f;
    float DistanceToPlayer = 0.0f;
    float PreviousDistanceToPlayer = 0.0f;
    
    bool bOptimizationEnabled = true;
    bool bLODTransitionInProgress = false;

    // Performance tracking
    float AccumulatedPhysicsTime = 0.0f;
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;

    // Internal methods
    void UpdateLODSystem(float DeltaTime);
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyLODSettings(const FPhysicsLODSettings& Settings);
    void TransitionToLODState(EPhysicsLODState NewState);
    
    EPhysicsLODState CalculateOptimalLODState() const;
    float GetDistanceToPlayer() const;
    bool ShouldUpdateLOD(float CurrentTime) const;
    
    void OptimizePhysicsComponents();
    void RestorePhysicsComponents();
    void UpdateCollisionSettings(UPrimitiveComponent* Component, const FPhysicsLODSettings& Settings);
    
    void LogOptimizationMetrics() const;
    void ResetMetrics();

    // LOD transition helpers
    void BeginLODTransition(EPhysicsLODState FromState, EPhysicsLODState ToState);
    void CompleteLODTransition();
    bool IsLODTransitionValid(EPhysicsLODState FromState, EPhysicsLODState ToState) const;

    // Performance helpers
    bool IsPerformanceTargetMet() const;
    float GetCurrentPhysicsLoad() const;
    void AdjustOptimizationForPerformance();
};