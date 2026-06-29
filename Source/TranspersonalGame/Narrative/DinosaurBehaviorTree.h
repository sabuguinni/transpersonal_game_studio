#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AIController.h"
#include "DinosaurBehaviorTree.generated.h"

// ============================================================
// ENarr_DinoState — AI state machine for dinosaur behavior
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// ============================================================
// ENarr_DinoSpecies — species classification
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaur      UMETA(DisplayName = "Ankylosaur"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

// ============================================================
// ENarr_DinoBehaviorMode — predator vs herbivore
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DinoBehaviorMode : uint8
{
    Predator    UMETA(DisplayName = "Predator"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Scavenger   UMETA(DisplayName = "Scavenger")
};

// ============================================================
// FNarr_DinoSenses — perception parameters
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DinoSenses
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    float SmellRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    bool bCanDetectMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Senses")
    float LoseSightAfterSeconds = 8.0f;
};

// ============================================================
// FNarr_DinoCombatStats — combat parameters
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DinoCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.25f;
};

// ============================================================
// ADinosaurBehaviorTree — main dinosaur AI actor
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBehaviorTree : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorTree();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Mode ─────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ENarr_DinoSpecies Species = ENarr_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ENarr_DinoBehaviorMode BehaviorMode = ENarr_DinoBehaviorMode::Predator;

    // ── AI State ────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    ENarr_DinoState CurrentState = ENarr_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    FNarr_DinoSenses Senses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    FNarr_DinoCombatStats CombatStats;

    // ── Patrol ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<AActor*> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolWaitTime = 3.0f;

    // ── Herd ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Herd")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Herd")
    float HerdAlertRadius = 1200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Herd")
    TArray<ADinosaurBehaviorTree*> NearbyPackMembers;

    // ── Target ──────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // ── Perception ──────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    // ── State Machine Functions ─────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetState(ENarr_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    ENarr_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDamageFromPlayer(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Herd")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Herd")
    void FindNearbyPackMembers();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    float GetDistanceToTarget() const;

protected:
    // Internal state handlers
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleInvestigateState(float DeltaTime);
    void HandleChaseState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void HandleFleeState(float DeltaTime);

    void AdvancePatrolPoint();
    void MoveToLocation(const FVector& Location, float Speed);

    float TimeSinceLastAttack = 0.0f;
    float PatrolWaitTimer = 0.0f;
    float InvestigateTimer = 0.0f;
    bool bWaiting = false;
};
