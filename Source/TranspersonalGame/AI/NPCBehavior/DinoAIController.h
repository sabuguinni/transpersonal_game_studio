#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "DinoAIController.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Blackboard key names (match the BT asset exactly)
// ─────────────────────────────────────────────────────────────────────────────
#define BB_TARGET_ACTOR    TEXT("TargetActor")
#define BB_TARGET_LOCATION TEXT("TargetLocation")
#define BB_PATROL_ORIGIN   TEXT("PatrolOrigin")
#define BB_ALERT_LEVEL     TEXT("AlertLevel")
#define BB_LAST_KNOWN_POS  TEXT("LastKnownPlayerPos")
#define BB_IS_INVESTIGATING TEXT("bIsInvestigating")
#define BB_CAN_ATTACK      TEXT("bCanAttack")

// ─────────────────────────────────────────────────────────────────────────────
// ENPC_DinoSpecies — species-specific behaviour profiles
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FNPC_DinoPerceptionProfile — per-species perception radii
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNPC_DinoPerceptionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 4000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngle = 70.f;   // half-angle in degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float AlertDecayRate = 0.5f;          // alert units lost per second when calm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float ChaseSpeed = 600.f;             // cm/s when chasing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PatrolSpeed = 200.f;            // cm/s when patrolling

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float AttackRange = 300.f;            // cm — melee trigger distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PatrolRadius = 5000.f;          // cm — wander area around spawn
};

// ─────────────────────────────────────────────────────────────────────────────
// FNPC_DinoMemoryEntry — one remembered stimulus
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNPC_DinoMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float Timestamp = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float ThreatScore = 0.f;   // 0=curiosity, 1=full threat

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bWasVisual = true;    // true=seen, false=heard
};

// ─────────────────────────────────────────────────────────────────────────────
// ADinoAIController
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "Dino AI Controller"))
class TRANSPERSONALGAME_API ADinoAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinoAIController();

    // ── Species & Profile ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Species")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Species")
    FNPC_DinoPerceptionProfile PerceptionProfile;

    // ── Behavior Tree ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    // ── Alert State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|AI",
              meta = (AllowPrivateAccess = "true"))
    float AlertLevel = 0.f;   // 0=calm … 1=full alert

    UPROPERTY(BlueprintReadOnly, Category = "Dino|AI")
    bool bIsChasing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dino|AI")
    bool bIsAttacking = false;

    // ── Memory ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dino|Memory")
    TArray<FNPC_DinoMemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Memory")
    float MemoryDecaySeconds = 30.f;

    // ── Public API ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void SetSpeciesProfile(ENPC_DinoSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void OnPlayerDetected(AActor* Player, const FVector& DetectionLocation, bool bVisual);

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void AddMemoryEntry(const FVector& Location, float ThreatScore, bool bVisual);

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void PruneOldMemories();

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    float GetAlertLevel() const { return AlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    bool IsHostile() const { return AlertLevel >= 0.75f; }

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    FNPC_DinoPerceptionProfile GetDefaultProfileForSpecies(ENPC_DinoSpecies InSpecies) const;

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Patrol helpers
    void ChooseNextPatrolPoint();
    bool IsAtDestination(float Tolerance = 150.f) const;

private:
    // Perception components
    UPROPERTY()
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // Patrol state
    FVector PatrolOrigin = FVector::ZeroVector;
    FVector CurrentPatrolTarget = FVector::ZeroVector;
    float PatrolWaitTimer = 0.f;

    // Alert decay timer
    float TimeSinceLastStimulus = 0.f;

    // Cached world time for memory timestamps
    float GetWorldTime() const;

    // Apply profile to perception component
    void ApplyPerceptionProfile(const FNPC_DinoPerceptionProfile& Profile);
};
