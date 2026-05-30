#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprint.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "Anim_CharacterAnimController.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimController : public UActorComponent
{
    GENERATED_BODY()

public:    
    UAnim_CharacterAnimController();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimations(float Speed, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationBlendWeight(const FString& AnimationName, float Weight);

    // Combat Animations
    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void PlayAttackAnimation(EAnim_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void PlayDefenseAnimation(bool bIsBlocking);

    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void PlayDodgeAnimation(EAnim_DodgeDirection Direction);

    // Survival Animations
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void PlayCraftingAnimation(EAnim_CraftingType CraftingType);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void PlayGatheringAnimation(EAnim_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void PlayEmotionalAnimation(EAnim_EmotionalState EmotionalState);

    // Animation Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInCombat;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DefenseMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

private:
    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Internal state tracking
    EAnim_MovementState CurrentMovementState;
    EAnim_CombatState CurrentCombatState;
    EAnim_EmotionalState CurrentEmotionalState;

    float BlendTimeRemaining;
    float CurrentAnimationTime;

    void InitializeAnimationReferences();
    void UpdateMovementState(float DeltaTime);
    void UpdateCombatState(float DeltaTime);
    void BlendAnimations(float DeltaTime);
};