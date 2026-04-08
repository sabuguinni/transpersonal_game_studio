#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearch.h"
#include "Animation/AnimInstance.h"
#include "AnimationSystemCore.h"
#include "MotionMatchingController.generated.h"

/**
 * Motion Matching Controller for Transpersonal Game
 * Manages pose search databases and motion matching queries
 * Implements context-aware animation selection based on gameplay state
 */

USTRUCT(BlueprintType)
struct FMotionMatchingQuery
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    FVector DesiredVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    float DesiredSpeed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    FRotator DesiredRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    FRotator CurrentRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    ECharacterMovementState MovementState = ECharacterMovementState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Query")
    float StaminaLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FDatabaseWeights
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float IdleWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float LocomotionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float PanicWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float StealthWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float InjuredWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
    float ExhaustedWeight = 1.0f;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Databases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Databases")
    TMap<ECharacterMovementState, class UPoseSearchDatabase*> StateDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Databases")
    TMap<EThreatLevel, class UPoseSearchDatabase*> ThreatDatabases;

    // Database Selection Weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Weights")
    FDatabaseWeights DatabaseWeights;

    // Query Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Query")
    float TrajectoryPredictionTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Query")
    int32 TrajectoryHistoryCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Query")
    float PoseHistoryTime = 0.5f;

    // Blending Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Blending")
    float DefaultBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Blending")
    float PanicBlendTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Blending")
    float StealthBlendTime = 0.4f;

    // Animation System Reference
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UAnimationSystemCore* AnimationSystem;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FMotionMatchingQuery BuildQuery() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UPoseSearchDatabase* SelectOptimalDatabase(const FMotionMatchingQuery& Query) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetBlendTimeForState(ECharacterMovementState State) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTrajectoryPrediction(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ShouldUseContextualAnimation(const FMotionMatchingQuery& Query) const;

private:
    // Trajectory tracking
    TArray<FVector> TrajectoryHistory;
    TArray<FVector> PredictedTrajectory;
    
    // Performance optimization
    float LastQueryTime = 0.0f;
    float QueryCooldown = 0.016f; // ~60fps
    
    void UpdateTrajectoryHistory(float DeltaTime);
    void PredictFutureTrajectory();
    float CalculateDatabaseScore(class UPoseSearchDatabase* Database, const FMotionMatchingQuery& Query) const;
    void OptimizeQueryFrequency(float DeltaTime);
};