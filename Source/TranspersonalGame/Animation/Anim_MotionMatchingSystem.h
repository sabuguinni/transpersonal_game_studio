#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_LocomotionDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_LocomotionDirection LocomotionDirection = EAnim_LocomotionDirection::Forward;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace* IdleBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace* WalkBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace* RunBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace* SprintBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceDirection = 5.0f;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState CalculateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_LocomotionDirection CalculateLocomotionDirection();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UBlendSpace* GetCurrentBlendSpace() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetBlendSpaceX() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetBlendSpaceY() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FAnim_BlendSpaceSettings BlendSpaceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionSmoothingSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SpeedSmoothingSpeed = 8.0f;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    float SmoothedSpeed = 0.0f;
    float SmoothedDirection = 0.0f;
    FVector PreviousVelocity = FVector::ZeroVector;
};