// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implementation of universal NPC behavior: patrol, investigate, flee, memory, senses

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    CurrentDailyPhase = ENPC_DailyPhase::Morning;
    AlertLevel = 0.0f;
    bIsAwareOfPlayer = false;
    FleeSpeedMultiplier = 1.8f;
    AlertThreshold = 0.4f;
    FleeThreshold = 0.75f;
    CurrentWaypointIndex = 0;
    StateTimer = 0.0f;
    ThreatDecayTimer = 0.0f;
    TimeSincePlayerSeen = 0.0f;
    LastKnownPlayerLocation = FVector::ZeroVector;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in patrol if waypoints are configured
    if (PatrolConfig.WaypointLocations.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    TimeSincePlayerSeen += DeltaTime;

    TickAlertDecay(DeltaTime);
    TickThreatMemories(DeltaTime);
    EvaluateStateTransition();
}

// ─── State Management ─────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return; // Dead is terminal

    OnStateExit(CurrentState);
    CurrentState = NewState;
    StateTimer = 0.0f;
    OnStateEnter(NewState);
}

void UNPCBehaviorComponent::OnStateEnter(ENPC_BehaviorState NewState)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    switch (NewState)
    {
    case ENPC_BehaviorState::Patrol:
        // Reset waypoint if starting fresh
        if (CurrentWaypointIndex >= PatrolConfig.WaypointLocations.Num())
        {
            CurrentWaypointIndex = 0;
        }
        break;

    case ENPC_BehaviorState::Alert:
        // Boost alert level on entry
        AlertLevel = FMath::Max(AlertLevel, AlertThreshold + 0.1f);
        break;

    case ENPC_BehaviorState::Flee:
        // Apply flee speed multiplier to character movement
        if (ACharacter* Char = Cast<ACharacter>(Owner))
        {
            UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed *= FleeSpeedMultiplier;
            }
        }
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::OnStateExit(ENPC_BehaviorState OldState)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    switch (OldState)
    {
    case ENPC_BehaviorState::Flee:
        // Restore normal speed after fleeing
        if (ACharacter* Char = Cast<ACharacter>(Owner))
        {
            UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed /= FleeSpeedMultiplier;
            }
        }
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::EvaluateStateTransition()
{
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    // High threat → flee
    if (AlertLevel >= FleeThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // Medium threat → alert/investigate
    if (AlertLevel >= AlertThreshold && CurrentState == ENPC_BehaviorState::Patrol)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    // Alert decayed → return to patrol
    if (AlertLevel < AlertThreshold * 0.3f && CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }

    // Flee ended (threat gone for 10s) → investigate last known location
    if (CurrentState == ENPC_BehaviorState::Flee && AlertLevel < AlertThreshold && StateTimer > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
        return;
    }

    // Investigation complete (5s) → return to patrol
    if (CurrentState == ENPC_BehaviorState::Investigate && StateTimer > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }
}

// ─── Sensory System ───────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::CanSeeActor(AActor* Target) const
{
    if (!Target || !GetOwner()) return false;

    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();
    float Distance = FVector::Dist(OwnerLoc, TargetLoc);

    if (Distance > SensoryConfig.SightRange) return false;

    // Angle check
    FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();
    FVector OwnerForward = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(OwnerForward, ToTarget);
    float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(SensoryConfig.SightAngleDegrees * 0.5f));

    if (DotProduct < HalfAngleCos) return false;

    // Line of sight trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLoc + FVector(0, 0, 60.0f),
        TargetLoc + FVector(0, 0, 60.0f),
        ECC_Visibility,
        QueryParams
    );

    return !bBlocked;
}

bool UNPCBehaviorComponent::CanHearActor(AActor* Target, float NoiseLevel) const
{
    if (!Target || !GetOwner()) return false;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float EffectiveHearingRange = SensoryConfig.HearingRange * NoiseLevel;

    return Distance <= EffectiveHearingRange;
}

