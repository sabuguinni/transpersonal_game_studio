#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataTable.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FName FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FName ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    class UAnimMontage* TransitionMontage;

    FAnim_StateTransition()
    {
        FromState = NAME_None;
        ToState = NAME_None;
        BlendTime = 0.2f;
        TransitionMontage = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FName, class UBlendSpace*> MovementBlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FName, class UAnimMontage*> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_StateTransition> StateTransitions;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FName CurrentAnimationState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MotionMatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float StateTransitionCooldown;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& InVelocity, bool bInIsInAir, bool bInIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RequestStateTransition(FName NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UBlendSpace* GetCurrentBlendSpace() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimMontage* GetActionMontage(FName ActionName) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnStateChanged(FName OldState, FName NewState);

private:
    UPROPERTY()
    float LastTransitionTime;

    void ProcessMotionMatching();
    FName DetermineOptimalState() const;
    bool CanTransitionToState(FName TargetState) const;
};