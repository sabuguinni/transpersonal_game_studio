#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

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
    PackHunting UMETA(DisplayName = "Pack Hunting")
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
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 600.0f;
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
    FVector PackCenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackTarget = nullptr;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_PackBehavior PackBehavior;

    // Behavior Tree assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* BlackboardComponent = nullptr;

    // Territory and patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Target tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    AActor* LastKnownThreat = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    // Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastFeedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastSleepTime = 0.0f;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* DetectNearbyThreats();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* DetectNearbyPrey();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StartPatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(UNPC_DinosaurBehaviorTree* PackLeaderComponent);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void InitiatePackHunt(AActor* Target);

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species")
    void ApplySpeciesTraits();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void TRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void VelociraptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void HerbivoreBehavior();

private:
    // Internal state management
    void UpdateBehaviorState(float DeltaTime);
    void UpdateStats(float DeltaTime);
    void CheckEnvironmentalFactors();
    void HandleStateTransitions();
    
    // Utility functions
    float GetDistanceToActor(AActor* Actor);
    bool IsActorInRange(AActor* Actor, float Range);
    FVector GetRandomLocationInRadius(FVector Center, float Radius);
    
    // Timer handles
    FTimerHandle StateUpdateTimer;
    FTimerHandle StatsUpdateTimer;
    FTimerHandle PatrolTimer;
};