#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Carnotaurus  UMETA(DisplayName = "Carnotaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Charge      UMETA(DisplayName = "Charge"),
    Tail        UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Claw        UMETA(DisplayName = "Claw Slash"),
    Headbutt    UMETA(DisplayName = "Headbutt"),
};

UENUM(BlueprintType)
enum class ECombat_AIPhase : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Attack      UMETA(DisplayName = "Attack"),
    Recover     UMETA(DisplayName = "Recover"),
    Flee        UMETA(DisplayName = "Flee"),
    Roar        UMETA(DisplayName = "Roar"),
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AggroRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float FleeHealthThreshold = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float RoarRadius = 2000.0f;

    FCombat_SpeciesTraits() {}
};

USTRUCT(BlueprintType)
struct FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_AIPhase CurrentPhase = ECombat_AIPhase::Patrol;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float StalkDuration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bHasTarget = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsRoaring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    FCombat_TacticalState() {}
};

// ============================================================
// ADinosaurCombatAIController
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── Species configuration ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_SpeciesTraits SpeciesTraits;

    // ── Tactical state (read-only in BP) ──────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_TacticalState TacticalState;

    // ── Public API ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AIPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetingPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ForceFleeFromActor(AActor* Threat);

    // ── Pack coordination ──────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void NotifyPackMemberOfTarget(AActor* Target, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(AActor* Target, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetFlankingOffset(FVector Offset);

protected:
    // ── Perception callbacks ───────────────────────────────
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // ── Phase transitions ──────────────────────────────────
    void TransitionToPhase(ECombat_AIPhase NewPhase);
    void UpdatePatrol(float DeltaTime);
    void UpdateStalk(float DeltaTime);
    void UpdateCharge(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateRecover(float DeltaTime);
    void UpdateFlee(float DeltaTime);
    void UpdateRoar(float DeltaTime);

    // ── Helpers ────────────────────────────────────────────
    bool IsTargetInAttackRange() const;
    bool IsTargetInAggroRange() const;
    FVector GetFleeDestination() const;
    void ExecuteAttack();
    void BroadcastRoarToNearbyDinos();

    // ── Components ─────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

private:
    // ── Internal state ─────────────────────────────────────
    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    UNPCBehaviorComponent* BehaviorComponent;

    FVector FlankingOffset;
    float PhaseTimer;
    float RoarTimer;
    float PatrolWaitTimer;
    int32 PatrolPointIndex;

    static const int32 MAX_PATROL_POINTS = 6;
    FVector PatrolPoints[MAX_PATROL_POINTS];

    void GeneratePatrolPoints();
    void ApplySpeciesDefaults();
};
