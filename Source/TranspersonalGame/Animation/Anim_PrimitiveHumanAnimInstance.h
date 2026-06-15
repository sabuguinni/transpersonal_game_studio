#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "../Core/SharedTypes.h"
#include "Anim_PrimitiveHumanAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Combat          UMETA(DisplayName = "Combat"),
    Eating          UMETA(DisplayName = "Eating"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Ritual          UMETA(DisplayName = "Ritual")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAmount = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        bIsMoving = false;
        LeanAmount = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalGestureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FName GestureName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    class UAnimMontage* GestureMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    bool bIsLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float Priority = 1.0f;

    FAnim_TribalGestureData()
    {
        GestureName = NAME_None;
        GestureMontage = nullptr;
        Duration = 2.0f;
        bIsLooping = false;
        Priority = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveHumanAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveHumanAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Machine
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_ActionState CurrentActionState = EAnim_ActionState::None;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwningCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent = nullptr;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* IdleBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* WalkRunBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpStartSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpLoopSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpEndSequence = nullptr;

    // Tribal Gestures
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    TArray<FAnim_TribalGestureData> TribalGestures;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal Animations")
    bool bIsPerformingGesture = false;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal Animations")
    FName CurrentGestureName = NAME_None;

    // IK System Integration
    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    FVector LeftFootIKOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    FVector RightFootIKOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    float LeftFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    float RightFootIKAlpha = 0.0f;

public:
    // Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayTribalGesture(FName GestureName);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopCurrentGesture();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    bool IsGesturePlaying() const { return bIsPerformingGesture; }

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetActionState(EAnim_ActionState NewActionState);

    // State Query Functions
    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    UFUNCTION(BlueprintPure, Category = "Movement Data")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    // IK Integration
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(const FVector& LeftOffset, const FVector& RightOffset, float LeftAlpha, float RightAlpha);

private:
    // Internal State Management
    void UpdateMovementState();
    void UpdateMovementData();
    void CalculateDirection();
    void UpdateLeanAmount();

    // Gesture Management
    UFUNCTION()
    void OnGestureMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Cached Values
    FVector LastVelocity = FVector::ZeroVector;
    float LastUpdateTime = 0.0f;
    float DirectionSmoothingSpeed = 10.0f;
    float LeanSmoothingSpeed = 5.0f;
};