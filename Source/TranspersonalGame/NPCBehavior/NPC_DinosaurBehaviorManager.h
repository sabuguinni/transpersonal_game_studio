#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Patrolling   UMETA(DisplayName = "Patrolling"), 
    Hunting      UMETA(DisplayName = "Hunting"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Feeding      UMETA(DisplayName = "Feeding"),
    Sleeping     UMETA(DisplayName = "Sleeping"),
    Territorial  UMETA(DisplayName = "Territorial"),
    PackHunting  UMETA(DisplayName = "Pack Hunting")
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
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HearingRange = 2000.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector PackCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackTarget = nullptr;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_PackBehavior PackData;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Targets and Threats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Targets")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Targets")
    AActor* CurrentThreat = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Targets")
    TArray<AActor*> KnownThreats;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaurBehavior(ENPC_DinosaurSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateDinosaurStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(FVector Center);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "AI Targets")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI Targets")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "AI Targets")
    void RemoveThreat(AActor* ThreatActor);

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorManager* PackLeaderComponent);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitiatePackHunt(AActor* Target);

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteVelociraptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteHerbivoreBehavior();

    // State Machine Functions
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ProcessIdleState();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ProcessPatrollingState();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ProcessHuntingState();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ProcessFleeingState();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetRandomPointInTerritory() const;

private:
    // Internal state tracking
    float StateTimer = 0.0f;
    float LastStateChange = 0.0f;
    bool bInitialized = false;
    
    // Cached references
    UPROPERTY()
    APawn* CachedPlayerPawn = nullptr;

    UPROPERTY()
    AAIController* CachedAIController = nullptr;

    // Internal helper functions
    void CacheReferences();
    void UpdateStateTimer(float DeltaTime);
    bool ShouldChangeState() const;
    void ApplySpeciesDefaults();
    void ValidatePackData();
};