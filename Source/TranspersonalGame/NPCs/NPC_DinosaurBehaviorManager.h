#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Following   UMETA(DisplayName = "Following Pack")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastPlayerSighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HomeTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastPlayerSighting = 0.0f;
        ThreatLevel = 0.0f;
        HomeTerritory = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Aggression = 30.0f;
        Speed = 600.0f;
        AttackDamage = 25.0f;
        DetectionRange = 3000.0f;
        AttackRange = 300.0f;
    }
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

    // Species and behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    FNPC_DinosaurMemory DinosaurMemory;

    // Behavior Tree and AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // Daily routine system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float DayNightCycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bIsNocturnal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float SleepStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float WakeUpTime;

    // Pack behavior (for Velociraptors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    UNPC_DinosaurBehaviorManager* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<UNPC_DinosaurBehaviorManager*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateDailyRoutine(float CurrentTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateMemory(AActor* PerceivedActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeSpeciesStats();

    // Pack behavior functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorManager* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CommunicateWithPack(ENPC_DinosaurBehaviorState AlertState);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackCenterLocation();

    // Territory and patrol functions
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetHomeTerritory(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInHomeTerritory();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetRandomPatrolPoint();

    // Combat and interaction
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void FindFood();

private:
    // Internal state management
    float StateTimer;
    float LastThreatCheckTime;
    float HungerDecayRate;
    AActor* CurrentTarget;
    FVector PatrolDestination;
    bool bHasPatrolDestination;

    // Helper functions
    void UpdateBlackboard();
    void ProcessSpeciesBehavior(float DeltaTime);
    AActor* FindNearestPlayer();
    float CalculateDistanceToPlayer();
    void HandleTRexBehavior(float DeltaTime);
    void HandleVelociraptorBehavior(float DeltaTime);
    void HandleHerbivoreBehavior(float DeltaTime);
};