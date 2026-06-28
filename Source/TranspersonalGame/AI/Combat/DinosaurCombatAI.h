#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

// Combat state enum for dinosaur AI
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    PackHunting     UMETA(DisplayName = "PackHunting")
};

// Attack type enum
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    TailSwipe       UMETA(DisplayName = "TailSwipe"),
    Charge          UMETA(DisplayName = "Charge"),
    Pounce          UMETA(DisplayName = "Pounce"),
    Stomp           UMETA(DisplayName = "Stomp")
};

// Threat level enum
UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

// Dinosaur species traits struct
USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChargeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float FleeHealthThreshold;

    FCombat_DinoSpeciesTraits()
        : SpeciesName(NAME_None)
        , DetectionRadius(1500.f)
        , AttackRange(200.f)
        , ChargeSpeed(800.f)
        , BaseDamage(35.f)
        , MaxHealth(300.f)
        , bIsPackHunter(false)
        , PackSize(1)
        , FleeHealthThreshold(0.2f)
    {}
};

// Combat event struct
USTRUCT(BlueprintType)
struct FCombat_DinoAttackEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    ECombat_AttackType AttackType;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    float DamageDealt;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    FVector ImpactLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Attacker;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Target;

    FCombat_DinoAttackEvent()
        : AttackType(ECombat_AttackType::None)
        , DamageDealt(0.f)
        , ImpactLocation(FVector::ZeroVector)
        , Attacker(nullptr)
        , Target(nullptr)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnAttackExecuted, const FCombat_DinoAttackEvent&, AttackEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnStateChanged, ECombat_DinoState, OldState, ECombat_DinoState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatDetected, AActor*, ThreatActor);

/**
 * DinosaurCombatAI - Core combat AI component for dinosaur enemies
 * Implements tactical combat behavior: stalking, charging, pack coordination
 * Agent #12 — Combat & Enemy AI
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === SPECIES CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoSpeciesTraits SpeciesTraits;

    // === COMBAT STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float Stamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget;

    // === PACK COORDINATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius;

    // === TACTICAL PARAMETERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float StalkingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float FlankingAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float ReactionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float AggressionMultiplier;

    // === EVENTS ===
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackExecuted OnAttackExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnThreatDetected OnThreatDetected;

    // === PUBLIC FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage_Combat(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(UDinosaurCombatAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackType SelectBestAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpecies(FName Species);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(int32 PackIndex, int32 TotalPackSize) const;

private:
    float AttackCooldownTimer;
    float StateTimer;
    float ReactionTimer;
    bool bAttackOnCooldown;

    void UpdateCombatState(float DeltaTime);
    void UpdateIdleState(float DeltaTime);
    void UpdateStalkingState(float DeltaTime);
    void UpdateChargingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateRetreatingState(float DeltaTime);
    void EvaluateThreatLevel();
    void BroadcastPackAlert(AActor* Threat);
    AActor* FindNearestThreat() const;
    bool HasLineOfSightToTarget() const;
};
