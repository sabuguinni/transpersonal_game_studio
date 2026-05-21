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
    Idle,
    Patrolling,
    Hunting,
    Fleeing,
    Feeding,
    Sleeping,
    Territorial
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex,
    Velociraptor,
    Triceratops,
    Brachiosaurus,
    Ankylosaurus,
    Parasaurolophus
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 5000.0f;
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
    FVector HomeTerritory = FVector::ZeroVector;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float MovementSpeed = 600.0f;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateStats(float DeltaTime);

private:
    FTimerHandle BehaviorUpdateTimer;
    AActor* CurrentTarget = nullptr;
    FVector CurrentDestination = FVector::ZeroVector;
    
    void UpdateBehavior();
    void ProcessIdleState();
    void ProcessPatrolState();
    void ProcessHuntingState();
    void ProcessFleeingState();
    AActor* FindNearestPlayer();
    void MoveToLocation(FVector TargetLocation);
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorManager : public AActor
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Manager")
    TArray<AActor*> ManagedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Manager")
    float GlobalAggressionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Manager")
    bool bEnablePackBehavior = true;

    UFUNCTION(BlueprintCallable, Category = "Behavior Manager")
    void RegisterDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Behavior Manager")
    void UnregisterDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Behavior Manager")
    void UpdateGlobalBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior Manager")
    TArray<AActor*> GetDinosaursInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Behavior Manager")
    void TriggerTerritorialResponse(FVector Location, float Radius);

private:
    FTimerHandle GlobalUpdateTimer;
    
    void ProcessPackBehavior();
    void ProcessTerritorialBehavior();
    void UpdateDinosaurInteractions();
};