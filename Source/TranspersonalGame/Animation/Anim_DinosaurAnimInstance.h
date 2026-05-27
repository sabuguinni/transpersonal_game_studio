#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalo"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurMotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsGrounded;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float GroundDistance;

    FAnim_DinosaurMotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsGrounded = true;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    EAnim_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    EAnim_DinosaurSpecies Species;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bIsTerritory;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bHasTarget;

    FAnim_DinosaurBehaviorData()
    {
        CurrentState = EAnim_DinosaurState::Idle;
        Species = EAnim_DinosaurSpecies::TRex;
        AggressionLevel = 0.0f;
        HungerLevel = 0.5f;
        HealthPercentage = 1.0f;
        bIsHunting = false;
        bIsTerritory = false;
        bHasTarget = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_DinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation State Properties
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_DinosaurMotionData MotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_DinosaurBehaviorData BehaviorData;

    // Animation Blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float IdleToWalkBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float WalkToRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float AttackIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float FeedingBlend;

    // IK and Procedural Animation
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HeadLookAtAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector HeadLookAtTarget;

    // Species-Specific Animation Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Species")
    float TailSwayIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Species")
    float NeckFlexibility;

    UPROPERTY(BlueprintReadOnly, Category = "Species")
    float BodyMassScale;

protected:
    // Internal calculation methods
    void UpdateMotionData(float DeltaTime);
    void UpdateBehaviorData(float DeltaTime);
    void UpdateFootIK(float DeltaTime);
    void UpdateHeadTracking(float DeltaTime);
    void UpdateSpeciesParameters(float DeltaTime);

    // Cached references
    UPROPERTY()
    class APawn* OwningPawn;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Animation timing
    float LastUpdateTime;
    float AnimationDeltaTime;

    // IK calculation helpers
    FVector TraceFootIK(const FName& SocketName, float TraceDistance = 50.0f);
    FRotator CalculateFootRotation(const FVector& ImpactNormal);
    FVector CalculateHeadLookAt(const FVector& TargetLocation);
};