#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    Mating      UMETA(DisplayName = "Mating"),
    Injured     UMETA(DisplayName = "Injured"),
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
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalo    UMETA(DisplayName = "Pachycephalo"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 300.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastPlayerSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsKnownThreat = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory Memory;

    // Behavior timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float StateChangeInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float FleeSpeed = 800.0f;

    // AI References
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* DinosaurAIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UPawnSensingComponent* PawnSensingComponent;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ProcessPerception(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ExecuteCurrentBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AddThreatToMemory(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void RemoveThreatFromMemory(AActor* Threat);

    // Species-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteTRexBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteVelociraptorBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteHerbivoreBehavior(float DeltaTime);

    // State transition functions
    UFUNCTION(BlueprintCallable, Category = "State Transitions")
    void TransitionToIdle();

    UFUNCTION(BlueprintCallable, Category = "State Transitions")
    void TransitionToPatrolling();

    UFUNCTION(BlueprintCallable, Category = "State Transitions")
    void TransitionToHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "State Transitions")
    void TransitionToFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "State Transitions")
    void TransitionToFeeding();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void UpdateBlackboardValues();

private:
    float TimeSinceLastStateChange = 0.0f;
    float TimeSinceLastStatsUpdate = 0.0f;
    AActor* CurrentTarget = nullptr;
    int32 CurrentPatrolIndex = 0;

    void InitializeSpeciesDefaults();
    void SetupAIReferences();
    bool ShouldChangeState() const;
    void HandleStateTransitions();
    void UpdateMovement(float DeltaTime);
    void UpdatePerceptionData();
};