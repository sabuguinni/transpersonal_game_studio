#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Raptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
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
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<TWeakObjectPtr<APawn>> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FNPC_PackBehavior PackBehavior;

    // Behavior Tree and Blackboard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Sensing Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UPawnSensingComponent* PawnSensingComponent;

    // Target and Navigation
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector PatrolCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius = 2000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float TimeSincePlayerSeen = 0.0f;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    ENPC_DinosaurState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(UNPC_DinosaurAI* PackLeaderAI);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CallPackToHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    TArray<APawn*> GetNearbyPackMembers(float Radius = 1000.0f);

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Species AI")
    void InitializeSpeciesTraits();

    UFUNCTION(BlueprintCallable, Category = "Species AI")
    bool ShouldFleeFromThreat(AActor* Threat) const;

    UFUNCTION(BlueprintCallable, Category = "Species AI")
    float GetOptimalAttackRange() const;

protected:
    // Internal AI Logic
    void UpdateAIState(float DeltaTime);
    void UpdateHunger(float DeltaTime);
    void UpdateFear(float DeltaTime);
    void ProcessSensedActors();
    
    // Sensing Callbacks
    UFUNCTION()
    void OnSeePawn(APawn* Pawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

    // State-specific updates
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrollingState(float DeltaTime);
    void UpdateHuntingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);
    void UpdateFeedingState(float DeltaTime);
    void UpdateTerritorialState(float DeltaTime);

    // Utility functions
    FVector GetRandomPatrolPoint() const;
    bool CanSeeTarget(AActor* Target) const;
    void UpdateBlackboardValues();

private:
    // Timers
    float StateChangeTimer = 0.0f;
    float HungerUpdateTimer = 0.0f;
    float NextPatrolPointTimer = 0.0f;
    
    // Cached references
    APawn* OwnerPawn;
    AAIController* AIController;
    UBlackboardComponent* BlackboardComponent;
};