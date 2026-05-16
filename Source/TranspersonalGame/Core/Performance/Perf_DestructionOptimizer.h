#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Perf_DestructionOptimizer.generated.h"

class ACore_DestructionSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DistanceThreshold = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDebrisCount = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DebrisLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsSimulation = true;

    FPerf_DestructionLOD()
    {
        DistanceThreshold = 5000.0f;
        MaxDebrisCount = 25;
        DebrisLifetime = 30.0f;
        bEnablePhysicsSimulation = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDebrisCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DestructionEventsThisFrame = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsSimulationTime = 0.0f;

    FPerf_DestructionMetrics()
    {
        ActiveDebrisCount = 0;
        AverageFrameTime = 0.0f;
        DestructionEventsThisFrame = 0;
        PhysicsSimulationTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_DestructionOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_DestructionOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance optimization methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeDestructionSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantDebris();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BatchUpdateDebris();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupOldDebris();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_DestructionMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDestructionFrameImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimal() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODBasedOnDistance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDebrisLODSettings(const FPerf_DestructionLOD& LODSettings);

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_DestructionLOD HighQualityLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_DestructionLOD MediumQualityLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_DestructionLOD LowQualityLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxDestructionBudget = 5.0f; // 5ms per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxSimultaneousDestructions = 3;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_DestructionMetrics CurrentMetrics;

    // Timer handles
    FTimerHandle OptimizationTimerHandle;
    FTimerHandle CleanupTimerHandle;
    FTimerHandle MetricsTimerHandle;

    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> TrackedDebrisActors;
    TArray<TWeakObjectPtr<ACore_DestructionSystem>> DestructionSystems;
    
    float LastFrameTime = 0.0f;
    int32 FrameCounter = 0;
    float AccumulatedFrameTime = 0.0f;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics();
    void AdjustLODBasedOnPerformance();
    void RegisterDestructionSystem(ACore_DestructionSystem* System);
    void UnregisterDestructionSystem(ACore_DestructionSystem* System);
    void ProcessDebrisQueue();
    void OptimizePhysicsSettings();
};