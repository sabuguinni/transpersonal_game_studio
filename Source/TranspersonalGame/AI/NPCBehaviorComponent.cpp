// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_003
// Full implementation: state evaluation, patrol cycling, memory decay, BT blackboard sync.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for AI, saves perf
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Seed default patrol points in a cross pattern around spawn location
    if (PatrolPoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector Origin = Owner->GetActorLocation();
            float R = PatrolRadius * 0.5f;
            TArray<FVector> Offsets = {
                FVector(R, 0, 0),
                FVector(0, R, 0),
                FVector(-R, 0, 0),
                FVector(0, -R, 0),
            };
            for (const FVector& Offset : Offsets)
            {
                FNPC_PatrolPoint PP;
                PP.Location = Origin + Offset;
                PP.WaitDuration = FMath::RandRange(1.5f, 4.0f);
                PP.bIsReached = false;
                PatrolPoints.Add(PP);
            }
        }
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ElapsedTime += DeltaTime;

    // Cache distance to player
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            AActor* Owner = GetOwner();
            if (Owner)
            {
                DistanceToPlayerCached = FVector::Dist(Owner->GetActorLocation(), PC->GetPawn()->GetActorLocation());
            }
        }
    }

    // Decay memory entries
    TickMemoryDecay(DeltaTime);

    // Evaluate and update state
    CurrentState = EvaluateState(DistanceToPlayerCached);

    // Sync blackboard keys
    UpdateBlackboardKeys();
}

// ─── State Evaluation ─────────────────────────────────────────────────────────

ENPC_BehaviorState UNPCBehaviorComponent::EvaluateState(float DistanceToPlayer)
{
    // Lethal threat: flee if health is low, attack if healthy
    if (DistanceToPlayer <= AttackRadius)
    {
        if (HealthNormalized < 0.25f || FearLevel > 0.8f)
        {
            bIsFleeing = true;
            bIsAttacking = false;
            return ENPC_BehaviorState::Flee;
        }
        bIsAttacking = true;
        bIsFleeing = false;
        return ENPC_BehaviorState::Combat;
    }

    // Aggro range: pursue or alert
    if (DistanceToPlayer <= AggroRadius)
    {
        bThreatVisible = true;
        bIsAttacking = false;
        bIsFleeing = false;

        if (FearLevel > 0.6f || HealthNormalized < 0.4f)
        {
            return ENPC_BehaviorState::Flee;
        }
        return ENPC_BehaviorState::Alert;
    }

    // Out of aggro — check memory for last known threat
    bThreatVisible = false;
    bIsAttacking = false;

    FNPC_MemoryEntry Recent = GetMostRecentThreat();
    if (Recent.bIsActive && Recent.ThreatLevel >= ENPC_ThreatLevel::Medium)
    {
        return ENPC_BehaviorState::Investigate;
    }

    // Default: patrol or rest based on routine phase
    if (RoutinePhase == ENPC_DailyRoutinePhase::Night || RoutinePhase == ENPC_DailyRoutinePhase::DeepNight)
    {
        return ENPC_BehaviorState::Rest;
    }

    if (HungerNormalized < 0.3f)
    {
        return ENPC_BehaviorState::Forage;
    }

    return ENPC_BehaviorState::Patrol;
}

