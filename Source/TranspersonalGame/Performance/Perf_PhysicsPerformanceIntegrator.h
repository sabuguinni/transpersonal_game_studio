#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

// Forward declarations for Core Systems integration
class UCore_RagdollController;
class UCore_TerrainPhysics;

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainAnalysisTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_PhysicsPerformanceMetrics()
        : CurrentFrameTime(0.0f)
        , PhysicsStepTime(0.0f)
        , ActiveRagdolls(0)
        , ActivePhysicsBodies(0)
        , TerrainAnalysisTime(0.0f)
        , MemoryUsageMB(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsStepTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float RagdollCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TerrainAnalysisFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableRagdollCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableTerrainOptimization;

    FPerf_PhysicsOptimizationSettings()
        : MaxPhysicsStepTime(16.67f)
        , MaxActiveRagdolls(10)
        , RagdollCullingDistance(5000.0f)
        , TerrainAnalysisFrequency(0.1f)
        , bEnablePhysicsLOD(true)
        , bEnableRagdollCulling(true)
        , bEnableTerrainOptimization(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsPerformanceChanged, const FPerf_PhysicsPerformanceMetrics&, Metrics);

/**
 * Performance integrator specifically designed to optimize Core Systems physics
 * Manages ragdoll performance, terrain physics optimization, and overall physics budget
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core optimization methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    // Ragdoll optimization integration with Core Systems
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveRagdollCount() const;

    // Terrain physics optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateTerrainAnalysisFrequency(float NewFrequency);

    // Physics LOD system
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsLODLevel(AActor* Actor, int32 LODLevel);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsFrameTime() const;

    // Emergency optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EmergencyPhysicsOptimization();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void TestPhysicsPerformance();

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PerformanceThreshold;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    // Core Systems integration
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<AActor*> TrackedRagdollActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<AActor*> TrackedTerrainActors;

    // Optimization timers
    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    float OptimizationInterval;

    UPROPERTY()
    float LastMetricsUpdateTime;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsPerformanceChanged OnPhysicsPerformanceChanged;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics();
    void ApplyOptimizationLevel();
    void ManagePhysicsBudget();
    void OptimizePhysicsSettings();
    void CullUnnecessaryPhysics();
    void UpdatePhysicsLOD();
    
    // Core Systems specific optimizations
    void OptimizeCoreRagdollSystems();
    void OptimizeCoreTerrainPhysics();
    void IntegrateWithCorePhysicsManager();
    
    // Performance analysis
    void AnalyzePhysicsBottlenecks();
    void LogPerformanceMetrics();
    bool ShouldOptimize() const;
};