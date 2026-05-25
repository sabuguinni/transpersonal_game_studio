#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_PrimitiveMovementController.generated.h"

UENUM(BlueprintType)
enum class EAnim_PrimitiveMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAnim_PrimitiveAction : uint8
{
    None            UMETA(DisplayName = "None"),
    SpearThrow      UMETA(DisplayName = "Spear Throw"),
    StoneKnapping   UMETA(DisplayName = "Stone Knapping"),
    BerryPicking    UMETA(DisplayName = "Berry Picking"),
    WaterDrinking   UMETA(DisplayName = "Water Drinking"),
    FireMaking      UMETA(DisplayName = "Fire Making"),
    ShelterBuilding UMETA(DisplayName = "Shelter Building"),
    HideWorking     UMETA(DisplayName = "Hide Working"),
    ToolCrafting    UMETA(DisplayName = "Tool Crafting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrimitiveMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_PrimitiveMovementState MovementState = EAnim_PrimitiveMovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Actions")
    EAnim_PrimitiveAction CurrentAction = EAnim_PrimitiveAction::None;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    FAnim_PrimitiveMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsInAir = false;
        bIsMoving = false;
        bIsCrouching = false;
        MovementState = EAnim_PrimitiveMovementState::Idle;
        CurrentAction = EAnim_PrimitiveAction::None;
        StaminaPercentage = 1.0f;
        HealthPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveMovementController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveMovementController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* CharacterMovement;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_PrimitiveMovementData MovementData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* SpearThrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float SpeedSmoothingRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float DirectionSmoothingRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float MovementThreshold = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkSpeedThreshold = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float SprintSpeedThreshold = 600.0f;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_PrimitiveAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingActionMontage() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCurrentAction(EAnim_PrimitiveAction NewAction);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats(float Stamina, float Health, float Fear);

    // Getters for Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_PrimitiveMovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PrimitiveMovementState GetMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_PrimitiveAction GetCurrentAction() const { return MovementData.CurrentAction; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsInAir() const { return MovementData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    bool IsMoving() const { return MovementData.bIsMoving; }

private:
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    EAnim_PrimitiveMovementState CalculateMovementState() const;
    UAnimMontage* GetMontageForAction(EAnim_PrimitiveAction Action) const;

    // Smoothed values
    float SmoothedSpeed = 0.0f;
    float SmoothedDirection = 0.0f;
};