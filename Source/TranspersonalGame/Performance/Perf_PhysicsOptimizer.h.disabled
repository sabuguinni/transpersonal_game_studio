#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Perf_PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsLOD : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Simplified  UMETA(DisplayName = "Simplified"),
    Standard    UMETA(DisplayName = "Standard"),
    Full        UMETA(DisplayName = "Full")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float FullPhysicsDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float StandardPhysicsDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float SimplifiedPhysicsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxActiveRagdolls = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bEnableDistanceCulling = true;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForDistance(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLOD(EPerf_PhysicsLOD NewLOD);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    EPerf_PhysicsLOD GetCurrentPhysicsLOD() const { return CurrentPhysicsLOD; }

    // Ragdoll Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeRagdollPhysics(USkeletalMeshComponent* SkeletalMesh, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnableRagdollOptimization(bool bEnable);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    float GetPhysicsPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    int32 GetActiveRagdollCount() const;

    // Collision Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCollisionComplexity(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetCollisionLOD(int32 LODLevel);

    // Batch Operations
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeAllPhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLODForAllActors();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsSettings(const FPerf_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FPerf_PhysicsSettings GetPhysicsSettings() const { return PhysicsSettings; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    FPerf_PhysicsSettings PhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    EPerf_PhysicsLOD CurrentPhysicsLOD;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    int32 ActiveRagdollCount;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float LastOptimizationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    TArray<TWeakObjectPtr<USkeletalMeshComponent>> TrackedRagdolls;

private:
    void UpdateRagdollTracking();
    void ApplyPhysicsLOD(USkeletalMeshComponent* SkeletalMesh, EPerf_PhysicsLOD LODLevel);
    float CalculateDistanceToPlayer(AActor* Actor);
    void CleanupInvalidRagdolls();
    void LimitActiveRagdolls();

    float OptimizationUpdateInterval = 0.1f;
    float LastUpdateTime = 0.0f;
};