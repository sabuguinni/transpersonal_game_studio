#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Anim_StateManager.generated.h"

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
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Interacting     UMETA(DisplayName = "Interacting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_ActionState ActionState = EAnim_ActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsCrouched = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float GroundDistance = 0.0f;

    FAnim_StateData()
    {
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouched = false;
        GroundDistance = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_StateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_StateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core state management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetActionState(EAnim_ActionState NewActionState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    FAnim_StateData GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_MovementState GetMovementState() const { return CurrentState.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_ActionState GetActionState() const { return CurrentState.ActionState; }

    // State transition checks
    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool CanTransitionTo(EAnim_MovementState NewState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void ForceMovementState(EAnim_MovementState NewState);

    // Animation event handlers
    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpStarted();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnLanded();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnCrouchStarted();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnCrouchEnded();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_StateData CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_StateData PreviousState;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // State transition timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float StateTransitionDelay = 0.1f;

    UPROPERTY()
    float LastStateChangeTime = 0.0f;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float GroundTraceDistance = 120.0f;

private:
    void CacheComponentReferences();
    void UpdateGroundDistance();
    EAnim_MovementState CalculateMovementState() const;
    bool IsValidStateTransition(EAnim_MovementState From, EAnim_MovementState To) const;
};