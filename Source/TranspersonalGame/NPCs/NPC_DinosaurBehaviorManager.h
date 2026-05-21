#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
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
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

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
    float TimeSincePlayerSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastFeedingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bHasSeenPlayer = false;
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
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    TArray<UNPC_DinosaurBehaviorManager*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsPackLeader = false;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaurBehavior(ENPC_DinosaurSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorTick(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartFleeing(FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateStamina(float DeltaTime);

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorManager* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BroadcastToPackMembers(ENPC_DinosaurState NewState, FVector TargetLocation);

private:
    // Internal state tracking
    float StateTimer = 0.0f;
    int32 CurrentPatrolIndex = 0;
    AActor* CurrentTarget = nullptr;
    FVector FleeDirection = FVector::ZeroVector;
    
    // Timers
    float HungerUpdateTimer = 0.0f;
    float StaminaUpdateTimer = 0.0f;
    float PlayerDetectionTimer = 0.0f;

    // Helper functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrollingBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateFeedingBehavior(float DeltaTime);
    void UpdateSleepingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    
    void GeneratePatrolPoints();
    FVector GetNextPatrolPoint();
    bool IsInTerritorialRange(FVector Location) const;
    void MoveToLocation(FVector TargetLocation);
    
    // Species-specific behavior initialization
    void InitializeTRexBehavior();
    void InitializeVelociraptorBehavior();
    void InitializeTriceratopsBehavior();
    void InitializeBrachiosaurusBehavior();
    void InitializeAnkylosaurusBehavior();
};