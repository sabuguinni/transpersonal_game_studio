/**
 * @file PhysicsOptimizer.h
 * @brief Performance optimization system for consciousness physics
 * 
 * Manages LOD, culling, and adaptive quality for consciousness-based physics systems
 * to maintain 60+ FPS during peak spiritual experiences.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessLOD : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"), 
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct FPhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConsciousnessActors = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsCalculationsPerFrame = 0;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessFieldComplexity = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EConsciousnessLOD GetCurrentLODLevel() const { return CurrentLOD; }

    // Adaptive quality control
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetConsciousnessLOD(EConsciousnessLOD NewLOD);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableAdaptiveQuality(bool bEnable);

    // Culling and distance management
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetConsciousnessCullingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateConsciousnessActorCulling();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeConsciousnessMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void FlushUnusedConsciousnessData();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQualityEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ConsciousnessCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ConsciousnessLODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ConsciousnessLODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ConsciousnessLODDistance3 = 4000.0f;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessLOD CurrentLOD = EConsciousnessLOD::High;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FPhysicsPerformanceMetrics CurrentMetrics;

private:
    float PerformanceTimer = 0.0f;
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 60;

    // Internal optimization methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void CheckAdaptiveQuality();
    EConsciousnessLOD CalculateOptimalLOD() const;
    void ApplyLODSettings(EConsciousnessLOD LOD);
    void UpdateConsciousnessActorLOD(class AConsciousnessActor* Actor, float Distance);
};