#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MovementStateMachine.generated.h"

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
    float GroundDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
        CurrentState = EAnim_MovementState::Idle;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MovementStateMachine : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MovementStateMachine();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateMovementState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    EAnim_MovementState GetCurrentState() const { return MovementData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    // State transitions
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void TransitionToState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool CanTransitionTo(EAnim_MovementState TargetState) const;

    // Movement analysis
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void AnalyzeMovement(class UCharacterMovementComponent* MovementComponent);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void CalculateGroundDistance();

protected:
    // Core movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMesh;

    // State transition thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds", meta = (AllowPrivateAccess = "true"))
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds", meta = (AllowPrivateAccess = "true"))
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds", meta = (AllowPrivateAccess = "true"))
    float JumpVelocityThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds", meta = (AllowPrivateAccess = "true"))
    float GroundTraceDistance = 200.0f;

    // State timing
    UPROPERTY(BlueprintReadOnly, Category = "State Timing", meta = (AllowPrivateAccess = "true"))
    float StateTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State Timing", meta = (AllowPrivateAccess = "true"))
    float LastStateChangeTime = 0.0f;

private:
    // Internal state tracking
    EAnim_MovementState PreviousState = EAnim_MovementState::Idle;
    float MinStateTime = 0.1f; // Minimum time to stay in a state
    
    // Helper functions
    EAnim_MovementState DetermineMovementState() const;
    bool IsValidStateTransition(EAnim_MovementState From, EAnim_MovementState To) const;
    void OnStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);
};