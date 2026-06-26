#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Combat AI Controller — Dinosaur tactical combat intelligence
// Agent #12 — Combat & Enemy AI Agent
// Cycle: PROD_CYCLE_AUTO_20260626_008
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Generic         UMETA(DisplayName = "Generic"),
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsPlayer = false;
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* MemberActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoState MemberState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector AssignedFlankPosition = FVector::ZeroVector;
};

UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // ── Species Configuration ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    float AggroRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    int32 MaxPackSize = 3;

    // ── Combat State ───────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentCombatState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|State")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State",
        meta = (AllowPrivateAccess = "true"))
    float AlertTimer = 0.0f;

    // ── Threat Memory ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Threats",
        meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatEntry> ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    float ThreatDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    float ThreatMemoryMaxAge = 30.0f;

    // ── Pack Coordination ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack",
        meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float FlankingSpread = 600.0f;

    // ── UFUNCTIONS ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, float Score, bool bIsPlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeCombatDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* MemberActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BroadcastAlertToPackMembers(AActor* ThreatActor, float Score);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHighestThreatScore() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetHighestThreatLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankPosition(int32 FlankIndex, FVector TargetLocation) const;

private:
    void UpdateCombatState(float DeltaTime);
    void ScanForThreats();
    void DecayThreatMemory(float DeltaTime);
    void PurgeStaleThreats();
    void ExecuteStateLogic(float DeltaTime);
    void ExecutePackFlankingLogic();
    void ApplySpeciesDefaults();

    AActor* CachedPrimaryTarget = nullptr;
    float ScanInterval = 0.2f;
    float ScanTimer = 0.0f;
    float AttackCooldown = 0.0f;
    static constexpr int32 MaxThreatEntries = 6;
};
