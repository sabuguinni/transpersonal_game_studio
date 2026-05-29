#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_DinosaurAnimController.generated.h"

// Dinosaur movement states
UENUM(BlueprintType)
enum class EAnim_DinosaurMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Charging    UMETA(DisplayName = "Charging"),
    Turning     UMETA(DisplayName = "Turning"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dying       UMETA(DisplayName = "Dying"),
    Dead        UMETA(DisplayName = "Dead")
};

// Dinosaur behavior states
UENUM(BlueprintType)
enum class EAnim_DinosaurBehaviorState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Territorial UMETA(DisplayName = "Territorial"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Mating      UMETA(DisplayName = "Mating"),
    Nesting     UMETA(DisplayName = "Nesting"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Socializing UMETA(DisplayName = "Socializing"),
    Patrolling  UMETA(DisplayName = "Patrolling")
};

// Dinosaur species types
UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

// Dinosaur size categories
UENUM(BlueprintType)
enum class EAnim_DinosaurSize : uint8
{
    Small   UMETA(DisplayName = "Small (0-2m)"),
    Medium  UMETA(DisplayName = "Medium (2-6m)"),
    Large   UMETA(DisplayName = "Large (6-12m)"),
    Massive UMETA(DisplayName = "Massive (12m+)")
};

// Animation blend data for smooth transitions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurBlendData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float MovementBlendTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float BehaviorBlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float TurnBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float AlertnessBlendSpeed = 1.5f;

    FAnim_DinosaurBlendData()
    {
        MovementBlendTime = 0.3f;
        BehaviorBlendTime = 0.5f;
        TurnBlendSpeed = 2.0f;
        AlertnessBlendSpeed = 1.5f;
    }
};

// Foot IK data for terrain adaptation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurFootIK
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector RightFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FrontLeftFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FrontRightFootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float TraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK = true;

    FAnim_DinosaurFootIK()
    {
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        FrontLeftFootOffset = FVector::ZeroVector;
        FrontRightFootOffset = FVector::ZeroVector;
        IKAlpha = 1.0f;
        TraceDistance = 100.0f;
        bEnableFootIK = true;
    }
};

/**
 * Comprehensive animation controller for dinosaur characters
 * Handles movement states, behavior states, foot IK, and species-specific animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_DinosaurAnimController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurAnimController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Core animation state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Animation")
    EAnim_DinosaurMovementState MovementState = EAnim_DinosaurMovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Animation")
    EAnim_DinosaurBehaviorState BehaviorState = EAnim_DinosaurBehaviorState::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Animation")
    EAnim_DinosaurSpecies Species = EAnim_DinosaurSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Animation")
    EAnim_DinosaurSize SizeCategory = EAnim_DinosaurSize::Medium;

    // Movement parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCharging = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsTurning = false;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsAlert = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsFleeing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsFeeding = false;

    // Animation blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    FAnim_DinosaurBlendData BlendData;

    // Foot IK system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_DinosaurFootIK FootIK;

    // Species-specific parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BodyLength = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BodyHeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float NeckLength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float TailLength = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsBiped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    // Animation functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetMovementState(EAnim_DinosaurMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetBehaviorState(EAnim_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void UpdateMovementParameters(float NewSpeed, float NewDirection);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void UpdateBehaviorParameters(float Alertness, float Aggression, float Hunger, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerDeath();

    // Foot IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector TraceForGround(const FVector& FootLocation, float TraceDistance = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableFootIK(bool bEnable);

    // Species configuration
    UFUNCTION(BlueprintCallable, Category = "Species Configuration")
    void ConfigureForSpecies(EAnim_DinosaurSpecies TargetSpecies);

protected:
    // Internal state tracking
    EAnim_DinosaurMovementState PreviousMovementState = EAnim_DinosaurMovementState::Idle;
    EAnim_DinosaurBehaviorState PreviousBehaviorState = EAnim_DinosaurBehaviorState::Passive;
    
    float StateTransitionTimer = 0.0f;
    float LastSpeedUpdate = 0.0f;
    float LastDirectionUpdate = 0.0f;

    // Character reference
    UPROPERTY()
    ACharacter* OwningCharacter = nullptr;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Internal update functions
    void UpdateMovementState();
    void UpdateBehaviorState();
    void UpdateAnimationBlending(float DeltaTime);
    void UpdateSpeciesParameters();
    
    // Helper functions
    bool ShouldTransitionToState(EAnim_DinosaurMovementState NewState) const;
    float CalculateBlendWeight(float CurrentValue, float TargetValue, float BlendSpeed, float DeltaTime) const;
    void LogAnimationState() const;
};