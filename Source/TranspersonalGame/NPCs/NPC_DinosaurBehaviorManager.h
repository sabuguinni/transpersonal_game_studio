#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "NavigationSystem.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

// Dinosaur behavior states for AI decision making
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
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial")
};

// Dinosaur species types with different behaviors
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

// Pack behavior data for group hunting
USTRUCT(BlueprintType)
struct FNPC_PackBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FVector PackCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsHunting;

    FNPC_PackBehaviorData()
    {
        PackMembers = TArray<AActor*>();
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        PackCenterLocation = FVector::ZeroVector;
        PackRadius = 2000.0f;
        bIsHunting = false;
    }
};

// Territory data for territorial dinosaurs
USTRUCT(BlueprintType)
struct FNPC_TerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bDefendingTerritory;

    FNPC_TerritoryData()
    {
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        PatrolPoints = TArray<FVector>();
        CurrentPatrolIndex = 0;
        bDefendingTerritory = false;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE BEHAVIOR PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float FleeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float Aggression;

    // === BEHAVIOR TREE AND AI ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AAIController* DinosaurAIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // === PACK BEHAVIOR ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FNPC_PackBehaviorData PackData;

    // === TERRITORY BEHAVIOR ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsTerritorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FNPC_TerritoryData TerritoryData;

    // === PERCEPTION ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    UPawnSensingComponent* PawnSensingComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LastSeenTargetTime;

    // === BEHAVIOR FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaurBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorTick(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToTarget(AActor* Target);

    // === PACK BEHAVIOR FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorManager* PackLeaderComponent);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackHunt(AActor* Target);

    // === TERRITORY FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetupTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void PatrolTerritory();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(AActor* Intruder);

    // === SPECIES-SPECIFIC BEHAVIORS ===

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void SetupTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void SetupVelociraptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void SetupHerbivoreBehavior();

    // === PERCEPTION CALLBACKS ===

    UFUNCTION()
    void OnSeePawn(APawn* Pawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

private:
    // Internal state tracking
    float StateChangeTime;
    FVector LastKnownTargetLocation;
    float IdleTimer;
    float HungerTimer;
    bool bInitialized;

    // Helper functions
    void UpdateHunger(float DeltaTime);
    void UpdateAggression(float DeltaTime);
    void HandleIdleBehavior();
    void HandlePatrolBehavior();
    void HandleHuntingBehavior();
    void HandleChasingBehavior();
    void HandleAttackingBehavior();
    void HandleFleeingBehavior();
    AActor* FindNearestPlayer();
    AActor* FindNearestPrey();
    void MoveToLocation(FVector TargetLocation);
    bool IsPlayerInRange(float Range);
};