// NPCBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Full implementation: patrol state machine, sight/hearing detection, memory decay, daily routine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for NPC behavior
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        HomeLocation = OwnerCharacter->GetActorLocation();
    }

    // Auto-generate patrol waypoints in a circle if none are set
    if (PatrolWaypoints.Num() == 0 && PatrolRadius > 0.0f)
    {
        const int32 NumWaypoints = 4;
        for (int32 i = 0; i < NumWaypoints; ++i)
        {
            float Angle = (360.0f / NumWaypoints) * i;
            float RadAngle = FMath::DegreesToRadians(Angle);
            FNPC_PatrolWaypoint WP;
            WP.WorldLocation = HomeLocation + FVector(
                FMath::Cos(RadAngle) * PatrolRadius,
                FMath::Sin(RadAngle) * PatrolRadius,
                0.0f
            );
            WP.WaitDuration = FMath::RandRange(1.5f, 4.0f);
            PatrolWaypoints.Add(WP);
        }
    }

    TransitionToState(ENPC_BehaviorState::Patrolling);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    TickMemoryDecay(DeltaTime);
    TickAlertDecay(DeltaTime);
    TickDetection(DeltaTime);

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Patrolling:
        case ENPC_BehaviorState::Idle:
            TickPatrol(DeltaTime);
            break;

        case ENPC_BehaviorState::Fleeing:
            // Flee logic: keep moving away from highest-threat memory entry
            // Actual movement handled by AIController via MoveToLocation
            // Here we update FearLevel for AnimInstance
            FearLevel = FMath::FInterpTo(FearLevel, 0.9f, DeltaTime, 3.0f);
            break;

        case ENPC_BehaviorState::Investigating:
            // Move toward last known location of stimulus
            FearLevel = FMath::FInterpTo(FearLevel, 0.4f, DeltaTime, 2.0f);
            break;

        default:
            break;
    }
}

// ─── Patrol ──────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0)
    {
        return;
    }

    if (bWaitingAtWaypoint)
    {
        WaypointWaitTimer -= DeltaTime;
        if (WaypointWaitTimer <= 0.0f)
        {
            bWaitingAtWaypoint = false;
            CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        }
        return;
    }

    const FNPC_PatrolWaypoint& Target = PatrolWaypoints[CurrentWaypointIndex];
    FVector OwnerLoc = OwnerCharacter->GetActorLocation();
    float DistToWaypoint = FVector::Dist2D(OwnerLoc, Target.WorldLocation);

    if (DistToWaypoint < 150.0f)
    {
        // Arrived — start wait
        bWaitingAtWaypoint = true;
        WaypointWaitTimer = Target.WaitDuration;
        TransitionToState(ENPC_BehaviorState::Idle);
        return;
    }

    // Move toward waypoint via AIController
    AAIController* AIC = Cast<AAIController>(OwnerCharacter->GetController());
    if (AIC)
    {
        AIC->MoveToLocation(Target.WorldLocation, 100.0f, true, true, false, true);
    }

    TransitionToState(ENPC_BehaviorState::Patrolling);
}

// ─── Detection ───────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickDetection(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get player pawn as primary threat target
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector OwnerLoc = OwnerCharacter->GetActorLocation();
    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    float DistToPlayer = FVector::Dist(OwnerLoc, PlayerLoc);

    // Hearing check — omnidirectional
    if (DistToPlayer <= HearingRange)
    {
        float HearingWeight = 1.0f - (DistToPlayer / HearingRange);
        RegisterStimulus(PlayerPawn, ENPC_ThreatTier::Noise, HearingWeight * 0.5f);
    }

    // Sight check — cone + range
    if (DistToPlayer <= SightRange && CanSeeActor(PlayerPawn))
    {
        float SightWeight = 1.0f - (DistToPlayer / SightRange);
        RegisterStimulus(PlayerPawn, ENPC_ThreatTier::Sight, SightWeight);

        // If player is very close and visible — flee
        if (DistToPlayer <= FleeDistance && AlertLevel > 0.6f)
        {
            ForceFlee(PlayerLoc);
        }
    }
}

bool UNPCBehaviorComponent::CanSeeActor(AActor* Target) const
{
    if (!Target || !OwnerCharacter)
    {
        return false;
    }

    FVector OwnerLoc = OwnerCharacter->GetActorLocation() + FVector(0, 0, 60.0f); // eye height
    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();
    FVector OwnerForward = OwnerCharacter->GetActorForwardVector();

    // Cone check
    float DotProduct = FVector::DotProduct(OwnerForward, ToTarget);
    float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(SightAngleDegrees));
    if (DotProduct < HalfAngleCos)
    {
        return false;
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLoc,
        TargetLoc,
        ECC_Visibility,
        Params
    );

    // If we hit something that isn't the target, sight is blocked
    if (bHit && HitResult.GetActor() != Target)
    {
        return false;
    }

    return true;
}

