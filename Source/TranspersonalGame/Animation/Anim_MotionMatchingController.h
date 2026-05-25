#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingController.generated.h"

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
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    SpearThrow      UMETA(DisplayName = "Spear Throw"),
    StoneKnapping   UMETA(DisplayName = "Stone Knapping"),
    FireMaking      UMETA(DisplayName = "Fire Making"),
    ShelterBuilding UMETA(DisplayName = "Shelter Building"),
    HideWorking     UMETA(DisplayName = "Hide Working"),
    ToolCrafting    UMETA(DisplayName = "Tool Crafting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float GroundDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FRotator BodyRotation;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
        BodyRotation = FRotator::ZeroRotator;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bIsPlanted;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsPlanted = false;
    }
};

/**
 * Advanced Motion Matching Controller for primitive survival character
 * Handles fluid movement transitions and adaptive foot IK for uneven terrain
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CORE MOTION MATCHING ===
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetMotionData() const { return MotionData; }

    // === STATE TRANSITIONS ===
    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    bool CanTransitionToState(EAnim_MovementState TargetState) const;

    // === FOOT IK SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_IKFootData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_IKFootData GetRightFootIK() const { return RightFootIK; }

    // === SURVIVAL ACTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void TriggerSpearThrow();

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void TriggerStoneKnapping();

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void TriggerGathering();

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void TriggerCrafting();

protected:
    // === CACHED REFERENCES ===
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    // === ANIMATION ASSETS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* SpearThrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* StoneKnappingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    // === MOTION MATCHING DATA ===
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData MotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_ActionState CurrentActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_MovementState PreviousMovementState;

    // === FOOT IK DATA ===
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_IKFootData RightFootIK;

    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Config")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Config")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Config")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float FootIKTraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float FootIKOffset = 5.0f;

    // === INTERNAL STATE ===
    UPROPERTY()
    float StateTransitionTimer;

    UPROPERTY()
    bool bIsTransitioning;

    UPROPERTY()
    float ActionCooldownTimer;

private:
    // === INTERNAL METHODS ===
    void CacheComponents();
    void UpdateMovementState(float DeltaTime);
    void UpdateActionState(float DeltaTime);
    void PerformFootTrace(bool bIsLeftFoot, FAnim_IKFootData& FootData);
    float CalculateMovementDirection() const;
    bool IsValidTransition(EAnim_MovementState From, EAnim_MovementState To) const;
};