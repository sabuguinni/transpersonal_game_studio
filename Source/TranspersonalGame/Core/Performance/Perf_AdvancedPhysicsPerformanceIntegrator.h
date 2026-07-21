#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Perf_AdvancedPhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsCollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainPhysicsComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsOptimizationLevel;

    FPerf_PhysicsPerformanceMetrics()
        : PhysicsFrameTime(0.0f)
        , ActivePhysicsBodies(0)
        , PhysicsCollisionChecks(0)
        , PhysicsMemoryUsage(0.0f)
        , TerrainPhysicsComplexity(0.0f)
        , PhysicsOptimizationLevel(1.0f)
    {}
};

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationStrategy : uint8
{
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced       UMETA(DisplayName = "Balanced"),
    Aggressive     UMETA(DisplayName = "Aggressive"),
    UltraPerformance UMETA(DisplayName = "Ultra Performance")
};

UENUM(BlueprintType)
enum class EPerf_PhysicsPerformanceLevel : uint8
{
    Ultra          UMETA(DisplayName = "Ultra"),
    High           UMETA(DisplayName = "High"),
    Medium         UMETA(DisplayName = "Medium"),
    Low            UMETA(DisplayName = "Low"),
    Minimal        UMETA(DisplayName = "Minimal")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_AdvancedPhysicsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_AdvancedPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Core performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPhysicsPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPhysicsPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentPhysicsMetrics() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsOptimizationStrategy(EPerf_PhysicsOptimizationStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel Level);

    // Integration with terrain and world systems
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AnalyzePhysicsBottlenecks();

    // Real-time performance adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustPhysicsQualityForFrameRate(float TargetFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPhysicsPerformanceOptimal() const;

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PhysicsOptimizationStrategy OptimizationStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PhysicsPerformanceLevel PerformanceLevel;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAcceptablePhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsMemoryThreshold;

    // Monitoring intervals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval;

private:
    // Internal monitoring
    float LastMonitoringUpdate;
    float LastOptimizationCheck;
    
    // Performance tracking
    TArray<float> PhysicsFrameTimeHistory;
    TArray<int32> PhysicsBodiesHistory;
    
    // Optimization state
    bool bNeedsOptimization;
    float CurrentOptimizationFactor;

    // Internal methods
    void UpdatePhysicsMetrics();
    void CheckPerformanceThresholds();
    void ApplyOptimizationStrategy();
    void UpdatePhysicsQuality();
    void LogPerformanceMetrics() const;
};

UCLASS()
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Global physics performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterPhysicsPerformanceIntegrator(APerf_AdvancedPhysicsPerformanceIntegrator* Integrator);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterPhysicsPerformanceIntegrator(APerf_AdvancedPhysicsPerformanceIntegrator* Integrator);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetGlobalPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeGlobalPhysicsPerformance();

protected:
    UPROPERTY()
    TArray<APerf_AdvancedPhysicsPerformanceIntegrator*> RegisteredIntegrators;

    UPROPERTY()
    FPerf_PhysicsPerformanceMetrics GlobalMetrics;

private:
    void UpdateGlobalMetrics();
    void ApplyGlobalOptimizations();
};