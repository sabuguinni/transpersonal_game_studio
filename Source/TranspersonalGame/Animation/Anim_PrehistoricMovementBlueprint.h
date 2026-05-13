#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_PrehistoricMovementBlueprint.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float VelocityZ;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsMoving = false;
        VelocityZ = 0.0f;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float ThirstPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsAfraid;

    FAnim_SurvivalState()
    {
        HealthPercentage = 100.0f;
        StaminaPercentage = 100.0f;
        HungerPercentage = 100.0f;
        ThirstPercentage = 100.0f;
        FearLevel = 0.0f;
        bIsExhausted = false;
        bIsInjured = false;
        bIsAfraid = false;
    }
};

UENUM(BlueprintType)
enum class EAnim_PrehistoricState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Combat          UMETA(DisplayName = "Combat"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Afraid          UMETA(DisplayName = "Afraid"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrehistoricMovementBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricMovementBlueprint();

protected:
    // Core movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Survival state data
    UPROPERTY(BlueprintReadOnly, Category = "Survival Data", meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalState SurvivalState;

    // Current animation state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State", meta = (AllowPrivateAccess = "true"))
    EAnim_PrehistoricState CurrentState;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* CombatMontage;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    // Animation state timers
    UPROPERTY(BlueprintReadOnly, Category = "Animation Timing", meta = (AllowPrivateAccess = "true"))
    float StateTransitionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Timing", meta = (AllowPrivateAccess = "true"))
    float IdleTime;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Timing", meta = (AllowPrivateAccess = "true"))
    float MovementTime;

    // Foot IK data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK", meta = (AllowPrivateAccess = "true"))
    float RightFootIKAlpha;

public:
    // Animation Blueprint interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // State management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetAnimationState(EAnim_PrehistoricState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_PrehistoricState GetCurrentAnimationState() const { return CurrentState; }

    // Movement data getters
    UFUNCTION(BlueprintPure, Category = "Movement Data")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Survival Data")
    FAnim_SurvivalState GetSurvivalState() const { return SurvivalState; }

    // Animation montage controls
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayCraftingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayGatheringAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayCombatAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopAllMontages();

    // Foot IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FVector GetLeftFootIKLocation() const { return LeftFootIKLocation; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FVector GetRightFootIKLocation() const { return RightFootIKLocation; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FRotator GetLeftFootIKRotation() const { return LeftFootIKRotation; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FRotator GetRightFootIKRotation() const { return RightFootIKRotation; }

protected:
    // Internal update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateSurvivalState(float DeltaTime);
    void UpdateAnimationState(float DeltaTime);
    void UpdateStateTimers(float DeltaTime);

    // Foot IK helpers
    FVector PerformFootTrace(const FName& SocketName, float TraceDistance = 50.0f);
    FRotator CalculateFootRotation(const FVector& ImpactNormal);

    // State transition logic
    EAnim_PrehistoricState CalculateDesiredState();
    bool CanTransitionToState(EAnim_PrehistoricState NewState);
    void OnStateChanged(EAnim_PrehistoricState OldState, EAnim_PrehistoricState NewState);
};