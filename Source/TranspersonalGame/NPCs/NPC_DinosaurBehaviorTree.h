#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BehaviorTree/Services/BTService.h"
#include "BehaviorTree/Decorators/BTDecorator.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "../Core/SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAIController;

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Territorial UMETA(DisplayName = "Territorial"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector PatrolTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LastStateChangeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    FNPC_DinosaurBehaviorData()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        CurrentState = ENPC_DinosaurState::Idle;
        PatrolRadius = 5000.0f;
        ChaseRadius = 3000.0f;
        AttackRadius = 300.0f;
        MovementSpeed = 400.0f;
        Aggression = 0.5f;
        Hunger = 50.0f;
        Health = 100.0f;
        HomeLocation = FVector::ZeroVector;
        PatrolTarget = FVector::ZeroVector;
        TargetActor = nullptr;
        LastStateChangeTime = 0.0f;
        StateTimer = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurBehaviorData BehaviorData;

    // Behavior tree references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // AI Controller reference
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* AIControllerRef;

    // State management functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    ENPC_DinosaurState GetDinosaurState() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeBehavior(ENPC_DinosaurSpecies InSpecies, FVector InHomeLocation);

    // Behavior logic functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorLogic(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void GeneratePatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AttackTarget();

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ConfigureSpeciesBehavior();

    // Blackboard key management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardVector(const FString& KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardFloat(const FString& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardBool(const FString& KeyName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardObject(const FString& KeyName, UObject* Value);

private:
    // Internal state tracking
    float TimeSinceLastUpdate;
    FVector LastKnownPlayerLocation;
    bool bPlayerDetected;
    float PatrolWaitTime;

    // Helper functions
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleChasingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleFeedingState(float DeltaTime);
    void HandleRestingState(float DeltaTime);
    void HandleTerritorialState(float DeltaTime);

    void TransitionToState(ENPC_DinosaurState NewState);
    bool CanTransitionToState(ENPC_DinosaurState TargetState) const;
    void OnStateEnter(ENPC_DinosaurState EnteredState);
    void OnStateExit(ENPC_DinosaurState ExitedState);
};