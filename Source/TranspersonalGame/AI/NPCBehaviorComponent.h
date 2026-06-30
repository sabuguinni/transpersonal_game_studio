#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
	Unaware       UMETA(DisplayName = "Unaware"),
	Curious       UMETA(DisplayName = "Curious"),
	Suspicious    UMETA(DisplayName = "Suspicious"),
	Alarmed       UMETA(DisplayName = "Alarmed"),
	Panicked      UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
	Sleeping      UMETA(DisplayName = "Sleeping"),
	Waking        UMETA(DisplayName = "Waking"),
	Foraging      UMETA(DisplayName = "Foraging"),
	Resting       UMETA(DisplayName = "Resting"),
	Socializing   UMETA(DisplayName = "Socializing"),
	Fleeing       UMETA(DisplayName = "Fleeing"),
	Investigating UMETA(DisplayName = "Investigating"),
	Patrolling    UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class ENPC_ThreatResponse : uint8
{
	Ignore        UMETA(DisplayName = "Ignore"),
	Observe       UMETA(DisplayName = "Observe"),
	Warn          UMETA(DisplayName = "Warn"),
	Flee          UMETA(DisplayName = "Flee"),
	Attack        UMETA(DisplayName = "Attack")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	AActor* ThreatActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	float TimeSinceLastSeen = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	float ThreatScore = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	ENPC_RoutinePhase Phase = ENPC_RoutinePhase::Foraging;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	float StartHour = 6.0f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	float EndHour = 12.0f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	float Priority = 1.0f;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCBehaviorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ── Alert System ──────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
	void RaiseThreat(AActor* ThreatSource, float ThreatAmount);

	UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
	void DecayThreat(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "NPC|Alert")
	ENPC_AlertLevel GetCurrentAlertLevel() const { return CurrentAlertLevel; }

	UFUNCTION(BlueprintPure, Category = "NPC|Alert")
	float GetThreatScore() const { return ThreatScore; }

	// ── Routine System ────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
	void UpdateDailyRoutine(float CurrentHour);

	UFUNCTION(BlueprintPure, Category = "NPC|Routine")
	ENPC_RoutinePhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
	void AddRoutineSlot(const FNPC_DailyRoutineSlot& Slot);

	// ── Memory System ─────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
	void RecordThreat(AActor* ThreatActor, FVector Location, float Score);

	UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
	void ForgetThreat(AActor* ThreatActor);

	UFUNCTION(BlueprintPure, Category = "NPC|Memory")
	bool HasMemoryOf(AActor* ThreatActor) const;

	UFUNCTION(BlueprintPure, Category = "NPC|Memory")
	FVector GetLastKnownThreatLocation(AActor* ThreatActor) const;

	// ── Threat Response ───────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
	ENPC_ThreatResponse EvaluateThreatResponse(AActor* ThreatActor) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
	void SetThreatResponse(ENPC_ThreatResponse Response);

	// ── Config ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float ThreatDecayRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float ThreatAlarmThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float ThreatPanicThreshold = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float MemoryForgetTime = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float DetectionRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	int32 MaxMemoryEntries = 8;

private:
	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	ENPC_AlertLevel CurrentAlertLevel;

	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	ENPC_RoutinePhase CurrentPhase;

	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	ENPC_ThreatResponse ActiveThreatResponse;

	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	float ThreatScore;

	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	TArray<FNPC_MemoryEntry> MemoryEntries;

	UPROPERTY(VisibleAnywhere, Category = "NPC|State")
	TArray<FNPC_DailyRoutineSlot> DailyRoutine;

	void UpdateAlertLevelFromScore();
	void TickMemoryDecay(float DeltaTime);
};
