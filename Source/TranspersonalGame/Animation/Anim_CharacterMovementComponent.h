#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_CharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        Velocity = FVector::ZeroVector;
        GroundDistance = 0.0f;
    }
};

/**
 * Enhanced Character Movement Component with animation-specific data
 * Provides movement state tracking for animation systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation data getter
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    // Movement state queries
    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMoving() const { return MovementData.Speed > 0.1f; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsRunning() const { return MovementData.Speed > WalkToRunThreshold; }

    // Animation event triggers
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnMovementStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnLanded(const FHitResult& Hit);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnJumped();

protected:
    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Thresholds")
    float WalkToRunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Thresholds")
    float IdleThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Thresholds")
    float GroundTraceDistance = 200.0f;

private:
    // Current movement data
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Previous state for change detection
    EAnim_MovementState PreviousMovementState = EAnim_MovementState::Idle;

    // Update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateGroundDistance();
    EAnim_MovementState CalculateMovementState() const;

    // Event handlers
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
    virtual void OnLanded(const FHitResult& Hit) override;
};

#include "Anim_CharacterMovementComponent.generated.h"