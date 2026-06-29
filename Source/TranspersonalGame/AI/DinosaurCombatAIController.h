#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Combat AI State — tactical phases for dinosaur combat
// ============================================================
UENUM(BlueprintType)
enum class ECombat_AIPhase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Alert           UMETA(DisplayName = "Alert"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Charge          UMETA(DisplayName = "Charge"),
    Attack          UMETA(DisplayName = "Attack"),
    Recover         UMETA(DisplayName = "Recover"),
    Flee            UMETA(DisplayName = "Flee"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

// ============================================================
// Dinosaur species type — drives AI parameter sets
// ============================================================
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Generic         UMETA(DisplayName = "Generic")
};

// ============================================================
// Combat threat entry — tracks perceived threats
// ============================================================
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
    float LastSeenTimestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsCurrentlyVisible = false;
};

// ============================================================
// Pack coordination data — shared between pack members
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_PackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* PackLeader = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* SharedTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector FlankPositionLeft = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector FlankPositionRight = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bFlankingActive = false;
};

// ============================================================
// ADinosaurCombatAIController
// Tactical combat AI for dinosaur pawns.
// Handles: perception, threat tracking, phase transitions,
//          pack coordination, attack timing, flee logic.
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ---- Core overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- Species configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    // ---- Combat phase ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIPhase CurrentPhase = ECombat_AIPhase::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_AIPhase PreviousPhase = ECombat_AIPhase::Idle;

    // ---- Threat tracking ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Threats")
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Threats")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    float ThreatDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Threats")
    float MaxThreatMemoryAge = 30.0f;

    // ---- Attack parameters ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float ChargeRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float StalkRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    float StalkSpeed = 350.0f;

    // ---- Health / flee threshold ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Survival")
    float MaxHealth = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Survival")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Survival")
    float FleeHealthThreshold = 0.25f;

    // ---- Pack coordination ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Pack")
    FCombat_PackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bPackHuntingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float PackCoordinationRadius = 2000.0f;

    // ---- Patrol waypoints ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Patrol")
    float PatrolAcceptanceRadius = 150.0f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, FVector Location, float Score);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeCombatDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatPhase(ECombat_AIPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeSpeciesParameters();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankPosition(bool bLeftFlank) const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetHighestThreatTarget() const;

protected:
    // ---- Perception callbacks ----
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // ---- Internal phase logic ----
    void UpdateCombatPhase(float DeltaTime);
    void ExecutePatrol(float DeltaTime);
    void ExecuteAlert(float DeltaTime);
    void ExecuteStalk(float DeltaTime);
    void ExecuteCharge(float DeltaTime);
    void ExecuteAttack(float DeltaTime);
    void ExecuteRecover(float DeltaTime);
    void ExecuteFlee(float DeltaTime);
    void ExecutePackCoordinate(float DeltaTime);

    void DecayThreats(float DeltaTime);
    void UpdatePrimaryTarget();
    void FindPackMembers();
    bool CanSeeTarget(AActor* Target) const;
    float GetDistanceTo(AActor* Target) const;
    void AdvancePatrolWaypoint();

    // ---- Perception component ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent_Combat = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig = nullptr;

private:
    float AttackCooldownTimer = 0.0f;
    float RecoverTimer = 0.0f;
    float PhaseTimer = 0.0f;
    bool bAttackOnCooldown = false;
    float LastPackCoordinationTime = 0.0f;
};
