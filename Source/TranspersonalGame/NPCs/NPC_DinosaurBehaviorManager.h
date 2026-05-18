#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "../SharedTypes.h"
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
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 500.0f;

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
    FVector PackCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesion = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackTarget = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_PackBehavior PackBehavior;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset = nullptr;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Target Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    FVector LastKnownLocation = FVector::ZeroVector;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaurBehavior(ENPC_DinosaurSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateFear(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ClearTarget();

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorManager* PackLeaderComponent);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackLeader() const;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void GeneratePatrolPoints(int32 NumPoints, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location);

    // Species-Specific Behaviors
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ApplySpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void HandleTRexBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void HandleVelociraptorBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void HandleHerbivoreBehavior(float DeltaTime);

private:
    // Internal state tracking
    float StateTimer = 0.0f;
    float HungerDecayRate = 1.0f;
    float FearDecayRate = 2.0f;
    bool bHasInitialized = false;

    // Helper functions
    void UpdateBehaviorState(float DeltaTime);
    void ProcessPerception(float DeltaTime);
    void HandleStateTransitions();
    AActor* FindNearestPlayer();
    AActor* FindNearestPrey();
    bool ShouldFlee();
    bool ShouldHunt();
    void UpdateBlackboard();
};