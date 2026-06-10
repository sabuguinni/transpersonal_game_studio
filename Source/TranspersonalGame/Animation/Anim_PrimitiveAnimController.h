#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame.h"
#include "Anim_PrimitiveAnimController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    MeleeWeapon UMETA(DisplayName = "Melee Weapon"),
    RangedWeapon UMETA(DisplayName = "Ranged Weapon"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dodging     UMETA(DisplayName = "Dodging")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector LeftFootIKLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector RightFootIKLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LeftFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float RightFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HipOffset;

    FAnim_IKData()
    {
        LeftFootIKLocation = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float TimeSinceLastMovement;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsMoving = false;
        TimeSinceLastMovement = 0.0f;
    }
};

/**
 * Primitive Animation Controller for prehistoric survival characters
 * Handles locomotion, combat, survival animations with motion matching and IK
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveAnimController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveAnimController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Movement state
    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    EAnim_CombatState CurrentCombatState;

    // Motion data for motion matching
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData MotionData;

    // IK data for foot placement
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKData IKData;

    // Locomotion parameters
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float MovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    // Combat parameters
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float CombatStance;

    // Survival parameters
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float AlertnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FatigueLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsCrafting;

    // Animation assets
    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UBlendSpace* CombatBlendSpace;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UAnimMontage* BlockMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UAnimMontage* DodgeMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UAnimMontage* GatherMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Assets")
    class UAnimMontage* CraftMontage;

    // IK settings
    UPROPERTY(EditDefaultsOnly, Category = "IK Settings")
    float IKTraceDistance;

    UPROPERTY(EditDefaultsOnly, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditDefaultsOnly, Category = "IK Settings")
    FName RightFootSocketName;

private:
    // Update functions
    void UpdateMovementState();
    void UpdateCombatState();
    void UpdateMotionData(float DeltaTime);
    void UpdateIKData(float DeltaTime);
    void UpdateSurvivalParameters();

    // IK calculation functions
    void CalculateFootIK(const FName& SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);
    float CalculateHipOffset();

    // Motion matching functions
    void UpdateMotionMatching(float DeltaTime);
    float CalculateMovementDirection();

    // State transition functions
    void TransitionToMovementState(EAnim_MovementState NewState);
    void TransitionToCombatState(EAnim_CombatState NewState);

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayBlockMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayDodgeMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGatherMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCraftMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatMode(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAlertness(float NewAlertness);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFatigue(float NewFatigue);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MotionData GetMotionData() const { return MotionData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_IKData GetIKData() const { return IKData; }
};