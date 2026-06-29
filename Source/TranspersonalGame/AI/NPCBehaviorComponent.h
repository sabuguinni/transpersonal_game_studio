#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENPC_BehaviorState — top-level NPC behavioral state machine
// Must be at global scope (UHT rule)
// ============================================================
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
	Idle            UMETA(DisplayName = "Idle"),
	Patrol          UMETA(DisplayName = "Patrol"),
	Gather          UMETA(DisplayName = "Gather"),
	Flee            UMETA(DisplayName = "Flee"),
	Seek            UMETA(DisplayName = "Seek"),
	Investigate     UMETA(DisplayName = "Investigate"),
	Interact        UMETA(DisplayName = "Interact"),
	Dead            UMETA(DisplayName = "Dead")
};

// ============================================================
// ENPC_ThreatLevel — perceived danger from environment
// ============================================================
UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
	None        UMETA(DisplayName = "None"),
	Low         UMETA(DisplayName = "Low"),
	Medium      UMETA(DisplayName = "Medium"),
	High        UMETA(DisplayName = "High"),
	Critical    UMETA(DisplayName = "Critical")
};

// ============================================================
// FNPC_MemoryEntry — a single remembered event/entity
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	float Timestamp = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	float ThreatScore = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	bool bIsDinosaur = false;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
	bool bIsPlayer = false;

	FNPC_MemoryEntry() {}
};

// ============================================================
// FNPC_DailyRoutineSlot — one time-block in the NPC's day
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
	GENERATED_BODY()

	/** Hour of day (0-24) when this activity starts */
	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	float StartHour = 0.f;

	/** Duration in in-game hours */
	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	float DurationHours = 2.f;

	/** World location where this activity takes place */
	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	FVector ActivityLocation = FVector::ZeroVector;

	/** Behavior state during this slot */
	UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
	ENPC_BehaviorState Activity = ENPC_BehaviorState::Idle;

	FNPC_DailyRoutineSlot() {}
};

// ============================================================
// Delegates
// ============================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_StateChangedDelegate,
	ENPC_BehaviorState, OldState,
	ENPC_BehaviorState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_ThreatDetectedDelegate,
	ENPC_ThreatLevel, ThreatLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_FleeStartedDelegate,
	FVector, FleeDirection);

// ============================================================
// UNPCBehaviorComponent — drives NPC daily routine + memory
// ============================================================
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
	DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCBehaviorComponent();

	// ---- Lifecycle ----
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ---- State Machine ----
	UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
	void SetBehaviorState(ENPC_BehaviorState NewState);

	UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
	ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

	// ---- Threat & Perception ----
	UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
	void ReportThreat(FVector ThreatLocation, float ThreatScore, bool bIsDino, bool bIsPlayerThreat);

	UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
	void ClearMemory();

	UFUNCTION(BlueprintPure, Category = "NPC|Perception")
	ENPC_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

	UFUNCTION(BlueprintPure, Category = "NPC|Perception")
	float GetFearLevel() const { return FearLevel; }

	// ---- Daily Routine ----
	UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
	void SetDailyRoutine(const TArray<FNPC_DailyRoutineSlot>& NewRoutine);

	UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
	void UpdateRoutineForHour(float CurrentHour);

	UFUNCTION(BlueprintPure, Category = "NPC|Routine")
	FNPC_DailyRoutineSlot GetCurrentRoutineSlot() const;

	// ---- Flee Logic ----
	UFUNCTION(BlueprintCallable, Category = "NPC|Flee")
	FVector ComputeFleeDirection(FVector ThreatLocation) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Flee")
	void TriggerFlee(FVector ThreatLocation);

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FNPC_StateChangedDelegate OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FNPC_ThreatDetectedDelegate OnThreatDetected;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FNPC_FleeStartedDelegate OnFleeStarted;

	// ---- Config ----
	/** Maximum number of memory entries retained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	int32 MaxMemoryEntries = 8;

	/** Time (seconds) before a memory entry expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float MemoryDecayTime = 60.f;

	/** Fear decay rate per second when no threats present */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float FearDecayRate = 0.05f;

	/** Fear gain multiplier when a dinosaur is detected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float DinoFearMultiplier = 2.5f;

	/** Minimum flee distance from threat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
	float FleeDistance = 2500.f;

private:
	UPROPERTY()
	ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

	UPROPERTY()
	ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

	UPROPERTY()
	float FearLevel = 0.f;

	UPROPERTY()
	TArray<FNPC_MemoryEntry> MemoryEntries;

	UPROPERTY()
	TArray<FNPC_DailyRoutineSlot> DailyRoutine;

	UPROPERTY()
	int32 ActiveRoutineIndex = 0;

	void TickMemoryDecay(float DeltaTime);
	void TickFearDecay(float DeltaTime);
	void RecalculateThreatLevel();
	ENPC_ThreatLevel ScoreToThreatLevel(float Score) const;
};
