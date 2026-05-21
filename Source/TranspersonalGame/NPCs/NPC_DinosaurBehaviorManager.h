#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
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
    PackHunt    UMETA(DisplayName = "Pack Hunt")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
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
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 500.0f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Aggression = 30.0f;
        Fear = 10.0f;
        Stamina = 100.0f;
        Speed = 600.0f;
        DetectionRange = 3000.0f;
        AttackRange = 500.0f;
    }
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
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TerritoryRadius = 5000.0f;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsTerritorial = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorUpdateInterval = 1.0f;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeSpeciesStats();

private:
    FTimerHandle BehaviorUpdateTimer;
    AActor* CurrentTarget = nullptr;
    int32 CurrentPatrolIndex = 0;
    
    void ExecuteIdleBehavior(float DeltaTime);
    void ExecutePatrolBehavior(float DeltaTime);
    void ExecuteHuntingBehavior(float DeltaTime);
    void ExecuteFeedingBehavior(float DeltaTime);
    void ExecuteFleeingBehavior(float DeltaTime);
    void ExecuteSleepingBehavior(float DeltaTime);
    void ExecuteTerritorialBehavior(float DeltaTime);
    void ExecutePackHuntBehavior(float DeltaTime);
    
    void MoveToLocation(FVector TargetLocation);
    bool IsAtLocation(FVector Location, float Tolerance = 200.0f);
    void GeneratePatrolPoints();
};

UCLASS()
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorManager : public AActor
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager")
    TArray<AActor*> ManagedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager")
    float GlobalBehaviorUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager")
    int32 MaxActiveBehaviors = 50;

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void RegisterDinosaur(AActor* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void UnregisterDinosaur(AActor* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void UpdateAllBehaviors();

    UFUNCTION(BlueprintCallable, Category = "Manager")
    TArray<AActor*> GetDinosaursInRange(FVector Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void SetGlobalAggressionLevel(float AggressionMultiplier);

private:
    FTimerHandle GlobalUpdateTimer;
    float GlobalAggressionMultiplier = 1.0f;
    
    void OptimizeBehaviorUpdates();
    void HandlePackBehavior();
    void HandleTerritorialConflicts();
};