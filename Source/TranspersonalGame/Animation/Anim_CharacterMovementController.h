#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterMovementController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty")
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
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    EAnim_SurvivalState SurvivalState;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        SurvivalState = EAnim_SurvivalState::Normal;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

/**
 * Animation controller that manages character movement and survival state animations
 * Bridges between character movement component and animation blueprint
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterMovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation data access
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return CurrentMovementData; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalState();

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearReaction(float FearIntensity);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerInjuryAnimation();

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* FearReactionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* InjuryMontage;

    // Blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* CrouchBlendSpace;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData CurrentMovementData;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal state tracking
    UPROPERTY()
    float LastSpeed;

    UPROPERTY()
    float SpeedChangeRate;

    UPROPERTY()
    float DirectionChangeRate;

    // Survival state thresholds
    UPROPERTY(EditAnywhere, Category = "Survival Thresholds")
    float ExhaustedThreshold = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Survival Thresholds")
    float InjuredThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Survival Thresholds")
    float FearThreshold = 0.3f;

private:
    void UpdateMovementValues();
    void UpdateSurvivalValues();
    EAnim_MovementState DetermineMovementState();
    EAnim_SurvivalState DetermineSurvivalState();
};