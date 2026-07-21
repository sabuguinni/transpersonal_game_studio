#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatEncounter.generated.h"

// ─── Encounter phase ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_EncounterPhase : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Patrol      UMETA(DisplayName = "Patrol"),
	Alert       UMETA(DisplayName = "Alert"),
	Stalk       UMETA(DisplayName = "Stalk"),
	Charge      UMETA(DisplayName = "Charge"),
	Attack      UMETA(DisplayName = "Attack"),
	Retreat     UMETA(DisplayName = "Retreat"),
	Flee        UMETA(DisplayName = "Flee")
};

// ─── Encounter outcome ───────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_EncounterOutcome : uint8
{
	Ongoing     UMETA(DisplayName = "Ongoing"),
	PlayerFled  UMETA(DisplayName = "PlayerFled"),
	DinoFled    UMETA(DisplayName = "DinoFled"),
	PlayerDied  UMETA(DisplayName = "PlayerDied"),
	DinoKilled  UMETA(DisplayName = "DinoKilled")
};

// ─── Per-encounter runtime data ──────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_EncounterState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	ECombat_EncounterPhase Phase = ECombat_EncounterPhase::Idle;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	ECombat_EncounterOutcome Outcome = ECombat_EncounterOutcome::Ongoing;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	float TimeInPhase = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	float PlayerDistanceCached = 9999.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	int32 AttackCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	bool bPlayerSpotted = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat|Encounter")
	bool bPlayerInCover = false;
};

// ─── Encounter trigger volume ────────────────────────────────────────────────
UCLASS(ClassGroup = "Combat", meta = (DisplayName = "Dinosaur Combat Encounter"))
class TRANSPERSONALGAME_API ADinosaurCombatEncounter : public AActor
{
	GENERATED_BODY()

public:
	ADinosaurCombatEncounter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ── Configuration ────────────────────────────────────────────────────────
	/** Radius at which the dinosaur first detects the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
	float DetectionRadius = 2000.f;

	/** Radius at which the dinosaur begins its charge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
	float ChargeRadius = 800.f;

	/** Radius at which a melee attack is triggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
	float MeleeRadius = 200.f;

	/** Base damage per melee hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float MeleeDamage = 45.f;

	/** Cooldown in seconds between melee attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	float MeleeCooldown = 2.5f;

	/** Distance at which the dinosaur gives up the chase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
	float FleeDistance = 4000.f;

	/** Chance (0-1) that dino retreats after taking significant damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
	float RetreatChanceOnDamage = 0.35f;

	// ── Runtime state ────────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	FCombat_EncounterState EncounterState;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	float DinoCurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
	float DinoMaxHealth = 100.f;

	// ── Public API ───────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartEncounter();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndEncounter(ECombat_EncounterOutcome Outcome);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamageToDino(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Combat")
	ECombat_EncounterPhase GetCurrentPhase() const { return EncounterState.Phase; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsEncounterActive() const { return EncounterState.Outcome == ECombat_EncounterOutcome::Ongoing; }

	// ── Delegates ────────────────────────────────────────────────────────────
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnPhaseChanged, ECombat_EncounterPhase, NewPhase);
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FCombat_OnPhaseChanged OnPhaseChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnEncounterEnded, ECombat_EncounterOutcome, Outcome);
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FCombat_OnEncounterEnded OnEncounterEnded;

private:
	void UpdatePhase(float DeltaTime);
	void TransitionToPhase(ECombat_EncounterPhase NewPhase);
	float GetDistanceToPlayer() const;
	bool IsPlayerInCover() const;

	float MeleeAttackTimer = 0.f;
	bool bEncounterStarted = false;

	UPROPERTY()
	class APawn* CachedPlayer = nullptr;
};
