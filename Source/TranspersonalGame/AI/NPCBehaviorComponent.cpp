#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"

// ============================================================
// Constructor
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentAlertLevel      = ENPC_AlertLevel::Unaware;
	CurrentPhase           = ENPC_RoutinePhase::Foraging;
	ActiveThreatResponse   = ENPC_ThreatResponse::Ignore;
	ThreatScore            = 0.0f;

	// Default daily routine: forage dawn to dusk, rest at night
	FNPC_DailyRoutineSlot Dawn;
	Dawn.Phase          = ENPC_RoutinePhase::Waking;
	Dawn.StartHour      = 5.0f;
	Dawn.EndHour        = 7.0f;
	Dawn.Priority       = 1.0f;

	FNPC_DailyRoutineSlot Morning;
	Morning.Phase       = ENPC_RoutinePhase::Foraging;
	Morning.StartHour   = 7.0f;
	Morning.EndHour     = 12.0f;
	Morning.Priority    = 1.0f;

	FNPC_DailyRoutineSlot Midday;
	Midday.Phase        = ENPC_RoutinePhase::Resting;
	Midday.StartHour    = 12.0f;
	Midday.EndHour      = 15.0f;
	Midday.Priority     = 0.8f;

	FNPC_DailyRoutineSlot Afternoon;
	Afternoon.Phase     = ENPC_RoutinePhase::Foraging;
	Afternoon.StartHour = 15.0f;
	Afternoon.EndHour   = 19.0f;
	Afternoon.Priority  = 1.0f;

	FNPC_DailyRoutineSlot Night;
	Night.Phase         = ENPC_RoutinePhase::Sleeping;
	Night.StartHour     = 19.0f;
	Night.EndHour       = 5.0f;
	Night.Priority      = 1.0f;

	DailyRoutine.Add(Dawn);
	DailyRoutine.Add(Morning);
	DailyRoutine.Add(Midday);
	DailyRoutine.Add(Afternoon);
	DailyRoutine.Add(Night);
}

// ============================================================
// BeginPlay
// ============================================================

void UNPCBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ============================================================
// TickComponent
// ============================================================

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DecayThreat(DeltaTime);
	TickMemoryDecay(DeltaTime);
}

// ============================================================
// Alert System
// ============================================================

void UNPCBehaviorComponent::RaiseThreat(AActor* ThreatSource, float ThreatAmount)
{
	if (!ThreatSource) return;

	ThreatScore = FMath::Clamp(ThreatScore + ThreatAmount, 0.0f, 100.0f);
	UpdateAlertLevelFromScore();

	// Record in memory
	RecordThreat(ThreatSource, ThreatSource->GetActorLocation(), ThreatAmount);
}

void UNPCBehaviorComponent::DecayThreat(float DeltaTime)
{
	if (ThreatScore <= 0.0f) return;

	// Threat decays faster when NPC is Unaware or Curious
	float DecayMultiplier = 1.0f;
	if (CurrentAlertLevel == ENPC_AlertLevel::Alarmed || CurrentAlertLevel == ENPC_AlertLevel::Panicked)
	{
		DecayMultiplier = 0.3f; // Alarmed state decays slowly
	}

	ThreatScore = FMath::Max(0.0f, ThreatScore - (ThreatDecayRate * DecayMultiplier * DeltaTime));
	UpdateAlertLevelFromScore();
}

void UNPCBehaviorComponent::UpdateAlertLevelFromScore()
{
	ENPC_AlertLevel NewLevel;

	if (ThreatScore <= 0.0f)
	{
		NewLevel = ENPC_AlertLevel::Unaware;
	}
	else if (ThreatScore < 20.0f)
	{
		NewLevel = ENPC_AlertLevel::Curious;
	}
	else if (ThreatScore < ThreatAlarmThreshold)
	{
		NewLevel = ENPC_AlertLevel::Suspicious;
	}
	else if (ThreatScore < ThreatPanicThreshold)
	{
		NewLevel = ENPC_AlertLevel::Alarmed;
	}
	else
	{
		NewLevel = ENPC_AlertLevel::Panicked;
	}

	CurrentAlertLevel = NewLevel;
}

// ============================================================
// Routine System
// ============================================================

