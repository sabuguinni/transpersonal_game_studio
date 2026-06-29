#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

// === FORWARD DECLARATIONS ===
class UBehaviorTree;
class UAIPerceptionComponent;
class UBlackboardComponent;

// === ENUMS ===

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Wounded         UMETA(DisplayName = "Wounded"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Charge      UMETA(DisplayName = "Charge"),
    Tailswipe   UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce"),
    Gore        UMETA(DisplayName = "Gore")
};

// === STRUCTS ===

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Range = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

// === COMPONENT CLASS ===

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Combat AI")
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === SPECIES CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attacks")
    TArray<FCombat_AttackData> AvailableAttacks;

    // === STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    TArray<FCombat_ThreatEntry> ThreatList;

    // === COMBAT FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack(AActor* Target, const FCombat_AttackData& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreat(AActor* ThreatActor, float ThreatAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* SelectHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackData SelectBestAttack(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertNearbyPackMembers();

    // === EVENTS (Blueprint-implementable) ===
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackStarted(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnTookDamage(float DamageAmount, AActor* Source);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnStateChanged(ECombat_DinoState OldState, ECombat_DinoState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnTargetLost();

private:
    float LastAttackTime = 0.0f;
    float StateEnterTime = 0.0f;
    bool bAttackOnCooldown = false;

    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateInvestigateState(float DeltaTime);
    void UpdateStalkState(float DeltaTime);
    void UpdateChargeState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateFleeState(float DeltaTime);
    void UpdateWoundedState(float DeltaTime);
    void DecayThreatList(float DeltaTime);
    float CalculateDistanceTo(AActor* Target) const;
};
