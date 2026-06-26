#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for NPC logic
}

// ============================================================
// BeginPlay
// ============================================================

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Seed a default daily routine if none configured
    if (DailyRoutine.Num() == 0)
    {
        auto AddSlot = [&](ENPC_DailyRoutineSlot Slot, ENPC_BehaviorState State, float Dur)
        {
            FNPC_DailyRoutineEntry E;
            E.TimeSlot      = Slot;
            E.DesiredState  = State;
            E.TargetLocation = HomeLocation;
            E.Duration      = Dur;
            DailyRoutine.Add(E);
        };

        AddSlot(ENPC_DailyRoutineSlot::EarlyMorning, ENPC_BehaviorState::Rest,     60.0f);
        AddSlot(ENPC_DailyRoutineSlot::Morning,      ENPC_BehaviorState::Forage,  120.0f);
        AddSlot(ENPC_DailyRoutineSlot::Midday,       ENPC_BehaviorState::Patrol,   90.0f);
        AddSlot(ENPC_DailyRoutineSlot::Afternoon,    ENPC_BehaviorState::Forage,  120.0f);
        AddSlot(ENPC_DailyRoutineSlot::Evening,      ENPC_BehaviorState::Socialise, 60.0f);
        AddSlot(ENPC_DailyRoutineSlot::Night,        ENPC_BehaviorState::Rest,    180.0f);
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
    EvaluateThreats();
    TickBehaviorState(DeltaTime);
}

// ============================================================
// State Machine
// ============================================================

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState) return;
    CurrentState = NewState;
    StateTimer   = 0.0f;
}

