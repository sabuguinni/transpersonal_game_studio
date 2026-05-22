#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "GameFramework/Character.h"
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
    Mating      UMETA(DisplayName = "Mating")
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
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;
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
    float LastPlayerSightTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastFeedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastRestTime = 0.0f;
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

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    FNPC_DinosaurMemory Memory;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Time Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // 20 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 0.0f;

    // Pack Behavior (for social species)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<UNPC_DinosaurBehaviorManager*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    UNPC_DinosaurBehaviorManager* PackLeader;

    // Core Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessDailyRoutine(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestFood();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemory(AActor* Actor, bool bIsThreat);

    // Movement and Navigation
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void PatrolArea();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ReturnToHome();

    // Combat Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange(AActor* Target);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(UNPC_DinosaurBehaviorManager* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackHunt(AActor* Target);

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Species")
    void InitializeSpeciesTraits();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void ApplySpeciesBehavior();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsNightTime();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void PlayDinosaurSound(const FString& SoundType);

private:
    // Internal state management
    float StateTimer = 0.0f;
    float LastUpdateTime = 0.0f;
    bool bIsInitialized = false;

    // AI Controller reference
    AAIController* AIControllerRef;

    // Player reference
    APawn* PlayerPawn;

    // Helper functions
    void InitializeAI();
    void UpdatePerception(float DeltaTime);
    void HandleStateTransition();
    void UpdateBlackboard();
    void ProcessSpeciesSpecificBehavior(float DeltaTime);
    void HandlePackCommunication();
    void UpdateTimeOfDay(float DeltaTime);
};