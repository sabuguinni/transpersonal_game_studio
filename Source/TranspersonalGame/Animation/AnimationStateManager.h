#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "AnimationStateManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
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
struct TRANSPERSONALGAME_API FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* TransitionMontage;

    FAnim_StateTransition()
    {
        FromState = EAnim_MovementState::Idle;
        ToState = EAnim_MovementState::Walking;
        TransitionDuration = 0.2f;
        TransitionMontage = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundDistance;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado actual do movimento
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    // Estado actual da acção
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_ActionState CurrentActionState;

    // Dados de movimento para animação
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData MovementData;

    // Transições configuradas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    TArray<FAnim_StateTransition> StateTransitions;

    // Montagens de acção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_ActionState, class UAnimMontage*> ActionMontages;

    // Blend spaces para movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    // Funções públicas para controlo de estado
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayActionMontage(EAnim_ActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopCurrentAction();

    UFUNCTION(BlueprintCallable, Category = "Animation Data")
    void UpdateMovementData(float Speed, float Direction, bool bInAir, bool bCrouching);

    UFUNCTION(BlueprintPure, Category = "Animation Query")
    bool CanTransitionTo(EAnim_MovementState TargetState) const;

    UFUNCTION(BlueprintPure, Category = "Animation Query")
    float GetTransitionDuration(EAnim_MovementState FromState, EAnim_MovementState ToState) const;

private:
    // Referência para o componente de animação
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    // Timer para transições
    float TransitionTimer;
    bool bIsTransitioning;
    EAnim_MovementState PendingState;

    // Funções internas
    void InitializeDefaultTransitions();
    void ProcessStateTransition(float DeltaTime);
    void UpdateAnimationInstance();
    FAnim_StateTransition* FindTransition(EAnim_MovementState From, EAnim_MovementState To);
};