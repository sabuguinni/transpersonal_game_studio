#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "SharedTypes.h"
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
    Mating      UMETA(DisplayName = "Mating")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Pteranodon      UMETA(DisplayName = "Pteranodon")
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
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 300.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<class ANPC_DinosaurBehaviorTree*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    class ANPC_DinosaurBehaviorTree* PackLeader = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorTree : public APawn
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBlackboardComponent* BlackboardComponent;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_PackBehavior PackBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    class UBehaviorTree* DinosaurBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    class UBlackboardData* DinosaurBlackboard;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestPrey() const;

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ANPC_DinosaurBehaviorTree* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    FVector GetPackCenterLocation() const;

    // Stats Management
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateThirst(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsAlive() const { return DinosaurStats.Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsHungry() const { return DinosaurStats.Hunger > 70.0f; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsThirsty() const { return DinosaurStats.Thirst > 70.0f; }

protected:
    // Internal behavior management
    void InitializeBehaviorTree();
    void UpdateBehaviorState(float DeltaTime);
    void HandleStateTransitions();
    
    // Species-specific behavior initialization
    void InitializeSpeciesBehavior();
    void SetupTRexBehavior();
    void SetupVelociraptorBehavior();
    void SetupHerbivoreBehavior();

    // Territory and patrol points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastStateChangeTime = 0.0f;

    // Timers for behavior updates
    float HungerUpdateTimer = 0.0f;
    float ThirstUpdateTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;

private:
    // Internal state management
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrollingState(float DeltaTime);
    void ProcessHuntingState(float DeltaTime);
    void ProcessFleeingState(float DeltaTime);
    void ProcessFeedingState(float DeltaTime);
    void ProcessTerritorialState(float DeltaTime);
};