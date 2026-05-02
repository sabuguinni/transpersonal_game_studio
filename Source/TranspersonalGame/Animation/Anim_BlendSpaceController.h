#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_BlendSpaceController.generated.h"

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
    Landing     UMETA(DisplayName = "Landing")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Tired       UMETA(DisplayName = "Tired"),
    Injured     UMETA(DisplayName = "Injured"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Afraid      UMETA(DisplayName = "Afraid"),
    Cautious    UMETA(DisplayName = "Cautious")
};

USTRUCT(BlueprintType)
struct FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float SlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInAir;

    FAnim_BlendSpaceData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        SlopeAngle = 0.0f;
        TurnRate = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Movement data for blend spaces
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    FAnim_BlendSpaceData MovementData;

    // Current states
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_SurvivalState CurrentSurvivalState;

    // Blend space assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpaces")
    class UBlendSpace* IdleWalkRunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpaces")
    class UBlendSpace1D* TurnInPlaceBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpaces")
    class UBlendSpace* SlopeMovementBlendSpace;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    float SlopeAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    float TurnRate;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters")
    bool bIsCrouching;

    // Survival parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ThirstLevel;

private:
    // Update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateSurvivalState();
    void CalculateDirection();
    void CalculateSlopeAngle();
    void CalculateTurnRate(float DeltaTime);

    // Previous frame data for calculations
    FVector PreviousVelocity;
    FRotator PreviousRotation;
    float PreviousYaw;

    // Smoothing parameters
    UPROPERTY(EditAnywhere, Category = "Animation|Smoothing", meta = (AllowPrivateAccess = "true"))
    float SpeedSmoothingRate;

    UPROPERTY(EditAnywhere, Category = "Animation|Smoothing", meta = (AllowPrivateAccess = "true"))
    float DirectionSmoothingRate;

    UPROPERTY(EditAnywhere, Category = "Animation|Smoothing", meta = (AllowPrivateAccess = "true"))
    float TurnRateSmoothingRate;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSurvivalState(EAnim_SurvivalState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_BlendSpaceData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayMovementAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayTurnInPlaceAnimation() const;
};