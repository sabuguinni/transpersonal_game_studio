#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimationBlueprint.generated.h"

UENUM(BlueprintType)
enum class EAnim_CharacterState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Injured     UMETA(DisplayName = "Injured"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterAnimationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation State")
    EAnim_CharacterState CurrentState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    EAnim_MovementDirection MovementDirection;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
    bool bIsInAir;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
    bool bIsGrounded;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    float TerrainSlope;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    bool bIsOnUnevenTerrain;

    FAnim_CharacterAnimationData()
    {
        CurrentState = EAnim_CharacterState::Idle;
        Speed = 0.0f;
        Direction = 0.0f;
        MovementDirection = EAnim_MovementDirection::Forward;
        bIsInAir = false;
        bIsGrounded = true;
        bIsInCombat = false;
        HealthPercentage = 100.0f;
        StaminaPercentage = 100.0f;
        FearLevel = 0.0f;
        TerrainSlope = 0.0f;
        bIsOnUnevenTerrain = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimationBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationBlueprint();

    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterState(EAnim_CharacterState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_CharacterState GetCurrentState() const { return AnimationData.CurrentState; }

    // Movement Animation
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementAnimation();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void CalculateMovementDirection();

    // Survival Animation
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalAnimations();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void PlaySurvivalAction(ESurvivalActionType ActionType);

    // Combat Animation
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PlayAttackAnimation(int32 AttackIndex);

    // Environmental Animation
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalAnimations();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void AdaptToTerrain();

    // Animation Montages
    UFUNCTION(BlueprintCallable, Category = "Montages")
    void PlayMontageByName(const FString& MontageName);

    UFUNCTION(BlueprintCallable, Category = "Montages")
    void StopAllMontages();

protected:
    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_CharacterAnimationData AnimationData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ATranspersonalCharacter* OwnerCharacter;

    // Movement Component
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class UCharacterMovementComponent* MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* CombatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TArray<class UAnimMontage*> SurvivalMontages;

    // Animation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Parameters")
    float MovementSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Parameters")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Parameters")
    float SprintSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Parameters")
    float DirectionChangeThreshold;

    // State Transition
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Transition")
    float StateTransitionTime;

    UPROPERTY(BlueprintReadOnly, Category = "State Transition")
    float CurrentStateTime;

private:
    // Internal Methods
    void UpdateMovementData();
    void UpdatePhysicsData();
    void UpdateSurvivalData();
    void UpdateEnvironmentalData();
    
    bool CanTransitionToState(EAnim_CharacterState NewState) const;
    void HandleStateTransition(EAnim_CharacterState OldState, EAnim_CharacterState NewState);
    
    // Cache
    FVector LastVelocity;
    float LastSpeed;
    EAnim_CharacterState LastState;
};