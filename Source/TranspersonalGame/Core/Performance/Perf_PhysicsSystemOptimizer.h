#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_PhysicsSystemOptimizer.generated.h"

// Forward declarations
class UCore_PhysicsConsolidator;
class UCore_PhysicsGameplayIntegrator;
class UCore_PhysicsSystemCoordinator;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CollisionFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float RagdollFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CharacterPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float DinosaurPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float TerrainPhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActiveCollisionPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsLODLevel;

    FPerf_PhysicsOptimizationMetrics()
        : PhysicsFrameTime(0.0f)
        , CollisionFrameTime(0.0f)
        , RagdollFrameTime(0.0f)
        , CharacterPhysicsTime(0.0f)
        , DinosaurPhysicsTime(0.0f)
        , TerrainPhysicsTime(0.0f)
        , ActivePhysicsActors(0)
        , ActiveCollisionPairs(0)
        , PhysicsMemoryUsageMB(0.0f)
        , PhysicsLODLevel(1.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetPhysicsFrameTime60fps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetPhysicsFrameTime30fps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxCollisionFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxRagdollFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float PhysicsLODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float PhysicsLODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float PhysicsLODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float PhysicsCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CollisionCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MaxPhysicsMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableAdaptiveQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableCollisionOptimization;

    FPerf_PhysicsOptimizationSettings()
        : TargetPhysicsFrameTime60fps(5.0f)
        , TargetPhysicsFrameTime30fps(10.0f)
        , MaxCollisionFrameTime(3.0f)
        , MaxRagdollFrameTime(2.0f)
        , PhysicsLODDistance1(1000.0f)
        , PhysicsLODDistance2(2000.0f)
        , PhysicsLODDistance3(4000.0f)
        , PhysicsCullingDistance(8000.0f)
        , CollisionCullingDistance(6000.0f)
        , MaxPhysicsMemoryMB(512.0f)
        , bEnableAdaptiveQuality(true)
        , bEnablePhysicsLOD(true)
        , bEnableCollisionOptimization(true)
    {}
};

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Low            UMETA(DisplayName = "Low Quality"),
    Medium         UMETA(DisplayName = "Medium Quality"),
    High           UMETA(DisplayName = "High Quality"),
    Ultra          UMETA(DisplayName = "Ultra Quality"),
    Adaptive       UMETA(DisplayName = "Adaptive Quality")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PhysicsSystemOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_PhysicsSystemOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsOptimizationMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;

    // Update frequency control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MetricsUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float OptimizationUpdateFrequency;

    // Physics system references
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    TWeakObjectPtr<AActor> PhysicsConsolidatorRef;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    TWeakObjectPtr<AActor> PhysicsGameplayIntegratorRef;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    TWeakObjectPtr<AActor> PhysicsCoordinatorRef;

private:
    float MetricsUpdateTimer;
    float OptimizationUpdateTimer;
    float LastFrameTime;
    float AverageFrameTime;
    int32 FrameCounter;

public:
    // Performance Monitoring Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_PhysicsOptimizationMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetPhysicsFrameTimePercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPhysicsPerformanceOptimal() const;

    // Optimization Control Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void EnableAdaptiveOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeCollisionDetection();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeRagdollPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeCharacterPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeDinosaurPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeTerrainPhysics();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizePhysicsMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ClearPhysicsCache();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    float GetPhysicsMemoryUsage() const;

    // Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterPhysicsSystem(AActor* PhysicsSystem, const FString& SystemType);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ValidatePhysicsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SynchronizeWithPhysicsSystems();

    // Emergency Optimization
    UFUNCTION(BlueprintCallable, Category = "Emergency Optimization")
    void TriggerEmergencyOptimization();

    UFUNCTION(BlueprintCallable, Category = "Emergency Optimization")
    void DisableNonEssentialPhysics();

    UFUNCTION(BlueprintCallable, Category = "Emergency Optimization")
    void RestorePhysicsSettings();

    // Debug and Profiling
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogPhysicsPerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void StartPhysicsProfiler();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void StopPhysicsProfiler();

protected:
    void UpdateAdaptiveOptimization();
    void CalculateOptimalLODLevel();
    void ApplyOptimizationLevel(EPerf_PhysicsOptimizationLevel Level);
    void MonitorFrameTime(float DeltaTime);
    bool ShouldOptimizePhysics() const;
    void FindPhysicsSystems();
};