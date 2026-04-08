#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimInstance.h"
#include "MotionMatchingComponent.generated.h"

UENUM(BlueprintType)
enum class EMotionMatchingState : uint8
{
    Locomotion,
    Combat,
    Interaction,
    Climbing,
    Swimming,
    Stealth,
    Panic,
    Injured
};

USTRUCT(BlueprintType)
struct FMotionMatchingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* Database;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseTrajectoryPrediction;

    FMotionMatchingConfig()
    {
        Database = nullptr;
        BlendTime = 0.2f;
        SearchThreshold = 0.1f;
        bUseTrajectoryPrediction = true;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingState(EMotionMatchingState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EMotionMatchingState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetDatabase(EMotionMatchingState State, UPoseSearchDatabase* Database);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetCurrentDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTrajectory(const FVector& DesiredVelocity, const FVector& DesiredFacing);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetStealthMode(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetPanicMode(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetInjuredMode(bool bEnabled, float InjurySeverity = 0.5f);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInStealthMode() const { return bStealthMode; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInPanicMode() const { return bPanicMode; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInjured() const { return bInjuredMode; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetInjurySeverity() const { return InjurySeverity; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<EMotionMatchingState, FMotionMatchingConfig> StateConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EMotionMatchingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StateTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bDebugMotionMatching;

    // Estados especiais para o jogo de sobrevivência
    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    bool bStealthMode;

    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    bool bPanicMode;

    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    bool bInjuredMode;

    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    float InjurySeverity;

    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival States")
    float FearLevel;

private:
    void InitializeDefaultConfigurations();
    void UpdateStateBasedOnConditions();
    void HandleStateTransition(EMotionMatchingState NewState);
    void UpdateSurvivalStates(float DeltaTime);

    float StateTransitionTimer;
    EMotionMatchingState PreviousState;
};