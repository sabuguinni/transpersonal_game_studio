#include "AI/NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

// ============================================================
// BeginPlay
// ============================================================
void UNPCBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	// Build a default daily routine if none was assigned
	if (DailyRoutine.Num() == 0)
	{
		AActor* Owner = GetOwner();
		FVector HomeLocation = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

		// Dawn: gather near spawn (06:00 - 10:00)
		FNPC_DailyRoutineSlot GatherSlot;
		GatherSlot.StartHour = 6.f;
		GatherSlot.DurationHours = 4.f;
		GatherSlot.ActivityLocation = HomeLocation + FVector(300.f, 0.f, 0.f);
		GatherSlot.Activity = ENPC_BehaviorState::Gather;

		// Midday: patrol perimeter (10:00 - 14:00)
		FNPC_DailyRoutineSlot PatrolSlot;
		PatrolSlot.StartHour = 10.f;
		PatrolSlot.DurationHours = 4.f;
		PatrolSlot.ActivityLocation = HomeLocation + FVector(0.f, 400.f, 0.f);
		PatrolSlot.Activity = ENPC_BehaviorState::Patrol;

		// Afternoon: idle/rest (14:00 - 18:00)
		FNPC_DailyRoutineSlot RestSlot;
		RestSlot.StartHour = 14.f;
		RestSlot.DurationHours = 4.f;
		RestSlot.ActivityLocation = HomeLocation;
		RestSlot.Activity = ENPC_BehaviorState::Idle;

		// Dusk: seek shelter (18:00 - 22:00)
		FNPC_DailyRoutineSlot SeekSlot;
		SeekSlot.StartHour = 18.f;
		SeekSlot.DurationHours = 4.f;
		SeekSlot.ActivityLocation = HomeLocation;
		SeekSlot.Activity = ENPC_BehaviorState::Seek;

		// Night: idle/sleep (22:00 - 06:00)
		FNPC_DailyRoutineSlot SleepSlot;
		SleepSlot.StartHour = 22.f;
		SleepSlot.DurationHours = 8.f;
		SleepSlot.ActivityLocation = HomeLocation;
		SleepSlot.Activity = ENPC_BehaviorState::Idle;

		DailyRoutine.Add(GatherSlot);
		DailyRoutine.Add(PatrolSlot);
		DailyRoutine.Add(RestSlot);
		DailyRoutine.Add(SeekSlot);
		DailyRoutine.Add(SleepSlot);
	}
}

// ============================================================
// TickComponent
// ============================================================
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickMemoryDecay(DeltaTime);
	TickFearDecay(DeltaTime);
}

// ============================================================
// State Machine
// ============================================================
void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	ENPC_BehaviorState OldState = CurrentState;
	CurrentState = NewState;
	OnStateChanged.Broadcast(OldState, NewState);
}

// ============================================================
// Threat & Perception
// ============================================================
void UNPCBehaviorComponent::ReportThreat(FVector ThreatLocation, float ThreatScore,
	bool bIsDino, bool bIsPlayerThreat)
{
	// Apply dinosaur fear multiplier
	float AdjustedScore = ThreatScore;
	if (bIsDino)
	{
		AdjustedScore *= DinoFearMultiplier;
	}

	// Add or update memory entry
	FNPC_MemoryEntry NewEntry;
	NewEntry.Location = ThreatLocation;
	NewEntry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	NewEntry.ThreatScore = AdjustedScore;
	NewEntry.bIsDinosaur = bIsDino;
	NewEntry.bIsPlayer = bIsPlayerThreat;

	// Check if we already have a nearby entry (within 500 units) — update it
	bool bUpdated = false;
	for (FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		if (FVector::Dist(Entry.Location, ThreatLocation) < 500.f && Entry.bIsDinosaur == bIsDino)
		{
			Entry = NewEntry;
			bUpdated = true;
			break;
		}
	}

	if (!bUpdated)
	{
		MemoryEntries.Add(NewEntry);

		// Trim oldest entries if over capacity
		while (MemoryEntries.Num() > MaxMemoryEntries)
		{
			MemoryEntries.RemoveAt(0);
		}
	}

	// Update fear level
	FearLevel = FMath::Clamp(FearLevel + AdjustedScore * 0.1f, 0.f, 1.f);

	// Recalculate threat level and broadcast
	ENPC_ThreatLevel OldThreat = CurrentThreatLevel;
	RecalculateThreatLevel();

	if (CurrentThreatLevel != OldThreat)
	{
		OnThreatDetected.Broadcast(CurrentThreatLevel);
	}

	// Auto-flee on High/Critical threat
	if (CurrentThreatLevel >= ENPC_ThreatLevel::High && CurrentState != ENPC_BehaviorState::Flee)
	{
		TriggerFlee(ThreatLocation);
	}
}

