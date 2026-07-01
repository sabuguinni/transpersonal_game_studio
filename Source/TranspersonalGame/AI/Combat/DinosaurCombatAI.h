// DinosaurCombatAI.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260701_003
// Tactical combat AI for dinosaur enemies — T-Rex and Raptor pack behavior
// Follows UE5.5 C++ rules: unique type names with Combat_ prefix, .generated.h last

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "DinosaurCombatAI.generated.h"

// ─────────────────────────────────────────────────────────────
// ENUMS — global scope, Combat_ prefix, unique names
// ─────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    Ignore              UMETA(DisplayName = "Ignore"),
    Investigate         UMETA(DisplayName = "Investigate"),
    Stalk               UMETA(DisplayName = "Stalk"),
    Charge              UMETA(DisplayName = "Charge"),
    Retreat             UMETA(DisplayName = "Retreat"),
    CallPack            UMETA(DisplayName = "Call Pack")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite                UMETA(DisplayName = "Bite"),
    Claw                UMETA(DisplayName = "Claw Swipe"),
    TailWhip            UMETA(DisplayName = "Tail Whip"),
    Charge              UMETA(DisplayName = "Charge Slam"),
    Pounce              UMETA(DisplayName = "Pounce"),
    Stomp               UMETA(DisplayName = "Stomp")
};

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Patrolling          UMETA(DisplayName = "Patrolling"),
    Alerted             UMETA(DisplayName = "Alerted"),
    Stalking            UMETA(DisplayName = "Stalking"),
    Charging            UMETA(DisplayName = "Charging"),
    Attacking           UMETA(DisplayName = "Attacking"),
    Retreating          UMETA(DisplayName = "Retreating"),
    Feeding             UMETA(DisplayName = "Feeding"),
    Resting             UMETA(DisplayName = "Resting")
};

// ─────────────────────────────────────────────────────────────
// STRUCTS — global scope
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCombat_SensoryData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    float VisionRange = 2500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    float HearingRange = 1800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    float SmellRange = 1200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    float VisionAngleDegrees = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    bool bCanDetectCamouflage = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Senses")
    float ThreatMemoryDuration = 30.0f;
};

USTRUCT(BlueprintType)
struct FCombat_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    float BaseDamage = 50.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    float AttackRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    float CooldownSeconds = 2.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    float WindupTimeSeconds = 0.8f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    float KnockbackForce = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Attack")
    bool bCanInterrupt = true;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float TurnRate = 90.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float Aggression = 0.7f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float FearThreshold = 0.2f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI|Stats")
    float TerritoryRadius = 3000.0f;
};

// ─────────────────────────────────────────────────────────────
// MAIN COMPONENT CLASS
// ─────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species & State ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_ThreatResponse CurrentThreatResponse = ECombat_ThreatResponse::Ignore;

    // ── Sensory Configuration ────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Senses")
    FCombat_SensoryData SensoryProfile;

    // ── Attack Configuration ─────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    TArray<FCombat_AttackProfile> AttackProfiles;

    // ── Stats ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    FCombat_DinoStats DinoStats;

    // ── Target Tracking ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Targeting")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Targeting")
    float LastKnownTargetTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    int32 PackID = -1;

    // ── Core AI Functions ────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* PotentialTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatResponse EvaluateThreatResponse(float InThreatLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanHearTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(int32 AttackProfileIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInAttackRange(int32 AttackProfileIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeSpeciesDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    // ── Events (Blueprint implementable) ─────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI|Events")
    void OnTargetAcquired(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI|Events")
    void OnAttackLanded(AActor* Target, float DamageDealt);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI|Events")
    void OnStateChanged(ECombat_DinoState OldState, ECombat_DinoState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI|Events")
    void OnDeath();

private:
    float AttackCooldownRemaining = 0.0f;
    float StateTimer = 0.0f;
    FVector HomeLocation = FVector::ZeroVector;

    void UpdateCombatLogic(float DeltaTime);
    void ScanForThreats();
    void UpdateAttackCooldown(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandleAlertedState(float DeltaTime);
    void HandleChargingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleRetreatingState(float DeltaTime);
};
