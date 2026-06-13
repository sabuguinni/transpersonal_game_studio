#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Sprinting UMETA(DisplayName = "Sprinting"),
    Crouching UMETA(DisplayName = "Crouching"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    Landing UMETA(DisplayName = "Landing"),
    Climbing UMETA(DisplayName = "Climbing"),
    Swimming UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed UMETA(DisplayName = "Unarmed"),
    MeleeReady UMETA(DisplayName = "Melee Ready"),
    RangedReady UMETA(DisplayName = "Ranged Ready"),
    Attacking UMETA(DisplayName = "Attacking"),
    Blocking UMETA(DisplayName = "Blocking"),
    Dodging UMETA(DisplayName = "Dodging"),
    Stunned UMETA(DisplayName = "Stunned"),
    Dead UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None UMETA(DisplayName = "None"),
    Gathering UMETA(DisplayName = "Gathering"),
    Crafting UMETA(DisplayName = "Crafting"),
    Eating UMETA(DisplayName = "Eating"),
    Drinking UMETA(DisplayName = "Drinking"),
    Building UMETA(DisplayName = "Building"),
    Fishing UMETA(DisplayName = "Fishing"),
    Hunting UMETA(DisplayName = "Hunting"),
    Sleeping UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* TransitionMontage;

    FAnim_StateTransition()
    {
        FromState = EAnim_MovementState::Idle;
        ToState = EAnim_MovementState::Walking;
        TransitionDuration = 0.2f;
        TransitionMontage = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* BlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float SpeedX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float SpeedY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction;

    FAnim_BlendSpaceData()
    {
        BlendSpace = nullptr;
        SpeedX = 0.0f;
        SpeedY = 0.0f;
        Direction = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_CombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_SurvivalAction CurrentSurvivalAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsInjured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float InjuryLevel;

    // Movement Animation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animation")
    FAnim_BlendSpaceData LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animation")
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animation")
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animation")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animation")
    bool bIsGrounded;

    // Animation Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    TMap<EAnim_SurvivalAction, class UAnimMontage*> SurvivalActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    TMap<EAnim_CombatState, class UAnimMontage*> CombatMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    class UAnimMontage* DeathMontage;

    // State Transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    TArray<FAnim_StateTransition> StateTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    float DefaultTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Transitions")
    bool bIsTransitioning;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlaySurvivalAction(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayCombatMontage(EAnim_CombatState CombatState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void TriggerJump();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void TriggerLanding();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void TriggerDeath();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateMovementData(float Speed, float Direction, bool IsGrounded);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetInjuryState(bool Injured, float Level);

    // Animation Query Functions
    UFUNCTION(BlueprintPure, Category = "Animation Query")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Query")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation Query")
    bool IsPlayingMontage() const;

    UFUNCTION(BlueprintPure, Category = "Animation Query")
    float GetCurrentMontagePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopCurrentMontage();

private:
    // Internal state management
    void UpdateAnimationState(float DeltaTime);
    void ProcessStateTransition(EAnim_MovementState NewState);
    bool CanTransitionTo(EAnim_MovementState NewState) const;
    float GetTransitionDuration(EAnim_MovementState FromState, EAnim_MovementState ToState) const;

    // Component references
    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Transition tracking
    float TransitionTimer;
    EAnim_MovementState PreviousState;
    EAnim_MovementState TargetState;
};