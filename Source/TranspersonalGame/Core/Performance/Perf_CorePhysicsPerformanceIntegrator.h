#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Perf_CorePhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsQualityLevel : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Ultra   UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsCollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsStepTime;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsActors = 0;
        PhysicsCollisionChecks = 0;
        PhysicsMemoryUsage = 0.0f;
        AveragePhysicsStepTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CorePhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CorePhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    EPerf_PhysicsQualityLevel CurrentQualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bAutoOptimizePhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float TargetPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxPhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ApplyPhysicsOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ResetPhysicsToDefaults();

private:
    void ApplyLowQualitySettings();
    void ApplyMediumQualitySettings();
    void ApplyHighQualitySettings();
    void ApplyUltraQualitySettings();

    void MonitorPhysicsPerformance();
    void AdjustPhysicsSettings();

    float PhysicsFrameTimeHistory[60];
    int32 FrameHistoryIndex;
    float LastOptimizationTime;
    bool bInitialized;
};