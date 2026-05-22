#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    Mating      UMETA(DisplayName = "Mating"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Drinking    UMETA(DisplayName = "Drinking")
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

UENUM(BlueprintType)
enum class ENPC_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Piscivore   UMETA(DisplayName = "Piscivore")
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
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Thirst = 50.0f;
        Stamina = 100.0f;
        MaxStamina = 100.0f;
        Aggression = 30.0f;
        Fear = 10.0f;
        TerritorialRadius = 2000.0f;
        DetectionRange = 1500.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceLastPlayerSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastFoodLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastWaterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastDamageTime = 0.0f;

    FNPC_DinosaurMemory()
    {
        LastSeenPlayerLocation = FVector::ZeroVector;
        TimeSinceLastPlayerSighting = 0.0f;
        HomeLocation = FVector::ZeroVector;
        LastFoodLocation = FVector::ZeroVector;
        LastWaterLocation = FVector::ZeroVector;
        CurrentPatrolIndex = 0;
        bPlayerIsHostile = false;
        LastDamageTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorManager : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurDiet Diet = ENPC_DinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    FNPC_DinosaurMemory Memory;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Movement Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float ChargeSpeed = 1000.0f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<ANPC_DinosaurBehaviorManager*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ANPC_DinosaurBehaviorManager* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackRadius = 1000.0f;

    // Time-based Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Behavior")
    float ActivityLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Behavior")
    float RestTime = 0.0f;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    ENPC_DinosaurState GetDinosaurState() const { return CurrentState; }

    // Stats Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Stats")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Stats")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Stats")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Stats")
    void ModifyStamina(float Amount);

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartFleeing(FVector DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartFeeding();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartDrinking();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    void UpdatePlayerMemory(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    void SetHomeLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Memory")
    void AddPatrolPoint(FVector Point);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ANPC_DinosaurBehaviorManager* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CallPackToLocation(FVector Location);

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Species-specific behavior setup
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Setup")
    void InitializeSpeciesTraits();

    // Daily routine management
    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void UpdateDailyRoutine(float TimeOfDay);

protected:
    // Internal state management
    void UpdateStats(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void ProcessCurrentState(float DeltaTime);
    
    // State-specific behavior
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrollingState(float DeltaTime);
    void ProcessHuntingState(float DeltaTime);
    void ProcessFeedingState(float DeltaTime);
    void ProcessFleeingState(float DeltaTime);
    void ProcessSleepingState(float DeltaTime);
    void ProcessTerritorialState(float DeltaTime);

    // Utility functions
    bool IsPlayerNearby() const;
    bool IsHungry() const { return Stats.Hunger > 70.0f; }
    bool IsThirsty() const { return Stats.Thirst > 70.0f; }
    bool IsTired() const { return Stats.Stamina < 30.0f; }
    bool IsInjured() const { return Stats.Health < Stats.MaxHealth * 0.5f; }
    
    FVector FindNearestWater() const;
    FVector FindNearestFood() const;
    AActor* FindNearestThreat() const;
    AActor* FindNearestPrey() const;

private:
    float StateTimer = 0.0f;
    float LastStatsUpdate = 0.0f;
    AActor* CurrentTarget = nullptr;
    FVector CurrentDestination = FVector::ZeroVector;
};