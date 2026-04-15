#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "AnimationStateManager.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UAnimSequence;
class UBlendSpace1D;
class UBlendSpace;

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
    Crawling    UMETA(DisplayName = "Crawling"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Combat      UMETA(DisplayName = "Combat"),
    Injured     UMETA(DisplayName = "Injured"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Carrying        UMETA(DisplayName = "Carrying"),
    Throwing        UMETA(DisplayName = "Throwing")
};

USTRUCT(BlueprintType)
struct FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bCanInterrupt;

    FAnim_StateTransition()
    {
        FromState = EAnim_MovementState::Idle;
        ToState = EAnim_MovementState::Walking;
        TransitionDuration = 0.2f;
        bCanInterrupt = true;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsMoving = false;
        GroundDistance = 0.0f;
        Velocity = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationStateManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // State management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    // Animation control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingMontage() const;

    // Movement data update
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float Speed, float Direction, bool bInAir, const FVector& Velocity);

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetGroundDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableTerrainAdaptation(bool bEnable);

protected:
    // Current states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_ActionState CurrentActionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_MovementState PreviousMovementState;

    // Movement data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    FAnim_MovementData MovementData;

    // State transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FAnim_StateTransition> StateTransitions;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TMap<EAnim_MovementState, UAnimSequence*> MovementAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TMap<EAnim_ActionState, UAnimMontage*> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UBlendSpace1D* MovementBlendSpace;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Terrain adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bTerrainAdaptationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float MaxGroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float TerrainAdaptationSpeed;

private:
    // Internal state management
    bool CanTransitionToState(EAnim_MovementState NewState) const;
    void InitializeDefaultTransitions();
    void UpdateAnimationBlending(float DeltaTime);

    // Transition timing
    float StateTransitionTimer;
    bool bIsTransitioning;
};