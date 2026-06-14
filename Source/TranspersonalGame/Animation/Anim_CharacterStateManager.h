#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Anim_CharacterStateManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_CharacterState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Walking      UMETA(DisplayName = "Walking"),
    Running      UMETA(DisplayName = "Running"),
    Jumping      UMETA(DisplayName = "Jumping"),
    Falling      UMETA(DisplayName = "Falling"),
    Landing      UMETA(DisplayName = "Landing"),
    Crouching    UMETA(DisplayName = "Crouching"),
    Climbing     UMETA(DisplayName = "Climbing"),
    Swimming     UMETA(DisplayName = "Swimming"),
    Combat       UMETA(DisplayName = "Combat"),
    Injured      UMETA(DisplayName = "Injured"),
    Dead         UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_CharacterState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_CharacterState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bCanInterrupt;

    FAnim_StateTransition()
    {
        FromState = EAnim_CharacterState::Idle;
        ToState = EAnim_CharacterState::Walking;
        TransitionDuration = 0.2f;
        bCanInterrupt = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterStateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetCharacterState(EAnim_CharacterState NewState, bool bForceTransition = false);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_CharacterState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_CharacterState GetPreviousState() const { return PreviousState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsInTransition() const { return bIsTransitioning; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    float GetTransitionProgress() const { return TransitionProgress; }

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsGrounded() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool CanTransitionTo(EAnim_CharacterState TargetState) const;

    // Movement Data
    UFUNCTION(BlueprintCallable, Category = "Animation Data")
    void UpdateMovementData(float Speed, FVector Velocity, bool bIsGrounded, bool bIsFalling);

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetMovementSpeed() const { return MovementSpeed; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FVector GetMovementDirection() const { return MovementDirection; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_CharacterState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_CharacterState PreviousState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    bool bIsTransitioning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    float TransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    TArray<FAnim_StateTransition> StateTransitions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    float MovementSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    FVector MovementDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    bool bGrounded;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Data")
    bool bFalling;

private:
    float TransitionTimer;
    float CurrentTransitionDuration;

    void InitializeDefaultTransitions();
    bool CanTransitionFromTo(EAnim_CharacterState From, EAnim_CharacterState To) const;
    void UpdateTransition(float DeltaTime);
    EAnim_CharacterState DetermineStateFromMovement() const;
};