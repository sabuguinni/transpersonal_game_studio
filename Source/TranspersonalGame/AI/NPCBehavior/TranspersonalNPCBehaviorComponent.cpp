#include "TranspersonalNPCBehaviorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================

UTranspersonalNPCBehaviorComponent::UTranspersonalNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC AI, saves perf
}

// ============================================================
// BeginPlay
// ============================================================

void UTranspersonalNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache spawn origin for patrol generation
    if (GetOwner())
    {
        SpawnOrigin = GetOwner()->GetActorLocation();
    }

    // Auto-generate patrol points if none assigned
    if (PatrolPoints.Num() == 0 && PatrolRadius > 0.0f)
    {
        GeneratePatrolPointsAroundOrigin(SpawnOrigin, 4);
    }

    // Cache blackboard from owning AI controller
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BlackboardComp = AIC->GetBlackboardComponent();

            // Run behavior tree if assigned
            if (BehaviorTreeAsset && AIC)
            {
                AIC->RunBehaviorTree(BehaviorTreeAsset);
            }
        }
    }

    // Start in appropriate state
    SetBehaviorState(ENPC_BehaviorState::Patrolling);
}

// ============================================================
// TickComponent
// ============================================================

void UTranspersonalNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickMemoryDecay(DeltaTime);
    UpdateThreatAwareness(DeltaTime);
    SyncStateToBlackboard();

    // Alert cooldown timer
    if (TimeSinceLastAlert < AlertCooldown)
    {
        TimeSinceLastAlert += DeltaTime;
    }

    // Fear naturally decays when no active threat
    if (!HasActiveThreat() && FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 0.2f);
    }
}

// ============================================================
// RegisterThreat
// ============================================================

void UTranspersonalNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float InThreatLevel, FVector LastKnownLocation)
{
    if (!ThreatActor)
    {
        return;
    }

    // Check if this threat is already tracked — update if so
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatActor == ThreatActor && Entry.bIsActive)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, InThreatLevel);
            Entry.LastKnownLocation = LastKnownLocation;
            Entry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            FearLevel = FMath::Clamp(FearLevel + InThreatLevel * 0.3f, 0.0f, 1.0f);
            return;
        }
    }

    // Evict oldest entry if at capacity
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        int32 OldestIdx = 0;
        float OldestTime = TNumericLimits<float>::Max();
        for (int32 i = 0; i < MemoryEntries.Num(); ++i)
        {
            if (MemoryEntries[i].TimeStamp < OldestTime)
            {
                OldestTime = MemoryEntries[i].TimeStamp;
                OldestIdx = i;
            }
        }
        MemoryEntries.RemoveAt(OldestIdx);
    }

    // Add new memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = InThreatLevel;
    NewEntry.LastKnownLocation = LastKnownLocation;
    NewEntry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEntry.bIsActive = true;
    MemoryEntries.Add(NewEntry);

    // Escalate fear
    FearLevel = FMath::Clamp(FearLevel + InThreatLevel * 0.5f, 0.0f, 1.0f);

    // Respond to threat level
    ENPC_ThreatResponse Response = EvaluateThreatResponse(InThreatLevel);
    switch (Response)
    {
        case ENPC_ThreatResponse::Flee:
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
            break;
        case ENPC_ThreatResponse::Attack:
            SetBehaviorState(ENPC_BehaviorState::Attacking);
            break;
        case ENPC_ThreatResponse::Investigate:
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            break;
        case ENPC_ThreatResponse::Alert:
            SetBehaviorState(ENPC_BehaviorState::Alerted);
            break;
        default:
            break;
    }
}

// ============================================================
// ClearThreat
// ============================================================

void UTranspersonalNPCBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return;
    }

    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.bIsActive = false;
            Entry.ThreatLevel = 0.0f;
        }
    }

    // Return to patrol if no remaining threats
    if (!HasActiveThreat())
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

// ============================================================
// EvaluateThreatResponse
// ============================================================

ENPC_ThreatResponse UTranspersonalNPCBehaviorComponent::EvaluateThreatResponse(float IncomingThreatLevel) const
{
    // High fear + high threat = flee
    if (FearLevel > 0.7f && IncomingThreatLevel > 0.5f)
    {
        return ENPC_ThreatResponse::Flee;
    }

    // Very high threat with low fear = attack (aggressive NPC)
    if (IncomingThreatLevel > 0.8f && FearLevel < 0.4f)
    {
        return ENPC_ThreatResponse::Attack;
    }

    // Medium-high threat = alert others
    if (IncomingThreatLevel > 0.5f)
    {
        return ENPC_ThreatResponse::Alert;
    }

    // Low-medium threat = investigate
    if (IncomingThreatLevel > 0.2f)
    {
        return ENPC_ThreatResponse::Investigate;
    }

    return ENPC_ThreatResponse::Ignore;
}

