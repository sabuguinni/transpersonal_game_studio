#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Anim_TribalAnimationManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling")
};

UENUM(BlueprintType)
enum class EAnim_TribalActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fishing         UMETA(DisplayName = "Fishing"),
    Building        UMETA(DisplayName = "Building"),
    Fighting        UMETA(DisplayName = "Fighting"),
    Eating          UMETA(DisplayName = "Eating"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Animations")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animations")
    TObjectPtr<UAnimMontage> WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animations")
    TObjectPtr<UAnimMontage> RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Animations")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Animations")
    TObjectPtr<UAnimMontage> GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Animations")
    TObjectPtr<UAnimMontage> CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Animations")
    TObjectPtr<UAnimMontage> HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animations")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animations")
    TObjectPtr<UAnimMontage> DefendMontage;

    FAnim_TribalAnimationSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        MovementBlendSpace = nullptr;
        GatheringMontage = nullptr;
        CraftingMontage = nullptr;
        HuntingMontage = nullptr;
        AttackMontage = nullptr;
        DefendMontage = nullptr;
    }
};

/**
 * Manages tribal character animations for prehistoric survival gameplay
 * Handles state transitions, montage playback, and blend space coordination
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TribalAnimationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalAnimationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetMovementState(EAnim_TribalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetActionState(EAnim_TribalActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    EAnim_TribalMovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    EAnim_TribalActionState GetCurrentActionState() const { return CurrentActionState; }

    // Montage Playback
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayTribalMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    bool IsPlayingMontage() const;

    // Survival Action Animations
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayGatheringAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayCraftingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayHuntingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayCombatAnimation(bool bIsAttacking = true);

    // Movement Blend Space Control
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void UpdateMovementBlendSpace(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetMovementSpeed(float Speed) { MovementSpeed = Speed; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    float GetMovementSpeed() const { return MovementSpeed; }

protected:
    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    FAnim_TribalAnimationSet AnimationSet;

    // Current States
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_TribalMovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_TribalActionState CurrentActionState;

    // Movement Parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementDirection;

    // Animation Instance Reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimInstance> CachedAnimInstance;

    // Internal Methods
    void CacheAnimInstance();
    void UpdateAnimationStates();
    UAnimMontage* GetMontageForAction(EAnim_TribalActionState Action);
    void TransitionToMovementState(EAnim_TribalMovementState NewState);
};