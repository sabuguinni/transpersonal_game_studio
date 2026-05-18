#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Territorial     UMETA(DisplayName = "Territorial"),
    PackHunting     UMETA(DisplayName = "Pack Hunting")
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
    Pachycephalo    UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float RunSpeed = 1200.0f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Stamina = 100.0f;
        MaxStamina = 100.0f;
        Hunger = 50.0f;
        Aggression = 30.0f;
        Fear = 0.0f;
        TerritorialRadius = 5000.0f;
        DetectionRange = 3000.0f;
        AttackRange = 500.0f;
        MovementSpeed = 600.0f;
        RunSpeed = 1200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    float TimeSincePlayerSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    FVector PatrolTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    bool bPlayerDetected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Memory")
    float LastAttackTime = 0.0f;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        HomeLocation = FVector::ZeroVector;
        PatrolTarget = FVector::ZeroVector;
        bPlayerDetected = false;
        bInCombat = false;
        LastAttackTime = 0.0f;
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

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory DinosaurMemory;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaur(ENPC_DinosaurSpecies InSpecies, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ReturnToHome();

    // Pack Behavior (for Raptors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<AActor*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackHunt();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ApplySpeciesStats();

private:
    // Internal state management
    float StateTimer = 0.0f;
    float BehaviorUpdateInterval = 0.5f;
    float LastBehaviorUpdate = 0.0f;

    // Player reference
    APawn* PlayerPawn = nullptr;

    // Timer handles
    FTimerHandle PatrolTimerHandle;
    FTimerHandle HuntTimerHandle;
    FTimerHandle StateTimerHandle;

    // Internal behavior functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateFeedingBehavior(float DeltaTime);
    void UpdateSleepingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    void UpdatePackHuntingBehavior(float DeltaTime);

    void MoveTo(FVector TargetLocation);
    bool IsAtLocation(FVector Location, float Tolerance = 100.0f);
    void RotateTowards(FVector TargetLocation);
};