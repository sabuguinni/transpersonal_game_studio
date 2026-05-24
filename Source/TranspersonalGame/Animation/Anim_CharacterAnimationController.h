#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Anim_CharacterAnimationController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    ECharacterState CharacterState = ECharacterState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsRunning = false;
        CharacterState = ECharacterState::Idle;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float NewSpeed, float NewDirection, bool bInAir, bool bCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterState(ECharacterState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(UAnimMontage* Montage);

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInAir() const { return MovementData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsCrouching() const { return MovementData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    ECharacterState GetCharacterState() const { return MovementData.CharacterState; }

protected:
    // Movement data tracking
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementData MovementData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DeathMontage;

    // Animation state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float StateTransitionAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float TimeSinceLastStateChange;

private:
    // Internal state management
    void UpdateStateTransition(float DeltaTime);
    void ValidateAnimationAssets();

    // Reference to character's mesh component
    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    // Reference to animation instance
    UPROPERTY()
    class UAnimInstance* AnimInstance;
};