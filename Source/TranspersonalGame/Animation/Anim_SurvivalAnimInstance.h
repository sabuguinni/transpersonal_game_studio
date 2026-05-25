#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_SurvivalAnimInstance.generated.h"

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
    ToolCrafting    UMETA(DisplayName = "Tool Crafting"),
    BerryPicking    UMETA(DisplayName = "Berry Picking"),
    WaterDrinking   UMETA(DisplayName = "Water Drinking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float TimeSinceLastFootstep = 0.0f;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        TimeSinceLastFootstep = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_SurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_SurvivalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_ActionState ActionState = EAnim_ActionState::None;

    // Motion Data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData MotionData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Animation Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float MovementBlendAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float ActionBlendAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float FootIKAlpha = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    FVector LeftFootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    FVector RightFootOffset = FVector::ZeroVector;

    // Survival Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float FootstepInterval = 0.5f;

private:
    // State Update Functions
    void UpdateMovementState();
    void UpdateActionState();
    void UpdateMotionData(float DeltaTime);
    void UpdateFootIK();

    // Helper Functions
    EAnim_MovementState CalculateMovementState() const;
    float CalculateDirectionAngle() const;
    void PerformFootIKTrace(const FName& SocketName, FVector& OutOffset);

    // Cached Values
    FVector LastVelocity = FVector::ZeroVector;
    float LastUpdateTime = 0.0f;
};