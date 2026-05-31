#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimationController.generated.h"

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage);

    // Movement Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementBlendSpace(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandAnimation();

    // Combat Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackAnimation(EAnim_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerBlockAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDodgeAnimation(EAnim_DodgeDirection Direction);

    // Survival Animation Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerCraftingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerGatheringAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerClimbingAnimation(bool bIsClimbing);

protected:
    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AttackLightMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AttackHeavyMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DodgeLeftMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DodgeRightMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* ClimbingMontage;

    // Animation State
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float CurrentDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsClimbing;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

private:
    // Internal state tracking
    float LastMontagePlayTime;
    UAnimMontage* CurrentlyPlayingMontage;

    // Helper functions
    void CacheComponentReferences();
    bool IsValidAnimationAsset(UAnimationAsset* Asset) const;
    void LogAnimationEvent(const FString& Event) const;
};