// ─── Threat Memory ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float Intensity, ENPC_ThreatLevel Level)
{
    if (!ThreatActor) return;

    // Check if already tracking this threat
    for (FNPC_ThreatMemory& Memory : ThreatMemories)
    {
        if (Memory.ThreatActor == ThreatActor)
        {
            Memory.ThreatIntensity = FMath::Max(Memory.ThreatIntensity, Intensity);
            Memory.LastKnownLocation = ThreatActor->GetActorLocation();
            Memory.TimeLastSeen = 0.0f;
            Memory.ThreatLevel = Level;
            AlertLevel = FMath::Clamp(AlertLevel + Intensity * 0.3f, 0.0f, 1.0f);
            return;
        }
    }

    // New threat
    FNPC_ThreatMemory NewMemory;
    NewMemory.ThreatActor = ThreatActor;
    NewMemory.LastKnownLocation = ThreatActor->GetActorLocation();
    NewMemory.ThreatIntensity = Intensity;
    NewMemory.TimeLastSeen = 0.0f;
    NewMemory.ThreatLevel = Level;
    ThreatMemories.Add(NewMemory);

    AlertLevel = FMath::Clamp(AlertLevel + Intensity * 0.5f, 0.0f, 1.0f);
    bIsAwareOfPlayer = true;
}

void UNPCBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    ThreatMemories.RemoveAll([ThreatActor](const FNPC_ThreatMemory& M) {
        return M.ThreatActor == ThreatActor;
    });

    if (ThreatMemories.Num() == 0)
    {
        bIsAwareOfPlayer = false;
    }
}

ENPC_ThreatLevel UNPCBehaviorComponent::EvaluateThreatLevel() const
{
    if (AlertLevel >= 0.9f) return ENPC_ThreatLevel::Critical;
    if (AlertLevel >= 0.7f) return ENPC_ThreatLevel::High;
    if (AlertLevel >= 0.4f) return ENPC_ThreatLevel::Medium;
    if (AlertLevel >= 0.1f) return ENPC_ThreatLevel::Low;
    return ENPC_ThreatLevel::None;
}

void UNPCBehaviorComponent::UpdateDailyPhase(ENPC_DailyPhase NewPhase)
{
    CurrentDailyPhase = NewPhase;

    // Adjust sensory ranges based on time of day
    switch (NewPhase)
    {
    case ENPC_DailyPhase::Night:
        SensoryConfig.SightRange *= 0.5f;   // Reduced night vision
        SensoryConfig.HearingRange *= 1.3f; // Heightened hearing at night
        break;
    case ENPC_DailyPhase::Dawn:
    case ENPC_DailyPhase::Dusk:
        SensoryConfig.SightRange *= 0.75f;
        break;
    default:
        break;
    }
}

// ─── Patrol System ────────────────────────────────────────────────────────────

FVector UNPCBehaviorComponent::GetNextPatrolWaypoint()
{
    if (PatrolConfig.WaypointLocations.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    if (PatrolConfig.bRandomizeOrder)
    {
        CurrentWaypointIndex = FMath::RandRange(0, PatrolConfig.WaypointLocations.Num() - 1);
    }

    return PatrolConfig.WaypointLocations[CurrentWaypointIndex];
}

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolConfig.WaypointLocations.Num() == 0) return;

    if (PatrolConfig.bRandomizeOrder)
    {
        CurrentWaypointIndex = FMath::RandRange(0, PatrolConfig.WaypointLocations.Num() - 1);
    }
    else
    {
        CurrentWaypointIndex++;
        if (CurrentWaypointIndex >= PatrolConfig.WaypointLocations.Num())
        {
            CurrentWaypointIndex = PatrolConfig.bLoopPatrol ? 0 : PatrolConfig.WaypointLocations.Num() - 1;
        }
    }
}

// ─── Internal Tick Helpers ────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickAlertDecay(float DeltaTime)
{
    if (AlertLevel > 0.0f && CurrentState != ENPC_BehaviorState::Alert && CurrentState != ENPC_BehaviorState::Flee)
    {
        AlertLevel = FMath::Max(0.0f, AlertLevel - SensoryConfig.AlertDecayRate * DeltaTime);
    }

    CurrentThreatLevel = EvaluateThreatLevel();
}

void UNPCBehaviorComponent::TickThreatMemories(float DeltaTime)
{
    for (FNPC_ThreatMemory& Memory : ThreatMemories)
    {
        Memory.TimeLastSeen += DeltaTime;
    }

    // Remove memories older than MemoryDuration
    ThreatMemories.RemoveAll([this](const FNPC_ThreatMemory& M) {
        return M.TimeLastSeen > SensoryConfig.MemoryDuration;
    });

    if (ThreatMemories.Num() == 0)
    {
        bIsAwareOfPlayer = false;
    }
}