// ============================================================
// SetBehaviorState
// ============================================================

void UTranspersonalNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    // Sync to blackboard immediately on state change
    SyncStateToBlackboard();
}

// ============================================================
// Patrol
// ============================================================

void UTranspersonalNPCBehaviorComponent::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

FVector UTranspersonalNPCBehaviorComponent::GetCurrentPatrolTarget() const
{
    if (PatrolPoints.Num() == 0)
    {
        return SpawnOrigin;
    }

    return PatrolPoints[CurrentPatrolIndex].Location;
}

void UTranspersonalNPCBehaviorComponent::GeneratePatrolPointsAroundOrigin(FVector Origin, int32 NumPoints)
{
    PatrolPoints.Empty();

    for (int32 i = 0; i < NumPoints; ++i)
    {
        float Angle = (360.0f / NumPoints) * i;
        float RadAngle = FMath::DegreesToRadians(Angle);

        // Vary radius per point for natural-looking patrol
        float Radius = PatrolRadius * FMath::RandRange(0.5f, 1.0f);

        FNPC_PatrolPoint Point;
        Point.Location = Origin + FVector(FMath::Cos(RadAngle) * Radius, FMath::Sin(RadAngle) * Radius, 0.0f);
        Point.WaitDuration = FMath::RandRange(1.5f, 4.0f);
        Point.bLookAround = (i % 2 == 0); // Look around at alternate points
        PatrolPoints.Add(Point);
    }
}

// ============================================================
// Memory queries
// ============================================================

float UTranspersonalNPCBehaviorComponent::GetHighestActiveThreat() const
{
    float Highest = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive)
        {
            Highest = FMath::Max(Highest, Entry.ThreatLevel);
        }
    }
    return Highest;
}

bool UTranspersonalNPCBehaviorComponent::HasActiveThreat() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > 0.1f)
        {
            return true;
        }
    }
    return false;
}

// ============================================================
// Daily routine — driven by game time (0-24 hours)
// ============================================================

void UTranspersonalNPCBehaviorComponent::UpdateDailyRoutine(float GameTimeHours)
{
    // Prehistoric human NPC daily schedule
    if (GameTimeHours >= 22.0f || GameTimeHours < 5.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Sleeping;
        if (CurrentState != ENPC_BehaviorState::Alerted && CurrentState != ENPC_BehaviorState::Attacking)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
    else if (GameTimeHours >= 5.0f && GameTimeHours < 8.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Foraging;
        if (CurrentState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
    }
    else if (GameTimeHours >= 8.0f && GameTimeHours < 12.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Hunting;
        if (CurrentState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
    }
    else if (GameTimeHours >= 12.0f && GameTimeHours < 14.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Resting;
        if (CurrentState == ENPC_BehaviorState::Patrolling)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
    else if (GameTimeHours >= 14.0f && GameTimeHours < 18.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Socialising;
        if (CurrentState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
    }
    else if (GameTimeHours >= 18.0f && GameTimeHours < 22.0f)
    {
        CurrentRoutinePhase = ENPC_DailyRoutinePhase::Guarding;
        if (CurrentState == ENPC_BehaviorState::Patrolling)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

// ============================================================
// Private helpers
// ============================================================

void UTranspersonalNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (!Entry.bIsActive)
        {
            continue;
        }

        // Decay threat level over time — NPCs forget old threats
        float Age = CurrentTime - Entry.TimeStamp;
        if (Age > 30.0f) // After 30 seconds, start forgetting
        {
            Entry.ThreatLevel = FMath::Max(0.0f, Entry.ThreatLevel - MemoryDecayRate * DeltaTime);
            if (Entry.ThreatLevel <= 0.01f)
            {
                Entry.bIsActive = false;
            }
        }
    }
}

void UTranspersonalNPCBehaviorComponent::UpdateThreatAwareness(float DeltaTime)
{
    float HighestThreat = GetHighestActiveThreat();
    // Smooth awareness update — doesn't spike instantly
    ThreatAwareness = FMath::FInterpTo(ThreatAwareness, HighestThreat, DeltaTime, 2.0f);
}

void UTranspersonalNPCBehaviorComponent::SyncStateToBlackboard()
{
    if (!BlackboardComp)
    {
        return;
    }

    // Write NPC state to blackboard keys for Behavior Tree to read
    BlackboardComp->SetValueAsEnum(FName("BehaviorState"), static_cast<uint8>(CurrentState));
    BlackboardComp->SetValueAsFloat(FName("ThreatAwareness"), ThreatAwareness);
    BlackboardComp->SetValueAsFloat(FName("FearLevel"), FearLevel);
    BlackboardComp->SetValueAsBool(FName("HasActiveThreat"), HasActiveThreat());
    BlackboardComp->SetValueAsVector(FName("PatrolTarget"), GetCurrentPatrolTarget());
}
