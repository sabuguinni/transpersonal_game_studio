#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Perf_PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsLODLevel : uint8
{
    High = 0,
    Medium = 1,
    Low = 2,
    Disabled = 3
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MaxSimulationDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxActivePhysicsBodies = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float LODUpdateInterval = 1.0f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    FPerf_PhysicsProfile PhysicsProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    EPerf_PhysicsLODLevel CurrentLODLevel = EPerf_PhysicsLODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsBodiesCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float LastLODUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CurrentFrameTime = 0.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsLODLevel(EPerf_PhysicsLODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetActivePhysicsBodiesCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void DisableDistantPhysics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeCollisionComplexity();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetCurrentFrameTime() const { return CurrentFrameTime; }

private:
    void CountActivePhysicsBodies();
    void ApplyLODSettings();
    float CalculateDistanceToPlayer(AActor* Actor);
    void OptimizeActorPhysics(AActor* Actor, float Distance);
};