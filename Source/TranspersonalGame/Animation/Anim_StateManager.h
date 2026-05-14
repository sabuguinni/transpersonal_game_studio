#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
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
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    EAnim_ActionState ActionState = EAnim_ActionState::None;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float StaminaRatio = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float HealthRatio = 1.0f;

    FAnim_StateData()
    {
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        StaminaRatio = 1.0f;
        HealthRatio = 1.0f;
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

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateMovementState(float CurrentSpeed, bool bIsMoving, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetActionState(EAnim_ActionState NewActionState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentStateData.MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    EAnim_ActionState GetCurrentActionState() const { return CurrentStateData.ActionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    FAnim_StateData GetCurrentStateData() const { return CurrentStateData; }

    // Animation Triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingActionMontage() const;

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsIdle() const { return CurrentStateData.MovementState == EAnim_MovementState::Idle; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsMoving() const { return CurrentStateData.Speed > 0.1f; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsRunning() const { return CurrentStateData.MovementState == EAnim_MovementState::Running; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsInAction() const { return CurrentStateData.ActionState != EAnim_ActionState::None; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_StateData CurrentStateData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_StateData PreviousStateData;

    // Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovementComponent;

    // State Transition
    void UpdateStateTransitions(float DeltaTime);
    void OnMovementStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);
    void OnActionStateChanged(EAnim_ActionState OldState, EAnim_ActionState NewState);

    // Internal Updates
    void UpdateMovementData();
    void UpdateHealthAndStamina();
    void CacheComponentReferences();
};