void UNPCBehaviorComponent::EvaluateThreats()
{
    // Aggregate threat from memory bank
    float MaxThreat = 0.0f;
    for (const FNPC_MemoryEntry& Mem : MemoryBank)
    {
        float Decayed = Mem.ThreatScore * FMath::Max(0.0f, 1.0f - Mem.TimeSinceObserved / MemoryMaxAge);
        MaxThreat = FMath::Max(MaxThreat, Decayed);
    }

    CurrentThreatScore = MaxThreat;

    if      (MaxThreat >= 0.9f) CurrentThreatLevel = ENPC_ThreatLevel::Critical;
    else if (MaxThreat >= 0.7f) CurrentThreatLevel = ENPC_ThreatLevel::High;
    else if (MaxThreat >= 0.4f) CurrentThreatLevel = ENPC_ThreatLevel::Medium;
    else if (MaxThreat >= 0.1f) CurrentThreatLevel = ENPC_ThreatLevel::Low;
    else                         CurrentThreatLevel = ENPC_ThreatLevel::None;

    // Override state when threat is high enough
    if (MaxThreat >= FleeThreshold && CurrentState != ENPC_BehaviorState::Dead)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (MaxThreat >= 0.2f && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

// ============================================================
// Memory
// ============================================================

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatScore, bool bIsPredator)
{
    if (!ThreatActor) return;

    FVector ThreatLoc = ThreatActor->GetActorLocation();

    // Update existing entry if present
    for (FNPC_MemoryEntry& Mem : MemoryBank)
    {
        if (FVector::Dist(Mem.LastKnownLocation, ThreatLoc) < 200.0f)
        {
            Mem.LastKnownLocation  = ThreatLoc;
            Mem.TimeSinceObserved  = 0.0f;
            Mem.ThreatScore        = FMath::Max(Mem.ThreatScore, ThreatScore);
            Mem.bIsPredator        = bIsPredator;
            return;
        }
    }

    // Add new memory entry (cap at 8 entries)
    if (MemoryBank.Num() < 8)
    {
        FNPC_MemoryEntry NewMem;
        NewMem.LastKnownLocation = ThreatLoc;
        NewMem.TimeSinceObserved = 0.0f;
        NewMem.ThreatScore       = ThreatScore;
        NewMem.bIsPlayer         = false;
        NewMem.bIsPredator       = bIsPredator;
        MemoryBank.Add(NewMem);
    }
}

void UNPCBehaviorComponent::ForgetStaleMemories(float MaxAge)
{
    MemoryBank.RemoveAll([MaxAge](const FNPC_MemoryEntry& M)
    {
        return M.TimeSinceObserved >= MaxAge;
    });
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetHighestThreatMemory() const
{
    FNPC_MemoryEntry Best;
    Best.ThreatScore = -1.0f;

    for (const FNPC_MemoryEntry& Mem : MemoryBank)
    {
        if (Mem.ThreatScore > Best.ThreatScore)
        {
            Best = Mem;
        }
    }
    return Best;
}

// ============================================================
// Daily Routine
// ============================================================

void UNPCBehaviorComponent::UpdateDailyRoutine(float GameHour)
{
    ENPC_DailyRoutineSlot Slot = GetCurrentTimeSlot(GameHour);

    for (const FNPC_DailyRoutineEntry& Entry : DailyRoutine)
    {
        if (Entry.TimeSlot == Slot)
        {
            // Only apply routine if not currently fleeing or dead
            if (CurrentState != ENPC_BehaviorState::Flee &&
                CurrentState != ENPC_BehaviorState::Dead)
            {
                SetBehaviorState(Entry.DesiredState);
            }
            break;
        }
    }
}

ENPC_DailyRoutineSlot UNPCBehaviorComponent::GetCurrentTimeSlot(float GameHour) const
{
    // GameHour is 0-24
    if      (GameHour <  5.0f) return ENPC_DailyRoutineSlot::Night;
    else if (GameHour <  8.0f) return ENPC_DailyRoutineSlot::EarlyMorning;
    else if (GameHour < 12.0f) return ENPC_DailyRoutineSlot::Morning;
    else if (GameHour < 14.0f) return ENPC_DailyRoutineSlot::Midday;
    else if (GameHour < 18.0f) return ENPC_DailyRoutineSlot::Afternoon;
    else if (GameHour < 21.0f) return ENPC_DailyRoutineSlot::Evening;
    else                        return ENPC_DailyRoutineSlot::Night;
}

// ============================================================
// Social
// ============================================================

void UNPCBehaviorComponent::UpdateTrustLevel(AActor* OtherActor, float Delta)
{
    if (!OtherActor) return;

    for (FNPC_SocialRelation& Rel : SocialRelations)
    {
        if (Rel.RelatedActor == OtherActor)
        {
            Rel.TrustLevel = FMath::Clamp(Rel.TrustLevel + Delta, -1.0f, 1.0f);
            return;
        }
    }

    // New relation
    FNPC_SocialRelation NewRel;
    NewRel.RelatedActor = OtherActor;
    NewRel.TrustLevel   = FMath::Clamp(Delta, -1.0f, 1.0f);
    NewRel.bIsTribemate = false;
    SocialRelations.Add(NewRel);
}

float UNPCBehaviorComponent::GetTrustLevel(AActor* OtherActor) const
{
    if (!OtherActor) return 0.0f;

    for (const FNPC_SocialRelation& Rel : SocialRelations)
    {
        if (Rel.RelatedActor == OtherActor)
        {
            return Rel.TrustLevel;
        }
    }
    return 0.0f; // Neutral default
}

// ============================================================
// Private helpers
// ============================================================

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    for (FNPC_MemoryEntry& Mem : MemoryBank)
    {
        Mem.TimeSinceObserved += DeltaTime;
    }
    ForgetStaleMemories(MemoryMaxAge);
}

void UNPCBehaviorComponent::TickBehaviorState(float DeltaTime)
{
    StateTimer += DeltaTime;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        // Nothing — waiting for routine or threat
        break;

    case ENPC_BehaviorState::Patrol:
        // Patrol logic handled by BT / AIController
        // Component just tracks time in state
        break;

    case ENPC_BehaviorState::Forage:
        // Foraging — slow movement, random direction within patrol radius
        break;

    case ENPC_BehaviorState::Alert:
        // Scan for threats — if none found after 10s, return to routine
        if (StateTimer > 10.0f && CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        break;

    case ENPC_BehaviorState::Flee:
        // Flee until threat score drops below threshold
        if (CurrentThreatScore < FleeThreshold * 0.5f)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
        break;

    case ENPC_BehaviorState::Rest:
        // Resting — stationary, low awareness
        break;

    case ENPC_BehaviorState::Socialise:
        // Social interaction — handled by dialogue / animation system
        break;

    case ENPC_BehaviorState::Seek:
        // Seeking a resource or target
        break;

    case ENPC_BehaviorState::Dead:
        // No tick logic
        break;
    }
}
