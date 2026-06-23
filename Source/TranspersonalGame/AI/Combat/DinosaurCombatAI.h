// DinosaurCombatAI.h
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Dinosaur tactical combat AI component — controls aggression, flanking, pack coordination

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "DinosaurCombatAI.generated.h"

// Combat state enum — must be global scope (UE5 rule)
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Feeding         UMETA(DisplayName = "Feeding"),
    PackCoordinating UMETA(DisplayName = "PackCoordinating")
};

// Attack type enum
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Tail        UMETA(DisplayName = "Tail Sweep"),
    Charge      UMETA(DisplayName = "Charge"),
    Pounce      UMETA(DisplayName = "Pounce"),
    Stomp       UMETA(DisplayName = "Stomp")
};

// Dinosaur species type — affects AI behaviour
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Compsognathus       UMETA(DisplayName = "Compsognathus")
};

// Pack role for coordinated attacks
UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha — leads attack"),
    Flanker     UMETA(DisplayName = "Flanker — circles target"),
    Distractor  UMETA(DisplayName = "Distractor — draws attention"),
    Ambusher    UMETA(DisplayName = "Ambusher — waits for opening"),
    Lone        UMETA(DisplayName = "Lone — no pack")
};

// Threat assessment result
USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;           // 0-1 normalised

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bTargetIsArmed = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bTargetIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    int32 AlliedDinosNearby = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float RecommendedEngageDistance = 300.0f;
};

// Attack data
USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 25.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Cooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float WindupTime = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bCanKnockback = false;
};

// Main combat AI component
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === SPECIES CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_PackRole PackRole = ECombat_PackRole::Lone;

    // === DETECTION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float SmellRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Detection")
    float SightAngleDegrees = 120.0f;

    // === COMBAT STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    FCombat_ThreatAssessment LastThreatAssessment;

    // === ATTACK CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attacks")
    TArray<FCombat_AttackData> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attacks")
    float AggressionLevel = 0.5f;      // 0=passive, 1=always attacks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attacks")
    float FleeHealthThreshold = 0.2f;  // Flee when HP below this fraction

    // === PACK COORDINATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bUsesPackTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float FlankingRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float PackCoordinationRange = 1000.0f;

    // === STATS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChargeSpeed = 700.0f;

    // === PUBLIC FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_ThreatAssessment AssessThreat(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanHearTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AttackType SelectBestAttack(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target, float AngleOffset) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyPackMembersOfTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitialiseForSpecies(ECombat_DinoSpecies InSpecies);

    // Delegates for Blueprint event binding
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnTargetDetected, AActor*, DetectedTarget);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnAttackExecuted, ECombat_AttackType, AttackType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCombat_OnDeath);

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnTargetDetected OnTargetDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnAttackExecuted OnAttackExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnDeath OnDeath;

private:
    float AttackCooldownRemaining = 0.0f;
    float StateTimer = 0.0f;
    bool bIsDead = false;

    void UpdateCombatState(float DeltaTime);
    void TransitionToState(ECombat_DinoState NewState);
    void ExecuteAttack(AActor* Target, const FCombat_AttackData& Attack);
    void ApplySpeciesDefaults();
    bool IsTargetInRange(AActor* Target, float Range) const;
};