void UNPCBehaviorComponent::ClearMemory()
{
	MemoryEntries.Empty();
	FearLevel = 0.f;
	CurrentThreatLevel = ENPC_ThreatLevel::None;
}

// ============================================================
// Daily Routine
// ============================================================
void UNPCBehaviorComponent::SetDailyRoutine(const TArray<FNPC_DailyRoutineSlot>& NewRoutine)
{
	DailyRoutine = NewRoutine;
	ActiveRoutineIndex = 0;
}

void UNPCBehaviorComponent::UpdateRoutineForHour(float CurrentHour)
{
	// Only update routine if not fleeing or in danger
	if (CurrentState == ENPC_BehaviorState::Flee ||
		CurrentState == ENPC_BehaviorState::Dead ||
		CurrentThreatLevel >= ENPC_ThreatLevel::Medium)
	{
		return;
	}

	for (int32 i = 0; i < DailyRoutine.Num(); ++i)
	{
		const FNPC_DailyRoutineSlot& Slot = DailyRoutine[i];
		float EndHour = Slot.StartHour + Slot.DurationHours;

		// Handle midnight wrap-around
		bool bInSlot = false;
		if (EndHour > 24.f)
		{
			bInSlot = (CurrentHour >= Slot.StartHour) || (CurrentHour < (EndHour - 24.f));
		}
		else
		{
			bInSlot = (CurrentHour >= Slot.StartHour) && (CurrentHour < EndHour);
		}

		if (bInSlot && i != ActiveRoutineIndex)
		{
			ActiveRoutineIndex = i;
			SetBehaviorState(Slot.Activity);
			break;
		}
	}
}

FNPC_DailyRoutineSlot UNPCBehaviorComponent::GetCurrentRoutineSlot() const
{
	if (DailyRoutine.IsValidIndex(ActiveRoutineIndex))
	{
		return DailyRoutine[ActiveRoutineIndex];
	}
	return FNPC_DailyRoutineSlot();
}

// ============================================================
// Flee Logic
// ============================================================
FVector UNPCBehaviorComponent::ComputeFleeDirection(FVector ThreatLocation) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ForwardVector;
	}

	FVector OwnerLoc = Owner->GetActorLocation();
	FVector AwayDir = (OwnerLoc - ThreatLocation).GetSafeNormal();

	// Add slight lateral offset to avoid running directly away (more realistic)
	FVector LateralOffset = FVector::CrossProduct(AwayDir, FVector::UpVector) * 0.3f;
	return (AwayDir + LateralOffset).GetSafeNormal();
}

void UNPCBehaviorComponent::TriggerFlee(FVector ThreatLocation)
{
	FVector FleeDir = ComputeFleeDirection(ThreatLocation);
	SetBehaviorState(ENPC_BehaviorState::Flee);
	OnFleeStarted.Broadcast(FleeDir);
}

// ============================================================
// Private: Memory Decay
// ============================================================
void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
	if (!GetWorld())
	{
		return;
	}

	float Now = GetWorld()->GetTimeSeconds();

	// Remove expired entries
	MemoryEntries.RemoveAll([this, Now](const FNPC_MemoryEntry& Entry)
	{
		return (Now - Entry.Timestamp) > MemoryDecayTime;
	});

	RecalculateThreatLevel();
}

// ============================================================
// Private: Fear Decay
// ============================================================
void UNPCBehaviorComponent::TickFearDecay(float DeltaTime)
{
	if (CurrentThreatLevel == ENPC_ThreatLevel::None)
	{
		FearLevel = FMath::Max(0.f, FearLevel - FearDecayRate * DeltaTime);
	}
}

// ============================================================
// Private: Recalculate Threat Level from memory
// ============================================================
void UNPCBehaviorComponent::RecalculateThreatLevel()
{
	if (MemoryEntries.Num() == 0)
	{
		CurrentThreatLevel = ENPC_ThreatLevel::None;
		return;
	}

	float MaxScore = 0.f;
	for (const FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		MaxScore = FMath::Max(MaxScore, Entry.ThreatScore);
	}

	CurrentThreatLevel = ScoreToThreatLevel(MaxScore);
}

ENPC_ThreatLevel UNPCBehaviorComponent::ScoreToThreatLevel(float Score) const
{
	if (Score <= 0.f)   return ENPC_ThreatLevel::None;
	if (Score < 2.f)    return ENPC_ThreatLevel::Low;
	if (Score < 5.f)    return ENPC_ThreatLevel::Medium;
	if (Score < 10.f)   return ENPC_ThreatLevel::High;
	return ENPC_ThreatLevel::Critical;
}
