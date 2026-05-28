#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Feeding UMETA(DisplayName = "Feeding"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunting UMETA(DisplayName = "Pack Hunting")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
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
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 500.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector PackCenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius = 2000.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core AI State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Stats")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Pack")
    FNPC_PackData PackData;

    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    AActor* FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    AActor* FindNearestPrey();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    bool IsPlayerInRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdatePackBehavior();

    // Territory and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    AActor* CurrentTarget = nullptr;

    // Timers
    UPROPERTY()
    FTimerHandle StateUpdateTimer;

    UPROPERTY()
    FTimerHandle PackUpdateTimer;

    UPROPERTY()
    FTimerHandle HungerTimer;

private:
    void UpdateAIState();
    void UpdateHunger();
    void InitializeSpeciesStats();
    void GeneratePatrolPoints();
    void MoveToNextPatrolPoint();
    bool IsInTerritory(const FVector& Location) const;
    float GetDistanceToPlayer() const;
    void HandleCombatBehavior();
    void HandlePackCommunication();
};