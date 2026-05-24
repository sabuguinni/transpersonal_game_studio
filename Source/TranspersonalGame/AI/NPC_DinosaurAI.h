#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Sleeping       UMETA(DisplayName = "Sleeping"),
    Territorial    UMETA(DisplayName = "Territorial"),
    PackHunting    UMETA(DisplayName = "Pack Hunting")
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
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;
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
    float LastPlayerSightingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> PackMembers;
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

    // Core AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ENPC_DinosaurBehaviorState CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FNPC_DinosaurMemory DinosaurMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    float MinStateTime = 5.0f;

    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    AActor* FindNearestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdatePatrolRoute();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeSpeciesTraits();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdatePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ProcessDayNightCycle();

private:
    // Internal AI processing
    void ProcessIdleBehavior(float DeltaTime);
    void ProcessPatrolBehavior(float DeltaTime);
    void ProcessHuntingBehavior(float DeltaTime);
    void ProcessFleeingBehavior(float DeltaTime);
    void ProcessFeedingBehavior(float DeltaTime);
    void ProcessSleepingBehavior(float DeltaTime);
    void ProcessTerritorialBehavior(float DeltaTime);
    void ProcessPackHuntingBehavior(float DeltaTime);

    void UpdateStats(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    bool ShouldChangeState() const;
    ENPC_DinosaurBehaviorState DetermineNextState() const;

    // Cached references
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    ACharacter* PlayerCharacter;

    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

    // Internal timers and state
    float LastBehaviorUpdate = 0.0f;
    float HungerDecayRate = 1.0f;
    float StaminaRegenRate = 10.0f;
    int32 CurrentPatrolIndex = 0;
    bool bHasValidTarget = false;
    AActor* CurrentTarget = nullptr;
};