#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Perf_CoreSystemsPerformanceIntegrator.generated.h"

// Forward declarations
class UCore_TerrainPhysicsSystem;
class UCore_PhysicsSystemManager;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

UENUM(BlueprintType)
enum class EPerf_CoreSystemOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Performance"),
    High        UMETA(DisplayName = "High Performance"), 
    Medium      UMETA(DisplayName = "Medium Performance"),
    Low         UMETA(DisplayName = "Low Performance"),
    Minimal     UMETA(DisplayName = "Minimal Performance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CoreSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float TerrainPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float PhysicsSystemTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float RagdollSystemTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float DestructionSystemTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float TotalCoreSystemsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    int32 ActiveDestructibles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance")
    float CoreSystemsMemoryUsage = 0.0f;

    FPerf_CoreSystemMetrics()
    {
        TerrainPhysicsTime = 0.0f;
        PhysicsSystemTime = 0.0f;
        RagdollSystemTime = 0.0f;
        DestructionSystemTime = 0.0f;
        TotalCoreSystemsTime = 0.0f;
        ActivePhysicsActors = 0;
        ActiveRagdolls = 0;
        ActiveDestructibles = 0;
        CoreSystemsMemoryUsage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CoreSystemOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    EPerf_CoreSystemOptimizationLevel OptimizationLevel = EPerf_CoreSystemOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    float MaxTerrainPhysicsTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    float MaxPhysicsSystemTime = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    float MaxRagdollSystemTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    float MaxDestructionSystemTime = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    int32 MaxActivePhysicsActors = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    int32 MaxActiveRagdolls = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    int32 MaxActiveDestructibles = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    bool bEnableAdaptiveOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    bool bEnableTerrainPhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    bool bEnableRagdollOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Optimization")
    bool bEnableDestructionOptimization = true;

    FPerf_CoreSystemOptimizationSettings()
    {
        OptimizationLevel = EPerf_CoreSystemOptimizationLevel::Medium;
        MaxTerrainPhysicsTime = 5.0f;
        MaxPhysicsSystemTime = 8.0f;
        MaxRagdollSystemTime = 3.0f;
        MaxDestructionSystemTime = 4.0f;
        MaxActivePhysicsActors = 200;
        MaxActiveRagdolls = 50;
        MaxActiveDestructibles = 30;
        bEnableAdaptiveOptimization = true;
        bEnableTerrainPhysicsOptimization = true;
        bEnableRagdollOptimization = true;
        bEnableDestructionOptimization = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_CoreSystemsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_CoreSystemsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Performance Integration")
    FPerf_CoreSystemOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Core Systems Performance Integration")
    FPerf_CoreSystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Performance Integration")
    float MonitoringUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Performance Integration")
    bool bEnableRealTimeOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Systems Performance Integration")
    bool bEnableDebugLogging = false;

private:
    float LastMonitoringUpdate = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;

    // Performance monitoring methods
    void UpdateCoreSystemsMetrics();
    void MonitorTerrainPhysicsPerformance();
    void MonitorPhysicsSystemPerformance();
    void MonitorRagdollSystemPerformance();
    void MonitorDestructionSystemPerformance();
    void CalculateCoreSystemsMemoryUsage();

    // Optimization methods
    void ApplyOptimizationSettings();
    void OptimizeTerrainPhysics();
    void OptimizePhysicsSystem();
    void OptimizeRagdollSystem();
    void OptimizeDestructionSystem();
    void AdaptiveOptimization();

    // Utility methods
    void LogPerformanceMetrics();
    EPerf_CoreSystemOptimizationLevel DetermineOptimalLevel();

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    void SetOptimizationLevel(EPerf_CoreSystemOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    FPerf_CoreSystemMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    void ForceOptimizationUpdate();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    void ResetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration")
    void EnableAdaptiveOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Systems Performance Integration", CallInEditor = true)
    void RunCoreSystemsPerformanceTest();
};