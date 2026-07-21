#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Perf_PhysicsIntegrationOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra - Full Physics"),
    High        UMETA(DisplayName = "High - Optimized Physics"),
    Medium      UMETA(DisplayName = "Medium - Reduced Physics"),
    Low         UMETA(DisplayName = "Low - Minimal Physics"),
    Disabled    UMETA(DisplayName = "Disabled - No Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SimulatingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 RagdollBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_PhysicsOptimizationLevel OptimizationLevel = EPerf_PhysicsOptimizationLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxSimulatingBodies = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAdaptivePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bOptimizeRagdolls = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableCollisionCulling = true;
};

/**
 * Performance optimizer for Core Physics systems integration
 * Manages physics performance across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core optimization methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorPhysics(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeDinosaurPhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCollisionDetection();

    // Integration with Core Physics systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithPhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithCollisionManager();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithDestructionSystem();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void LogPerformanceReport() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastOptimizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OptimizationCycles = 0;

private:
    void OptimizePhysicsActors();
    void CullDistantPhysics();
    void AdaptPhysicsQuality();
    void ManagePhysicsMemory();
    
    float PerformanceCheckInterval = 1.0f;
    float LastPerformanceCheck = 0.0f;
    bool bIsOptimizing = false;
};