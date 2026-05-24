#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_SurvivalMovementSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalStance : uint8
{
    Relaxed     UMETA(DisplayName = "Relaxed"),
    Alert       UMETA(DisplayName = "Alert"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Injured     UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    EAnim_SurvivalMovementState MovementState = EAnim_SurvivalMovementState::Idle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    EAnim_SurvivalStance CurrentStance = EAnim_SurvivalStance::Relaxed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float StaminaPercentage = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float FearLevel = 0.0f;

    FAnim_SurvivalMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_SurvivalMovementState::Idle;
        CurrentStance = EAnim_SurvivalStance::Relaxed;
        StaminaPercentage = 1.0f;
        HealthPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalMovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalMovementSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalMovementData MovementData;

    // Animation assets
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* LandMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* CrouchMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* ClimbMontage;

    // Movement thresholds
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Thresholds")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Thresholds")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Thresholds")
    float SprintSpeedThreshold = 500.0f;

    // Survival parameters
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Parameters")
    float StaminaDepletionRate = 0.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Parameters")
    float FearIncreaseRate = 0.05f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Parameters")
    float InjuryMovementPenalty = 0.3f;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

public:
    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void SetMovementState(EAnim_SurvivalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void SetSurvivalStance(EAnim_SurvivalStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void PlayMovementMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void UpdateSurvivalParameters(float StaminaLevel, float HealthLevel, float CurrentFear);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    bool IsExhausted() const;

    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    bool IsInjured() const;

    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    bool IsFearful() const;

    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    float GetMovementSpeedMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    EAnim_SurvivalMovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Survival Movement")
    EAnim_SurvivalStance GetCurrentStance() const { return MovementData.CurrentStance; }

private:
    void InitializeReferences();
    void CalculateMovementState();
    void UpdateStanceBasedOnSurvival();
    void ApplySurvivalEffects();

    // Timers and state tracking
    float LastMovementChangeTime = 0.0f;
    float StanceTransitionTime = 0.0f;
    bool bWasMovingLastFrame = false;
};