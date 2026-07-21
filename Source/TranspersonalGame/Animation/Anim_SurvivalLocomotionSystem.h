#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_SurvivalLocomotionSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalMovementState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sprinting       UMETA(DisplayName = "Sprinting"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Crawling        UMETA(DisplayName = "Crawling"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Carrying        UMETA(DisplayName = "Carrying")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Building        UMETA(DisplayName = "Building"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Observing       UMETA(DisplayName = "Observing"),
    Communicating   UMETA(DisplayName = "Communicating"),
    Fighting        UMETA(DisplayName = "Fighting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Healing         UMETA(DisplayName = "Healing"),
    Exploring       UMETA(DisplayName = "Exploring")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Acceleration;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    EAnim_SurvivalMovementState MovementState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Action")
    EAnim_SurvivalActionState ActionState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float StaminaLevel;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float HealthLevel;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    float GroundSlope;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    bool bIsOnUnevenTerrain;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    bool bIsInWater;

    FAnim_SurvivalMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Acceleration = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_SurvivalMovementState::Idle;
        ActionState = EAnim_SurvivalActionState::None;
        StaminaLevel = 100.0f;
        HealthLevel = 100.0f;
        FearLevel = 0.0f;
        bIsInjured = false;
        bIsExhausted = false;
        GroundSlope = 0.0f;
        bIsOnUnevenTerrain = false;
        bIsInWater = false;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalLocomotionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalLocomotionSystem();

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
    class UBlendSpace* CrouchBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UBlendSpace* InjuredBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* LandingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* ClimbingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Assets")
    class UAnimMontage* SwimmingMontage;

    // Survival action montages
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* HuntingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* BuildingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* EatingMontage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Montages")
    class UAnimMontage* DrinkingMontage;

    // Movement settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings")
    float WalkSpeedThreshold;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings")
    float RunSpeedThreshold;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings")
    float SprintSpeedThreshold;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings")
    float MovementSmoothingSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement Settings")
    float DirectionSmoothingSpeed;

    // Survival settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Settings")
    float InjuredSpeedMultiplier;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Settings")
    float ExhaustedSpeedMultiplier;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Settings")
    float FearMovementMultiplier;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Settings")
    float StaminaDepletionRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival Settings")
    float HealthRegenerationRate;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_SurvivalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_SurvivalActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalAction(EAnim_SurvivalActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopSurvivalAction();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyInjury(float InjuryLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStamina(float StaminaChange);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalFactors();

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_SurvivalMovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPerformingAction() const { return MovementData.ActionState != EAnim_SurvivalActionState::None; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool CanPerformAction() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetMovementSpeedMultiplier() const;

private:
    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Internal state
    float SmoothedSpeed;
    float SmoothedDirection;
    float LastUpdateTime;
    bool bIsInitialized;

    // Private methods
    void InitializeComponent();
    void UpdateMovementState();
    void UpdateActionState();
    void CalculateMovementValues();
    void ApplySurvivalModifiers();
    void CheckEnvironmentalConditions();
    EAnim_SurvivalMovementState DetermineMovementState() const;
    class UAnimMontage* GetMontageForAction(EAnim_SurvivalActionState ActionType) const;
};