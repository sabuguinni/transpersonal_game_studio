#pragma once

// DinosaurCombatAIController.h
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260629_013
// Tactical AI controller for dinosaur combat behaviour.
// Handles aggro, attack sequencing, pack coordination, and threat broadcasting.

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DinosaurCombatAIController.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — prefixed Combat_ to avoid cross-agent collisions
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Alert           UMETA(DisplayName = "Alert"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Charge          UMETA(DisplayName = "Charge"),
    Attack          UMETA(DisplayName = "Attack"),
    Recover         UMETA(DisplayName = "Recover"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Charge          UMETA(DisplayName = "Charge"),
    TailSwipe       UMETA(DisplayName = "Tail Swipe"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Roar            UMETA(DisplayName = "Roar")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsVisible = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnStateChanged,
    ECombat_DinoState, OldState,
    ECombat_DinoState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnAttackExecuted,
    ECombat_AttackType, AttackType,
    AActor*, Target);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatDetected,
    AActor*, ThreatActor);

// ─────────────────────────────────────────────────────────────────────────────
// ADinosaurCombatAIController
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ── Lifecycle ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToAlert(AActor* DetectedThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToRetreat();

    // ── Threat System ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, FVector LastKnownLoc);

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    float GetThreatLevelFor(AActor* Actor) const;

    // ── Attack System ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteAttack(ECombat_AttackType AttackType, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void SelectBestAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void BroadcastRoarToPackMembers(float RoarRadius);

    // ── Pack Coordination ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RegisterPackMember(ADinosaurCombatAIController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void NotifyPackOfThreat(AActor* ThreatActor, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignFlankingRole(ADinosaurCombatAIController* PackMember, FVector FlankPosition);

    // ── Species Configuration ────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureForSpecies(ECombat_DinoSpecies Species);

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackExecuted OnAttackExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnThreatDetected OnThreatDetected;

    // ── Species & Identity ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    bool bIsApexPredator = false;

    // ── Detection Ranges ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float AggroRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float RetreatHealthThreshold = 0.2f;

    // ── Movement Speeds ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float StalkSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float ChargeSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float RetreatSpeed = 500.0f;

    // ── Attack Data ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attacks")
    TArray<FCombat_AttackData> AvailableAttacks;

    // ── Pack ─────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 MaxPackSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 2000.0f;

protected:
    // ── Internal State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState PreviousState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat",
        meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatMemory> ThreatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Attack",
        meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Attack",
        meta = (AllowPrivateAccess = "true"))
    float CurrentAttackCooldown = 2.0f;

    // ── Perception ───────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    // ── Timers ───────────────────────────────────────────────────────────────
    FTimerHandle ThreatDecayTimer;
    FTimerHandle AttackCooldownTimer;
    FTimerHandle PackCoordinationTimer;

    // ── Tick helpers ─────────────────────────────────────────────────────────
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickStalk(float DeltaTime);
    void TickCharge(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickRecover(float DeltaTime);
    void TickRetreat(float DeltaTime);

    // ── Internal helpers ─────────────────────────────────────────────────────
    void DecayThreatMemory();
    void OnAttackCooldownExpired();
    void CoordinateWithPack();
    float CalculateThreatScore(AActor* Actor) const;
    bool IsTargetInAttackRange(AActor* Target) const;
    void ApplyDamageToTarget(AActor* Target, const FCombat_AttackData& Attack);

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};
