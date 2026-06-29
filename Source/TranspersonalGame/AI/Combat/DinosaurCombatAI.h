#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon")
};

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
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Tail        UMETA(DisplayName = "Tail Sweep"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Roar        UMETA(DisplayName = "Intimidation Roar")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 1200.0f;

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
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    float TimeLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsFlankingActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    FVector FlankingPosition = FVector::ZeroVector;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species & Stats ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // ── State Machine ────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState PreviousState = ECombat_DinoState::Idle;

    // ── Threat System ────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FCombat_ThreatMemory PrimaryThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    TArray<FCombat_ThreatMemory> ThreatMemories;

    // ── Pack Coordination ────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_PackCoordination PackData;

    // ── Behavior Tree ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|AI")
    UBlackboardComponent* BlackboardComp;

    // ── Core Combat Functions ────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateDamage(ECombat_AttackType AttackType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldCharge() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankingPosition(AActor* Target, int32 FlankIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatLevel EvaluateThreatLevel(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastAlertToPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    // ── Species-specific defaults ────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplySpeciesDefaults();

    // ── Events ───────────────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnStateChanged(ECombat_DinoState OldState, ECombat_DinoState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackLanded(AActor* Target, float DamageDealt, ECombat_AttackType AttackType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnDeath(AActor* Killer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnPackAlertReceived(AActor* Threat);

private:
    float AttackCooldownTimer = 0.0f;
    float ThreatMemoryDecayRate = 0.1f;
    bool bIsDead = false;

    void TickStateMachine(float DeltaTime);
    void TickThreatEvaluation(float DeltaTime);
    void TickPackCoordination(float DeltaTime);
};
