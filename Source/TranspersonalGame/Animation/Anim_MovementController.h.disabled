#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MovementController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_LocomotionMode : uint8
{
    Ground      UMETA(DisplayName = "Ground"),
    Air         UMETA(DisplayName = "Air"),
    Water       UMETA(DisplayName = "Water"),
    Climbing    UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_LocomotionMode LocomotionMode = EAnim_LocomotionMode::Ground;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_MovementState::Idle;
        LocomotionMode = EAnim_LocomotionMode::Ground;
        Velocity = FVector::ZeroVector;
        GroundDistance = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement data calculation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementData CalculateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState DetermineMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float CalculateDirection();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsCharacterInAir();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetGroundDistance();

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerCrouchAnimation(bool bCrouch);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetCurrentSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentState() const { return MovementData.CurrentState; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* MeshComponent;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float MovingThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float GroundTraceDistance = 200.0f;

private:
    void CacheComponents();
    void UpdateMovementValues();
    void UpdateLocomotionMode();
    float CalculateSpeedRatio();
};