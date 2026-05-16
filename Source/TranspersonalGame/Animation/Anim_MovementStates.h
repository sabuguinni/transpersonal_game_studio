#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_MovementStates.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Defending       UMETA(DisplayName = "Defending"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Interacting     UMETA(DisplayName = "Interacting")
};

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
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Action")
    EAnim_ActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Lean;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimPitch;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        Lean = 0.0f;
        AimPitch = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MovementStates : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MovementStates();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    static FAnim_MovementData CalculateMovementData(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    static EAnim_MovementState GetMovementStateFromVelocity(const FVector& Velocity, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    static float CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    static float CalculateLean(const FVector& Velocity, const FRotator& ActorRotation, float DeltaTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean")
    float LeanInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean")
    float MaxLeanAngle;
};