// ─── Daily Routine ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateDailyRoutine(float GameHour)
{
    // GameHour: 0-24
    if (GameHour >= 5.0f && GameHour < 7.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Dawn;
    else if (GameHour >= 7.0f && GameHour < 12.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Morning;
    else if (GameHour >= 12.0f && GameHour < 14.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Midday;
    else if (GameHour >= 14.0f && GameHour < 18.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Afternoon;
    else if (GameHour >= 18.0f && GameHour < 20.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Dusk;
    else if (GameHour >= 20.0f && GameHour < 23.0f)
        RoutinePhase = ENPC_DailyRoutinePhase::Night;
    else
        RoutinePhase = ENPC_DailyRoutinePhase::DeepNight;
}

// ─── Patrol ───────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AdvancePatrolIndex()
{
    if (PatrolPoints.Num() == 0) return;

    if (CurrentPatrolIndex < PatrolPoints.Num())
    {
        PatrolPoints[CurrentPatrolIndex].bIsReached = true;
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

// ─── Memory ───────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, ENPC_ThreatLevel Level)
{
    // Find an inactive slot or evict the oldest
    int32 SlotIndex = -1;
    float OldestTime = TNumericLimits<float>::Max();
    int32 OldestIndex = 0;

    for (int32 i = 0; i < MemoryEntries.Num(); ++i)
    {
        if (!MemoryEntries[i].bIsActive)
        {
            SlotIndex = i;
            break;
        }
        if (MemoryEntries[i].ThreatTimestamp < OldestTime)
        {
            OldestTime = MemoryEntries[i].ThreatTimestamp;
            OldestIndex = i;
        }
    }

    if (SlotIndex == -1)
    {
        if (MemoryEntries.Num() < MaxMemoryEntries)
        {
            MemoryEntries.AddDefaulted();
            SlotIndex = MemoryEntries.Num() - 1;
        }
        else
        {
            SlotIndex = OldestIndex;
        }
    }

    FNPC_MemoryEntry& Entry = MemoryEntries[SlotIndex];
    Entry.ThreatLocation = ThreatLocation;
    Entry.ThreatTimestamp = ElapsedTime;
    Entry.ThreatLevel = Level;
    Entry.bIsActive = true;

    // Update blackboard
    LastKnownThreatLocation = ThreatLocation;
    CurrentThreatLevel = Level;

    // Raise fear proportional to threat level
    float FearDelta = 0.0f;
    switch (Level)
    {
        case ENPC_ThreatLevel::Low:    FearDelta = 0.1f; break;
        case ENPC_ThreatLevel::Medium: FearDelta = 0.25f; break;
        case ENPC_ThreatLevel::High:   FearDelta = 0.5f; break;
        case ENPC_ThreatLevel::Lethal: FearDelta = 0.9f; break;
        default: break;
    }
    FearLevel = FMath::Clamp(FearLevel + FearDelta, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::ClearMemory()
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        Entry.bIsActive = false;
    }
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    FearLevel = FMath::Max(0.0f, FearLevel - 0.3f);
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (!Entry.bIsActive) continue;

        float Age = ElapsedTime - Entry.ThreatTimestamp;
        if (Age >= MemoryDecayTime)
        {
            Entry.bIsActive = false;
        }
    }

    // Gradually reduce fear when no active threats
    bool bAnyActiveThreat = false;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive) { bAnyActiveThreat = true; break; }
    }

    if (!bAnyActiveThreat)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 0.05f);
        if (CurrentThreatLevel != ENPC_ThreatLevel::None && FearLevel < 0.05f)
        {
            CurrentThreatLevel = ENPC_ThreatLevel::None;
        }
    }
}

// ─── Blackboard Sync ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateBlackboardKeys()
{
    // bThreatVisible and LastKnownThreatLocation are updated in EvaluateState / RegisterThreat
    // bIsFleeing and bIsAttacking are set in EvaluateState
    // This function is the extension point for future BT component integration
}

// ─── Queries ──────────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::IsPlayerInAggroRange() const
{
    return DistanceToPlayerCached <= AggroRadius;
}

bool UNPCBehaviorComponent::IsPlayerInAttackRange() const
{
    return DistanceToPlayerCached <= AttackRadius;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentThreat() const
{
    FNPC_MemoryEntry Best;
    float BestTime = -1.0f;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsActive && Entry.ThreatTimestamp > BestTime)
        {
            BestTime = Entry.ThreatTimestamp;
            Best = Entry;
        }
    }

    return Best;
}
