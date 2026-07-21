#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_PlayerLocomotionSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_LocomotionState CurrentState;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_LocomotionState::Idle;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PlayerLocomotionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PlayerLocomotionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetCurrentLocomotionState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_LocomotionData GetLocomotionData() const;

    // Animation montage playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandingMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages();

protected:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Locomotion data
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionData LocomotionData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UAnimMontage* LandingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    // State transition thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float DirectionChangeSmoothing;

private:
    // Internal state tracking
    EAnim_LocomotionState PreviousState;
    float StateChangeTime;
    float SmoothedDirection;

    // Helper functions
    EAnim_LocomotionState CalculateLocomotionState() const;
    void HandleStateTransition(EAnim_LocomotionState NewState);
    float CalculateMovementDirection() const;
};

#include "Anim_PlayerLocomotionSystem.generated.h"