// ─── Memory ──────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterStimulus(AActor* Source, ENPC_ThreatTier Tier, float Weight)
{
    if (!Source)
    {
        return;
    }

    // Update existing memory if actor already known
    for (FNPC_MemoryEntry& Entry : MemoryBank)
    {
        if (Entry.SourceActor == Source)
        {
            Entry.LastKnownLocation = Source->GetActorLocation();
            Entry.TimeStamp = GetWorld()->GetTimeSeconds();
            Entry.ThreatWeight = FMath::Max(Entry.ThreatWeight, Weight);
            Entry.ThreatTier = (Tier > Entry.ThreatTier) ? Tier : Entry.ThreatTier;
            Entry.bStillValid = true;

            // Update alert level
            AlertLevel = FMath::Clamp(AlertLevel + Weight * 0.3f, 0.0f, 1.0f);
            return;
        }
    }

    // New memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.SourceActor = Source;
    NewEntry.LastKnownLocation = Source->GetActorLocation();
    NewEntry.TimeStamp = GetWorld()->GetTimeSeconds();
    NewEntry.ThreatWeight = Weight;
    NewEntry.ThreatTier = Tier;
    NewEntry.bStillValid = true;
    AddMemoryEntry(NewEntry);

    AlertLevel = FMath::Clamp(AlertLevel + Weight * 0.5f, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::AddMemoryEntry(FNPC_MemoryEntry Entry)
{
    if (MemoryBank.Num() >= MaxMemoryEntries)
    {
        // Evict oldest entry
        int32 OldestIdx = 0;
        float OldestTime = MemoryBank[0].TimeStamp;
        for (int32 i = 1; i < MemoryBank.Num(); ++i)
        {
            if (MemoryBank[i].TimeStamp < OldestTime)
            {
                OldestTime = MemoryBank[i].TimeStamp;
                OldestIdx = i;
            }
        }
        MemoryBank.RemoveAt(OldestIdx);
    }
    MemoryBank.Add(Entry);
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (FNPC_MemoryEntry& Entry : MemoryBank)
    {
        float Age = CurrentTime - Entry.TimeStamp;
        if (Age > MemoryDecaySeconds)
        {
            Entry.bStillValid = false;
            Entry.ThreatWeight = 0.0f;
        }
        else
        {
            // Gradual weight decay
            float DecayFactor = 1.0f - (Age / MemoryDecaySeconds);
            Entry.ThreatWeight *= DecayFactor;
        }
    }

    // Remove fully decayed entries
    MemoryBank.RemoveAll([](const FNPC_MemoryEntry& E) { return !E.bStillValid; });
}

void UNPCBehaviorComponent::TickAlertDecay(float DeltaTime)
{
    // Alert level decays toward 0 when no new stimuli — 0.05 per second
    if (MemoryBank.Num() == 0)
    {
        AlertLevel = FMath::FInterpTo(AlertLevel, 0.0f, DeltaTime, 0.05f);
        FearLevel = FMath::FInterpTo(FearLevel, 0.0f, DeltaTime, 0.1f);
    }
}

// ─── State transitions ────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }
    CurrentState = NewState;
}

// ─── Daily routine ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetDailyPhase(ENPC_DailyRoutinePhase NewPhase)
{
    CurrentRoutinePhase = NewPhase;

    // Adjust behavior parameters based on time of day
    switch (NewPhase)
    {
        case ENPC_DailyRoutinePhase::Dawn:
            PatrolMoveSpeed = 350.0f;
            SightRange = 2500.0f;   // Good light — wider sight
            HearingRange = 700.0f;
            break;

        case ENPC_DailyRoutinePhase::Midday:
            PatrolMoveSpeed = 200.0f;   // Slower — heat
            SightRange = 3000.0f;       // Best visibility
            HearingRange = 600.0f;
            break;

        case ENPC_DailyRoutinePhase::Dusk:
            PatrolMoveSpeed = 300.0f;
            SightRange = 1500.0f;       // Dimming light
            HearingRange = 900.0f;      // More reliant on hearing
            break;

        case ENPC_DailyRoutinePhase::Night:
            PatrolMoveSpeed = 150.0f;   // Slow — resting
            SightRange = 600.0f;        // Near-blind in darkness
            HearingRange = 1200.0f;     // Heightened hearing at night
            break;
    }
}

// ─── Flee ────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::ForceFlee(FVector ThreatLocation)
{
    TransitionToState(ENPC_BehaviorState::Fleeing);
    FearLevel = 1.0f;

    if (!OwnerCharacter)
    {
        return;
    }

    FVector OwnerLoc = OwnerCharacter->GetActorLocation();
    FVector FleeDirection = (OwnerLoc - ThreatLocation).GetSafeNormal();
    FVector FleeTarget = OwnerLoc + FleeDirection * FleeDistance;

    AAIController* AIC = Cast<AAIController>(OwnerCharacter->GetController());
    if (AIC)
    {
        AIC->MoveToLocation(FleeTarget, 50.0f, true, true, false, true);
    }
}

// ─── Query API ───────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    for (const FNPC_MemoryEntry& Entry : MemoryBank)
    {
        if (Entry.SourceActor == Actor && Entry.bStillValid)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownLocationOf(AActor* Actor) const
{
    if (!Actor)
    {
        return FVector::ZeroVector;
    }
    for (const FNPC_MemoryEntry& Entry : MemoryBank)
    {
        if (Entry.SourceActor == Actor && Entry.bStillValid)
        {
            return Entry.LastKnownLocation;
        }
    }
    return FVector::ZeroVector;
}
