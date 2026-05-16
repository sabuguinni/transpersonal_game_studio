#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "../SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

// Forward declarations
class UCombat_DamageSystem;
class UCombat_CombatManager;

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus"),
    Compsognathus  UMETA(DisplayName = "Compsognathus")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurBehaviorState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Investigating  UMETA(DisplayName = "Investigating"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Attacking      UMETA(DisplayName = "Attacking"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Territorial    UMETA(DisplayName = "Territorial"),
    PackHunting    UMETA(DisplayName = "Pack Hunting"),
    Wounded        UMETA(DisplayName = "Wounded")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurAggressionLevel : uint8
{
    Passive        UMETA(DisplayName = "Passive"),
    Cautious       UMETA(DisplayName = "Cautious"),
    Defensive      UMETA(DisplayName = "Defensive"),
    Aggressive     UMETA(DisplayName = "Aggressive"),
    Enraged        UMETA(DisplayName = "Enraged")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PackCoordinationRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FearThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HungerRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsPackHunter = false;

    FCombat_DinosaurStats()
    {
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        MovementSpeed = 400.0f;
        DetectionRange = 2000.0f;
        AttackRange = 200.0f;
        TerritoryRadius = 5000.0f;
        PackCoordinationRange = 1500.0f;
        FearThreshold = 30.0f;
        HungerRate = 1.0f;
        bIsPackHunter = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector RallyPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesion = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsHunting = false;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        RallyPoint = FVector::ZeroVector;
        PackCohesion = 0.8f;
        bIsHunting = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurStateChanged, ECombat_DinosaurBehaviorState, OldState, ECombat_DinosaurBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurTargetAcquired, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurAttack, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDinosaurRoar);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurSpecies Species = ECombat_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurBehaviorState CurrentState = ECombat_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurAggressionLevel AggressionLevel = ECombat_DinosaurAggressionLevel::Cautious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FCombat_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FCombat_PackCoordination PackData;

    // Current Status
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float CurrentHunger = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float CurrentFear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float TimeSinceLastTargetSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FVector PatrolTarget = FVector::ZeroVector;

    // Timers
    UPROPERTY(BlueprintReadOnly, Category = "Timers")
    float StateTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Timers")
    float AttackCooldown = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Timers")
    float RoarCooldown = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Timers")
    float LastAttackTime = 0.0f;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnDinosaurStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnDinosaurTargetAcquired OnTargetAcquired;

    UPROPERTY(BlueprintAssignable)
    FOnDinosaurAttack OnAttack;

    UPROPERTY(BlueprintAssignable)
    FOnDinosaurRoar OnRoar;

    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetBehaviorState(ECombat_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInTerritoryRange(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void PerformRoar();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ModifyHunger(float HungerChange);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(UCombat_DinosaurAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AddPackMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    FVector GetPackRallyPoint() const;

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species AI")
    void InitializeSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Species AI")
    void UpdateSpeciesBehavior(float DeltaTime);

protected:
    // Internal AI Logic
    void UpdateAI(float DeltaTime);
    void UpdateStateLogic(float DeltaTime);
    void UpdateTargetTracking(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void UpdateSurvivalNeeds(float DeltaTime);

    // State-specific updates
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrollingState(float DeltaTime);
    void UpdateInvestigatingState(float DeltaTime);
    void UpdateHuntingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);
    void UpdateFeedingState(float DeltaTime);
    void UpdateTerritorialState(float DeltaTime);
    void UpdatePackHuntingState(float DeltaTime);
    void UpdateWoundedState(float DeltaTime);

    // Utility functions
    AActor* FindNearestThreat() const;
    AActor* FindNearestPrey() const;
    FVector GeneratePatrolPoint() const;
    bool ShouldFlee() const;
    bool ShouldAttack() const;
    bool ShouldInvestigate() const;
    void MoveToLocation(const FVector& TargetLocation);

private:
    // Component references
    UPROPERTY()
    UCombat_DamageSystem* DamageComponent;

    UPROPERTY()
    UCombat_CombatManager* CombatManager;

    UPROPERTY()
    UPawnSensingComponent* PawnSensing;

    // Internal state
    bool bIsInitialized = false;
    float LastStateChangeTime = 0.0f;
    TArray<AActor*> KnownThreats;
    TArray<AActor*> KnownPrey;
};

#include "Combat_DinosaurAI.generated.h"