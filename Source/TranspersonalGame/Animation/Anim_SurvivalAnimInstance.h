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
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_SurvivalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    SpearThrow      UMETA(DisplayName = "Spear Throw"),
    StoneKnapping   UMETA(DisplayName = "Stone Knapping"),
    FireMaking      UMETA(DisplayName = "Fire Making"),
    Cooking         UMETA(DisplayName = "Cooking"),
    Building        UMETA(DisplayName = "Building"),
    Healing         UMETA(DisplayName = "Healing"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Eating          UMETA(DisplayName = "Eating")
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsCrouching;

    FAnim_MotionMatchingData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset;

    FAnim_IKFootData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootAlpha = 0.0f;
        RightFootAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_SurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_SurvivalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Management
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_SurvivalAction CurrentSurvivalAction;

    // Motion Matching Data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionMatchingData MotionData;

    // IK Foot Placement
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData IKData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TMap<EAnim_SurvivalAction, UAnimMontage*> SurvivalActionMontages;

    // Animation Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float MovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bIsClimbing;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    bool bIsSwimming;

    // Survival Action Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthLevel;

private:
    // Internal Methods
    void UpdateMovementState();
    void UpdateMotionMatchingData();
    void UpdateIKFootPlacement();
    void CalculateFootIK(const FName& FootBoneName, const FName& IKBoneName, float& FootAlpha, FVector& FootLocation, FRotator& FootRotation);
    FVector PerformLineTrace(const FVector& StartLocation, float TraceDistance) const;

public:
    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalAction(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopSurvivalAction();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsSurvivalActionPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeedRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FVector GetMovementDirection() const;
};