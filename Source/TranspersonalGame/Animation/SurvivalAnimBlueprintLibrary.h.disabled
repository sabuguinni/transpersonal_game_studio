#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "SurvivalAnimBlueprintLibrary.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Injured     UMETA(DisplayName = "Injured"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Hiding      UMETA(DisplayName = "Hiding")
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
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    EAnim_SurvivalState CurrentState = EAnim_SurvivalState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    EAnim_MovementDirection MovementDirection = EAnim_MovementDirection::Forward;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_SurvivalState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    bool bIsFearful = false;

    FAnim_SurvivalStats()
    {
        Health = 100.0f;
        Stamina = 100.0f;
        Fear = 0.0f;
        Hunger = 0.0f;
        Thirst = 0.0f;
        bIsInjured = false;
        bIsExhausted = false;
        bIsFearful = false;
    }
};

/**
 * Blueprint function library for survival animation utilities
 */
UCLASS()
class TRANSPERSONALGAME_API USurvivalAnimBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Locomotion Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static FAnim_LocomotionData CalculateLocomotionData(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static EAnim_SurvivalState DetermineSurvivalState(const FAnim_SurvivalStats& Stats, const FAnim_LocomotionData& Locomotion);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static EAnim_MovementDirection CalculateMovementDirection(const FVector& Velocity, const FRotator& ActorRotation);

    // Animation Blending Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static float CalculateSpeedBlendValue(float CurrentSpeed, float WalkSpeed = 150.0f, float RunSpeed = 600.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static float CalculateFearBlendValue(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static float CalculateInjuryBlendValue(float HealthPercentage);

    // Animation State Transitions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static bool ShouldTransitionToFearState(float FearLevel, float Threshold = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static bool ShouldTransitionToInjuredState(float HealthPercentage, float Threshold = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static bool ShouldTransitionToExhaustedState(float StaminaPercentage, float Threshold = 20.0f);

    // Animation Montage Helpers
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static bool PlaySurvivalMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static void StopSurvivalMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage, float BlendOutTime = 0.25f);

    // IK and Procedural Animation Helpers
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static FVector CalculateFootIKOffset(USkeletalMeshComponent* MeshComponent, const FName& BoneName, float TraceDistance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static FRotator CalculateFootIKRotation(USkeletalMeshComponent* MeshComponent, const FName& BoneName, const FVector& IKOffset);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static float GetCharacterGroundSpeed(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static bool IsCharacterOnGround(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    static FVector GetCharacterVelocity2D(ACharacter* Character);
};