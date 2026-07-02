#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ── Combat AI state machine ──────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

// ── Dinosaur species archetype ───────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

// ── Per-species combat parameters ────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float HearingRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PatrolSpeed = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float RoarRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float InvestigateTimeout = 8.0f;
};

// ── Combat event data ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_AttackEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Attacker = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Target = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    float DamageDealt = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    float Timestamp = 0.0f;
};

// ── Delegate declarations ─────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnAttackDelegate, const FCombat_AttackEvent&, AttackEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnStateChangedDelegate, ECombat_DinoState, OldState, ECombat_DinoState, NewState);

// ─────────────────────────────────────────────────────────────────────────────
// ADinosaurCombatAIController
// AIController subclass implementing full dinosaur combat state machine.
// Integrates UAIPerceptionComponent for sight/hearing detection.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = "TranspersonalGame|CombatAI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ── Overrides ─────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State machine ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ForceAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void TriggerRoar();

    // ── Species configuration ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Species")
    void SetSpecies(ECombat_DinoSpecies Species);

    UFUNCTION(BlueprintPure, Category = "Combat|Species")
    FCombat_DinoSpeciesParams GetSpeciesParams() const { return SpeciesParams; }

    // ── Pack coordination ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RegisterPackMember(ADinosaurCombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AlertPackToTarget(AActor* Target, FVector LastKnownLocation);

    // ── Events ────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackDelegate OnAttackLanded;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnStateChangedDelegate OnStateChanged;

protected:
    // ── Perception callbacks ──────────────────────────────────────────────────
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ── State handlers ────────────────────────────────────────────────────────
    void HandleIdle(float DeltaTime);
    void HandlePatrol(float DeltaTime);
    void HandleInvestigate(float DeltaTime);
    void HandleChase(float DeltaTime);
    void HandleAttack(float DeltaTime);
    void HandleRoar(float DeltaTime);
    void HandleRetreat(float DeltaTime);

    // ── Internal helpers ──────────────────────────────────────────────────────
    bool IsTargetInAttackRange() const;
    float GetDistanceToTarget() const;
    FVector GetRandomPatrolPoint() const;
    void ApplyDamageToTarget();
    FCombat_DinoSpeciesParams BuildSpeciesParams(ECombat_DinoSpecies Species) const;

    // ── Components ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig = nullptr;

    // ── State ─────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::TRex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    FCombat_DinoSpeciesParams SpeciesParams;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State",
        meta = (AllowPrivateAccess = "true"))
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack",
        meta = (AllowPrivateAccess = "true"))
    TArray<ADinosaurCombatAIController*> PackMembers;

    // ── Timers ────────────────────────────────────────────────────────────────
    float StateTimer = 0.0f;
    float AttackCooldownTimer = 0.0f;
    float RoarTimer = 0.0f;
    float InvestigateTimer = 0.0f;
    FVector HomeLocation = FVector::ZeroVector;
    FVector PatrolTarget = FVector::ZeroVector;
    bool bPatrolTargetSet = false;
};
