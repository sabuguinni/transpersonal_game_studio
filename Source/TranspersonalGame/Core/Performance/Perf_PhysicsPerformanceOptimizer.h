#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"
#include "Perf_PhysicsPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Basic       UMETA(DisplayName = "Basic Optimization"),
    Moderate    UMETA(DisplayName = "Moderate Optimization"),
    Aggressive  UMETA(DisplayName = "Aggressive Optimization"),
    Maximum     UMETA(DisplayName = "Maximum Optimization")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 TotalPhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SleepingPhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 DisabledPhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AveragePhysicsComplexity = 0.0f;

    FPerf_PhysicsMetrics()
    {
        TotalPhysicsActors = 0;
        ActivePhysicsActors = 0;
        SleepingPhysicsActors = 0;
        DisabledPhysicsActors = 0;
        PhysicsFrameTime = 0.0f;
        AveragePhysicsComplexity = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float SleepThresholdDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float DisableThresholdDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActivePhysicsObjects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bOptimizeCollisionComplexity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bUseDistanceBasedOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bOptimizeSleepingObjects = true;

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsDistance = 5000.0f;
        SleepThresholdDistance = 3000.0f;
        DisableThresholdDistance = 8000.0f;
        MaxActivePhysicsObjects = 50;
        PhysicsUpdateInterval = 0.5f;
        bOptimizeCollisionComplexity = true;
        bUseDistanceBasedOptimization = true;
        bOptimizeSleepingObjects = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PhysicsPerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_PhysicsPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Physics optimization methods
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeByDistance();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCollisionComplexity();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeSleepingObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    // Physics monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Monitoring")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Monitoring")
    FPerf_PhysicsMetrics GetCurrentPhysicsMetrics() const { return CurrentPhysicsMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Physics Monitoring")
    void LogPhysicsPerformance();

    // Physics management
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void EnablePhysicsForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void DisablePhysicsForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void PutPhysicsActorToSleep(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void WakePhysicsActor(AActor* Actor);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Physics Utility")
    float GetDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Utility")
    bool IsActorPhysicsEnabled(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Utility")
    int32 GetPhysicsComplexityScore(AActor* Actor) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Monitoring")
    FPerf_PhysicsMetrics CurrentPhysicsMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bAutoOptimize = true;

private:
    float LastOptimizationTime = 0.0f;
    
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<AActor>> DisabledPhysicsActors;
    TArray<TWeakObjectPtr<AActor>> SleepingPhysicsActors;

    void CollectPhysicsActors();
    void ApplyOptimizationLevel();
    void RestorePhysicsStates();
    bool ShouldOptimizeActor(AActor* Actor, float DistanceToPlayer) const;
    void OptimizeActorPhysics(AActor* Actor, float DistanceToPlayer);
    void UpdateTrackedActors();
};