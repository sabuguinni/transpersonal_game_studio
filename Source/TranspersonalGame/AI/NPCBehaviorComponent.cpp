#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UNPCBehaviorComponent — Implementation
// Agent #11 — NPC Behavior Agent
// Prehistoric survival game — dinosaur survival context only.
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in Idle, evaluate first transition after one interval
    CurrentState = ENPC_BehaviorState::Idle;
    StateEvalTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateEvalTimer += DeltaTime;
    if (StateEvalTimer >= StateEvalInterval)
    {
        StateEvalTimer = 0.0f;
        EvaluateStateTransition();
    }

    TickStateLogic(DeltaTime);
}

// ── State Machine ─────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    UE_LOG(LogTemp, Verbose, TEXT("NPC [%s] state: %d -> %d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        (int32)CurrentState, (int32)NewState);

    CurrentState = NewState;
}

void UNPCBehaviorComponent::EvaluateStateTransition()
{
    // Flee override — health critical
    if (CurrentHealth <= FleeHealthThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // Threat-driven transitions
    if (CurrentThreatActor != nullptr && CurrentThreatLevel != ENPC_ThreatLevel::None)
    {
        float DistToThreat = GetDistanceToThreat();

        if (DistToThreat <= AttackRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Attack);
        }
        else if (DistToThreat <= ChaseRange)
        {
            if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
            {
                SetBehaviorState(ENPC_BehaviorState::Flee);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        return;
    }

    // No threat — routine behavior
    if (CurrentState == ENPC_BehaviorState::Attack ||
        CurrentState == ENPC_BehaviorState::Chase ||
        CurrentState == ENPC_BehaviorState::Flee ||
        CurrentState == ENPC_BehaviorState::Investigate)
    {
        // Return to patrol after threat resolved
        SetBehaviorState(PatrolPoints.Num() > 0 ? ENPC_BehaviorState::Patrol : ENPC_BehaviorState::Idle);
    }

    // Daily routine override
    if (bEnableDailyRoutine)
    {
        UpdateDailyRoutine(CurrentGameHour);
    }
}

void UNPCBehaviorComponent::TickStateLogic(float DeltaTime)
{
    // Advance game hour (1 real second = 1 game minute, 60 real seconds = 1 game hour)
    CurrentGameHour += DeltaTime / 60.0f;
    if (CurrentGameHour >= 24.0f)
    {
        CurrentGameHour -= 24.0f;
    }

    // Forget old memories periodically
    ForgetOldMemories(120.0f);
}

// ── Memory System ─────────────────────────────────────────────

void UNPCBehaviorComponent::RecordMemory(FVector Location, ENPC_ThreatLevel Threat, const FString& Description, bool bPlayerRelated)
{
    FNPC_MemoryEntry Entry;
    Entry.Location = Location;
    Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Entry.ThreatLevel = Threat;
    Entry.EventDescription = Description;
    Entry.bIsPlayerRelated = bPlayerRelated;

    MemoryLog.Add(Entry);

    // Trim to max entries (remove oldest)
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    if (!GetWorld())
    {
        return;
    }

    float Now = GetWorld()->GetTimeSeconds();
    MemoryLog.RemoveAll([Now, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.Timestamp) > MaxAge;
    });
}

bool UNPCBehaviorComponent::HasRecentThreatMemory(float WithinSeconds) const
{
    if (!GetWorld())
    {
        return false;
    }

    float Now = GetWorld()->GetTimeSeconds();
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.ThreatLevel != ENPC_ThreatLevel::None && (Now - Entry.Timestamp) <= WithinSeconds)
        {
            return true;
        }
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentMemory() const
{
    if (MemoryLog.Num() == 0)
    {
        return FNPC_MemoryEntry();
    }
    return MemoryLog.Last();
}

// ── Patrol ────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FNPC_PatrolPoint>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;

    if (PatrolPoints.Num() > 0 && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolTarget()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector Target = PatrolPoints[CurrentPatrolIndex].WorldLocation;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return Target;
}

// ── Threat Detection ──────────────────────────────────────────

void UNPCBehaviorComponent::OnThreatDetected(AActor* ThreatActor, ENPC_ThreatLevel Level)
{
    if (!ThreatActor)
    {
        return;
    }

    CurrentThreatActor = ThreatActor;
    CurrentThreatLevel = Level;

    // Record in memory
    RecordMemory(
        ThreatActor->GetActorLocation(),
        Level,
        FString::Printf(TEXT("Threat detected: %s"), *ThreatActor->GetName()),
        true
    );

    // Immediately evaluate state
    EvaluateStateTransition();
}

void UNPCBehaviorComponent::OnThreatLost()
{
    if (CurrentThreatActor)
    {
        // Record last known position before losing the threat
        RecordMemory(
            CurrentThreatActor->GetActorLocation(),
            CurrentThreatLevel,
            TEXT("Threat lost — last known position"),
            true
        );
    }

    CurrentThreatActor = nullptr;
    CurrentThreatLevel = ENPC_ThreatLevel::None;

    // Transition to investigate (check last known position)
    if (HasRecentThreatMemory(30.0f))
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
    else
    {
        SetBehaviorState(PatrolPoints.Num() > 0 ? ENPC_BehaviorState::Patrol : ENPC_BehaviorState::Idle);
    }
}

// ── Daily Routine ─────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateDailyRoutine(float GameHour)
{
    // Only override if no active threat
    if (CurrentThreatActor != nullptr)
    {
        return;
    }

    // Dawn (5-7): Forage
    // Day (7-17): Patrol / Forage
    // Dusk (17-19): Return to shelter
    // Night (19-5): Sleep

    if (GameHour >= 19.0f || GameHour < 5.0f)
    {
        if (CurrentState != ENPC_BehaviorState::Sleep && CurrentState != ENPC_BehaviorState::Shelter)
        {
            SetBehaviorState(ENPC_BehaviorState::Shelter);
        }
    }
    else if (GameHour >= 5.0f && GameHour < 7.0f)
    {
        if (CurrentState == ENPC_BehaviorState::Sleep || CurrentState == ENPC_BehaviorState::Shelter)
        {
            SetBehaviorState(ENPC_BehaviorState::Forage);
        }
    }
    else if (GameHour >= 7.0f && GameHour < 17.0f)
    {
        if (CurrentState == ENPC_BehaviorState::Forage || CurrentState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(PatrolPoints.Num() > 0 ? ENPC_BehaviorState::Patrol : ENPC_BehaviorState::Forage);
        }
    }
}

// ── Private Helpers ───────────────────────────────────────────

float UNPCBehaviorComponent::GetDistanceToThreat() const
{
    if (!CurrentThreatActor || !GetOwner())
    {
        return TNumericLimits<float>::Max();
    }
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreatActor->GetActorLocation());
}
