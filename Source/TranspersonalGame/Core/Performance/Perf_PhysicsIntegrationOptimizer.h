#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "Perf_PhysicsIntegrationOptimizer.generated.h"

// Performance metrics for physics integration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SimulatingBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float RigidBodyUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 PhysicsLODLevel;

    FPerf_PhysicsIntegrationMetrics()
        : PhysicsFrameTime(0.0f)
        , ActivePhysicsActors(0)
        , SimulatingBodies(0)
        , CollisionDetectionTime(0.0f)
        , RigidBodyUpdateTime(0.0f)
        , PhysicsLODLevel(0)
    {}
};

// Physics LOD configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsLODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float PhysicsUpdateRate;

    FPerf_PhysicsLODConfig()
        : HighDetailDistance(1000.0f)
        , MediumDetailDistance(2500.0f)
        , LowDetailDistance(5000.0f)
        , bEnablePhysicsLOD(true)
        , PhysicsUpdateRate(60.0f)
    {}
};

/**
 * Physics Integration Optimizer - Manages physics performance across the game
 * Integrates with Core Systems Programmer's physics architecture
 * Provides LOD management and performance monitoring for physics simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPhysicsPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPhysicsPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsIntegrationMetrics GetCurrentPhysicsMetrics() const;

    // Physics LOD management
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UpdatePhysicsLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetPhysicsLODConfig(const FPerf_PhysicsLODConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    FPerf_PhysicsLODConfig GetPhysicsLODConfig() const { return PhysicsLODConfig; }

    // Physics optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForFrameRate(float TargetFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnableAdaptivePhysicsQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsQualityLevel(int32 QualityLevel);

    // Integration with Core Systems
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsActorLOD(AActor* Actor, int32 LODLevel);

    // Performance testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Testing")
    void RunPhysicsPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Testing")
    void GeneratePhysicsPerformanceReport();

protected:
    // Physics monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsIntegrationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    FPerf_PhysicsLODConfig PhysicsLODConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    int32 CurrentPhysicsQualityLevel;

    // Registered physics actors for LOD management
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredPhysicsActors;

    // Performance monitoring timer
    FTimerHandle MonitoringTimerHandle;

private:
    // Internal monitoring functions
    void UpdatePhysicsMetrics();
    void ApplyPhysicsLOD(AActor* Actor, int32 LODLevel);
    int32 CalculateLODLevel(const FVector& ActorLocation, const FVector& ViewerLocation) const;
    void AdjustPhysicsQuality();

    // Performance thresholds
    static constexpr float HIGH_PHYSICS_FRAME_TIME_THRESHOLD = 16.67f; // 60 FPS
    static constexpr float MEDIUM_PHYSICS_FRAME_TIME_THRESHOLD = 33.33f; // 30 FPS
    static constexpr int32 MAX_PHYSICS_ACTORS_HIGH_QUALITY = 100;
    static constexpr int32 MAX_PHYSICS_ACTORS_MEDIUM_QUALITY = 50;
};