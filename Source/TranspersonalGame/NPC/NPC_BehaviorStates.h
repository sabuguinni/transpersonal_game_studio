#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPC_BehaviorStates.generated.h"

/**
 * Defines behavior states for NPCs and dinosaurs
 * Each state has entry/exit actions and update logic
 */
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Territorial UMETA(DisplayName = "Territorial"),
    Pack        UMETA(DisplayName = "Pack Behavior")
};

/**
 * State transition conditions for behavior trees
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    ENPC_BehaviorState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    ENPC_BehaviorState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TriggerHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bRequiresLineOfSight;

    FNPC_StateTransition()
    {
        FromState = ENPC_BehaviorState::Idle;
        ToState = ENPC_BehaviorState::Patrol;
        TriggerDistance = 1000.0f;
        TriggerHealth = 50.0f;
        bRequiresLineOfSight = true;
    }
};

/**
 * Memory system for NPCs to remember players and events
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bWasHostile;

    FNPC_Memory()
    {
        RememberedActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        MemoryStrength = 1.0f;
        TimeLastSeen = 0.0f;
        bWasHostile = false;
    }
};

/**
 * Component that manages NPC behavior states and transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorStateComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    // Previous state for transition logic
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    ENPC_BehaviorState PreviousState;

    // Available state transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FNPC_StateTransition> StateTransitions;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 MaxMemories;

    // State management functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ChangeState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanTransitionTo(ENPC_BehaviorState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(AActor* Actor, const FVector& Location, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_Memory* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryStrength(float DeltaTime);

protected:
    // Internal state management
    void OnStateEnter(ENPC_BehaviorState NewState);
    void OnStateExit(ENPC_BehaviorState OldState);
    void UpdateCurrentState(float DeltaTime);

private:
    float TimeInCurrentState;
    float LastStateChangeTime;
};