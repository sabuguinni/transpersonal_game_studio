#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Math/Vector.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle_Grazing        UMETA(DisplayName = "Idle Grazing"),
    Walking_Patrol      UMETA(DisplayName = "Walking Patrol"),
    Running_Chase       UMETA(DisplayName = "Running Chase"),
    Attack_Bite         UMETA(DisplayName = "Attack Bite"),
    Death_Collapse      UMETA(DisplayName = "Death Collapse"),
    Roar_Intimidate     UMETA(DisplayName = "Roar Intimidate"),
    Fleeing_Panic       UMETA(DisplayName = "Fleeing Panic"),
    Sleeping_Rest       UMETA(DisplayName = "Sleeping Rest")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex                UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector LastPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bPlayerIsHostile;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 CurrentPatrolIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float LastFeedTime;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float LastRestTime;

    FNPC_DinosaurMemory()
    {
        LastPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 999.0f;
        bPlayerIsHostile = false;
        CurrentPatrolIndex = 0;
        LastFeedTime = 0.0f;
        LastRestTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Health;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Hunger;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Energy;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Aggression;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Fear;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float MovementSpeed;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float DetectionRange;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float AttackRange;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float AttackDamage;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Energy = 100.0f;
        Aggression = 30.0f;
        Fear = 10.0f;
        MovementSpeed = 300.0f;
        DetectionRange = 2000.0f;
        AttackRange = 200.0f;
        AttackDamage = 25.0f;
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
    // Core behavior state
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurMemory Memory;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float StateUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ChaseTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bIsHerbivore;

    // Pack behavior
    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TArray<UNPC_DinosaurBehaviorTree*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector PackRallyPoint;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetDistanceToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AttackTarget(AActor* Target);

    // State-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteIdleGrazing();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteWalkingPatrol();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteRunningChase();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteAttackBite();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteRoarIntimidate();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteFleeingPanic();

    UFUNCTION(BlueprintCallable, Category = "States")
    void ExecuteSleepingRest();

    // Pack behavior functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(UNPC_DinosaurBehaviorTree* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void FormPack(TArray<UNPC_DinosaurBehaviorTree*> Members);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CallPackToRally();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackHunt(AActor* Target);

    // Memory and decision making
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetNextPatrolPoint();

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species")
    void ConfigureForSpecies(ENPC_DinosaurSpecies DinosaurSpecies);

    // Animation integration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAnimationState(ENPC_DinosaurState StateToAnimate);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle StateTransitionTimer;
    
    APawn* OwnerPawn;
    AActor* PlayerActor;
    
    FVector HomeLocation;
    FVector CurrentTarget;
    
    float StateStartTime;
    float LastPlayerDetectionTime;
    
    void FindPlayerActor();
    void InitializeSpeciesDefaults();
    bool ShouldTransitionState();
    ENPC_DinosaurState DetermineNextState();
    void OnStateTransition(ENPC_DinosaurState OldState, ENPC_DinosaurState NewState);
};