void UNPCBehaviorComponent::UpdateDailyRoutine(float CurrentHour)
{
	// Fleeing/Investigating override routine
	if (CurrentAlertLevel == ENPC_AlertLevel::Panicked)
	{
		CurrentPhase = ENPC_RoutinePhase::Fleeing;
		return;
	}
	if (CurrentAlertLevel == ENPC_AlertLevel::Alarmed || CurrentAlertLevel == ENPC_AlertLevel::Suspicious)
	{
		CurrentPhase = ENPC_RoutinePhase::Investigating;
		return;
	}

	// Find matching routine slot
	for (const FNPC_DailyRoutineSlot& Slot : DailyRoutine)
	{
		bool bInRange = false;
		if (Slot.StartHour < Slot.EndHour)
		{
			bInRange = (CurrentHour >= Slot.StartHour && CurrentHour < Slot.EndHour);
		}
		else
		{
			// Wraps midnight (e.g. 19:00 to 05:00)
			bInRange = (CurrentHour >= Slot.StartHour || CurrentHour < Slot.EndHour);
		}

		if (bInRange)
		{
			CurrentPhase = Slot.Phase;
			return;
		}
	}
}

void UNPCBehaviorComponent::AddRoutineSlot(const FNPC_DailyRoutineSlot& Slot)
{
	DailyRoutine.Add(Slot);
}

// ============================================================
// Memory System
// ============================================================

void UNPCBehaviorComponent::RecordThreat(AActor* ThreatActor, FVector Location, float Score)
{
	if (!ThreatActor) return;

	// Update existing entry
	for (FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		if (Entry.ThreatActor == ThreatActor)
		{
			Entry.LastKnownLocation = Location;
			Entry.TimeSinceLastSeen = 0.0f;
			Entry.ThreatScore       = FMath::Max(Entry.ThreatScore, Score);
			Entry.bIsActive         = true;
			return;
		}
	}

	// Add new entry (evict oldest if at capacity)
	if (MemoryEntries.Num() >= MaxMemoryEntries)
	{
		int32 OldestIdx = 0;
		float OldestTime = 0.0f;
		for (int32 i = 0; i < MemoryEntries.Num(); ++i)
		{
			if (MemoryEntries[i].TimeSinceLastSeen > OldestTime)
			{
				OldestTime = MemoryEntries[i].TimeSinceLastSeen;
				OldestIdx  = i;
			}
		}
		MemoryEntries.RemoveAt(OldestIdx);
	}

	FNPC_MemoryEntry NewEntry;
	NewEntry.ThreatActor        = ThreatActor;
	NewEntry.LastKnownLocation  = Location;
	NewEntry.TimeSinceLastSeen  = 0.0f;
	NewEntry.ThreatScore        = Score;
	NewEntry.bIsActive          = true;
	MemoryEntries.Add(NewEntry);
}

void UNPCBehaviorComponent::ForgetThreat(AActor* ThreatActor)
{
	MemoryEntries.RemoveAll([ThreatActor](const FNPC_MemoryEntry& E)
	{
		return E.ThreatActor == ThreatActor;
	});
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* ThreatActor) const
{
	for (const FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		if (Entry.ThreatActor == ThreatActor && Entry.bIsActive)
		{
			return true;
		}
	}
	return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation(AActor* ThreatActor) const
{
	for (const FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		if (Entry.ThreatActor == ThreatActor)
		{
			return Entry.LastKnownLocation;
		}
	}
	return FVector::ZeroVector;
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
	for (FNPC_MemoryEntry& Entry : MemoryEntries)
	{
		Entry.TimeSinceLastSeen += DeltaTime;
		if (Entry.TimeSinceLastSeen >= MemoryForgetTime)
		{
			Entry.bIsActive = false;
		}
	}

	// Remove fully forgotten entries
	MemoryEntries.RemoveAll([](const FNPC_MemoryEntry& E)
	{
		return !E.bIsActive && E.ThreatActor == nullptr;
	});
}

// ============================================================
// Threat Response
// ============================================================

ENPC_ThreatResponse UNPCBehaviorComponent::EvaluateThreatResponse(AActor* ThreatActor) const
{
	if (!ThreatActor) return ENPC_ThreatResponse::Ignore;

	switch (CurrentAlertLevel)
	{
		case ENPC_AlertLevel::Unaware:
			return ENPC_ThreatResponse::Ignore;
		case ENPC_AlertLevel::Curious:
			return ENPC_ThreatResponse::Observe;
		case ENPC_AlertLevel::Suspicious:
			return ENPC_ThreatResponse::Warn;
		case ENPC_AlertLevel::Alarmed:
			return ENPC_ThreatResponse::Flee;
		case ENPC_AlertLevel::Panicked:
			return ENPC_ThreatResponse::Flee;
		default:
			return ENPC_ThreatResponse::Ignore;
	}
}

void UNPCBehaviorComponent::SetThreatResponse(ENPC_ThreatResponse Response)
{
	ActiveThreatResponse = Response;
}
