#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimationController.generated.h"

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
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ActionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_ActionState CurrentAction = EAnim_ActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    float ActionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bIsPerformingAction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    FString ActionTarget;

    FAnim_ActionData()
    {
        CurrentAction = EAnim_ActionState::None;
        ActionProgress = 0.0f;
        bIsPerformingAction = false;
        ActionTarget = TEXT("");
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Movement animation control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState GetMovementState() const { return MovementData.MovementState; }

    // Action animation control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_ActionState ActionType, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingActionMontage() const;

    // Animation data getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_ActionData GetActionData() const { return ActionData; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationBlueprint(TSubclassOf<UAnimInstance> NewAnimBP);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterAnimationMontage(EAnim_ActionState ActionType, UAnimMontage* Montage);

protected:
    // Animation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    FAnim_MovementData MovementData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    FAnim_ActionData ActionData;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TSubclassOf<UAnimInstance> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_ActionState, UAnimMontage*> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* MovementBlendSpace;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationSmoothingSpeed = 10.0f;

private:
    // Internal animation state
    float LastUpdateTime;
    FVector LastVelocity;
    bool bWasMovingLastFrame;

    // Helper functions
    void InitializeComponents();
    void UpdateMovementData(float DeltaTime);
    void UpdateActionData(float DeltaTime);
    EAnim_MovementState CalculateMovementState() const;
    float CalculateMovementDirection() const;
};