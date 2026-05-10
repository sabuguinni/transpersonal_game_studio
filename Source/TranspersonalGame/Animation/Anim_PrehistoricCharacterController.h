#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_PrehistoricCharacterController.generated.h"

UENUM(BlueprintType)
enum class EAnim_PrehistoricMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

UENUM(BlueprintType)
enum class EAnim_PrehistoricActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Eating      UMETA(DisplayName = "Eating"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrehistoricAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_PrehistoricMovementState MovementState = EAnim_PrehistoricMovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_PrehistoricActionState ActionState = EAnim_PrehistoricActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    float YawDelta = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fatigue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;
};

/**
 * Prehistoric Character Animation Controller
 * Manages complex animation states for survival gameplay in Cretaceous period
 * Handles locomotion, survival actions, and environmental interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PrehistoricCharacterController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricCharacterController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Animation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnim_PrehistoricAnimationData AnimationData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwnerCharacter;

    // Movement Component Reference
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    UCharacterMovementComponent* MovementComponent;

    // Animation Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* EatingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* DrinkingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* CombatMontage;

    // Blend Spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    UBlendSpace1D* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    UBlendSpace* DirectionalBlendSpace;

    // Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AnimationBlendSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionSmoothingSpeed = 5.0f;

    // Survival Animation Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FatigueAnimationScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerAnimationScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearAnimationScale = 1.0f;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_PrehistoricActionState NewActionState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_PrehistoricActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_PrehistoricAnimationData GetAnimationData() const { return AnimationData; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetSpeed() const { return AnimationData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetDirection() const { return AnimationData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsInAir() const { return AnimationData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsCrouching() const { return AnimationData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PrehistoricMovementState GetMovementState() const { return AnimationData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PrehistoricActionState GetActionState() const { return AnimationData.ActionState; }

    // Survival State Setters
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFatigue(float NewFatigue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetHunger(float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFear(float NewFear);

    // Animation Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMovementStateChanged(EAnim_PrehistoricMovementState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnActionStateChanged(EAnim_PrehistoricActionState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnSurvivalStateChanged(float Fatigue, float Hunger, float Fear);

private:
    // Internal state tracking
    EAnim_PrehistoricMovementState PreviousMovementState;
    EAnim_PrehistoricActionState PreviousActionState;
    float PreviousSpeed;
    float PreviousDirection;
    
    // Smooth interpolation values
    float SmoothedDirection;
    float SmoothedSpeed;

    // Helper functions
    void UpdateAnimationData(float DeltaTime);
    void SmoothAnimationValues(float DeltaTime);
    void ApplySurvivalModifiers();
    EAnim_PrehistoricMovementState CalculateMovementState() const;
    float CalculateDirection() const;
    UAnimMontage* GetMontageForAction(EAnim_PrehistoricActionState ActionType) const;
};