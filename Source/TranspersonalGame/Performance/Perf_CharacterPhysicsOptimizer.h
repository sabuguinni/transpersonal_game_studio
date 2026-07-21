#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Perf_CharacterPhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActivePhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 RagdollInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float CollisionQueryTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 PhysicsConstraints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float MemoryUsageMB = 0.0f;

    FPerf_PhysicsMetrics()
    {
        PhysicsTickTime = 0.0f;
        ActivePhysicsBodies = 0;
        RagdollInstances = 0;
        CollisionQueryTime = 0.0f;
        PhysicsConstraints = 0;
        MemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_PhysicsOptimizationLevel OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float MaxPhysicsDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxSimultaneousRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhysicsTickRateScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableRagdollLimiting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableCollisionOptimization = true;

    FPerf_PhysicsOptimizationSettings()
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
        MaxPhysicsDistance = 2000.0f;
        MaxSimultaneousRagdolls = 10;
        PhysicsTickRateScale = 1.0f;
        bEnableDistanceCulling = true;
        bEnableRagdollLimiting = true;
        bEnableCollisionOptimization = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CharacterPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CharacterPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    // Current physics metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    FPerf_PhysicsMetrics CurrentMetrics;

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float MetricsUpdateInterval = 0.5f;

    // Physics optimization functions
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCharacterPhysics(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnableRagdollOptimization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetMaxPhysicsDistance(float Distance);

    // Metrics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Metrics")
    FPerf_PhysicsMetrics GetCurrentPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Metrics")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    bool IsPhysicsOptimizationActive() const;

    // Ragdoll management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Optimization")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Optimization")
    int32 GetActiveRagdollCount() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Optimization")
    void LimitRagdollInstances();

private:
    // Internal optimization functions
    void ApplyOptimizationLevel();
    void UpdateDistanceCulling();
    void ManageRagdollInstances();
    void OptimizeCollisionQueries();
    void UpdatePhysicsTickRate();

    // Performance tracking
    float LastMetricsUpdate = 0.0f;
    float PhysicsStartTime = 0.0f;
    TArray<TWeakObjectPtr<ACharacter>> TrackedCharacters;
    TArray<TWeakObjectPtr<ACharacter>> ActiveRagdolls;

    // Optimization state
    bool bOptimizationActive = false;
    float CurrentPhysicsTickRate = 1.0f;
    int32 CurrentRagdollCount = 0;
};