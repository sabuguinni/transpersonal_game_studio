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
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_PrehistoricMovementSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Sprinting UMETA(DisplayName = "Sprinting"),
    Crouching UMETA(DisplayName = "Crouching"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    Landing UMETA(DisplayName = "Landing"),
    Swimming UMETA(DisplayName = "Swimming"),
    Climbing UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_LocomotionType : uint8
{
    Bipedal UMETA(DisplayName = "Bipedal Human"),
    Quadrupedal UMETA(DisplayName = "Quadrupedal Animal"),
    Flying UMETA(DisplayName = "Flying Creature"),
    Swimming UMETA(DisplayName = "Aquatic Creature")
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
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainAdaptation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float GroundSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FVector GroundNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float FootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FRotator RightFootRotation;

    FAnim_TerrainAdaptation()
    {
        GroundSlope = 0.0f;
        GroundNormal = FVector::UpVector;
        FootIKAlpha = 0.0f;
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
    }
};

/**
 * Prehistoric Movement System - Handles realistic movement animations for prehistoric characters
 * Adapts movement to terrain, handles primitive locomotion patterns, and provides smooth transitions
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PrehistoricMovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricMovementSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core movement properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    EAnim_LocomotionType LocomotionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float CrouchSpeed;

    // Animation blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* RunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace1D* SprintBlendSpace;

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CrouchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* ClimbMontage;

    // Terrain adaptation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float MaxSlopeAngle;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData CurrentMovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_TerrainAdaptation TerrainData;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UAnimInstance* AnimInstance;

public:
    // Core movement functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    EAnim_MovementState GetMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void PlayMovementMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    // Terrain adaptation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void UpdateTerrainAdaptation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void PerformFootIKTrace(FVector& FootOffset, FRotator& FootRotation, FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float CalculateGroundSlope();

    // Animation state functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TransitionToState(EAnim_MovementState TargetState, float TransitionTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanTransitionToState(EAnim_MovementState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceValues();

    // Prehistoric movement behaviors
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Behavior")
    void ApplyPrehistoricMovementStyle();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Behavior")
    void HandleRoughTerrain();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Behavior")
    void AdjustForCarriedWeight(float WeightFactor);

private:
    // Internal state tracking
    EAnim_MovementState PreviousMovementState;
    float StateTransitionTimer;
    float MovementBlendAlpha;
    
    // Foot IK data
    FVector LeftFootIKOffset;
    FVector RightFootIKOffset;
    FRotator LeftFootIKRotation;
    FRotator RightFootIKRotation;
    
    // Helper functions
    void InitializeReferences();
    void UpdateMovementSpeed();
    void UpdateMovementDirection();
    void SmoothFootIKTransitions(float DeltaTime);
    bool IsValidMovementTransition(EAnim_MovementState From, EAnim_MovementState To);
};