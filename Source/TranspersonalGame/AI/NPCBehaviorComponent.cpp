// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_001
// Full implementation of NPC behavior, memory, daily routines, and threat response

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogNPCBehavior, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC AI

    // Default state
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel   = ENPC_ThreatLevel::None;

    // Memory defaults
    MemoryRetentionTime    = 30.0f;
    MaxMemoryEntries       = 8;
    ThreatDetectionRadius  = 2000.0f;
    AlertRadius            = 3500.0f;

    // Patrol defaults
    PatrolRadius           = 1500.0f;
    PatrolWaitTime         = 3.0f;
    bIsPatrolling          = false;
    CurrentPatrolIndex     = 0;

    // Social defaults
    SocialRadius           = 800.0f;
    bIsSocializing         = false;

    // Combat/flee defaults
    FleeDistance           = 2500.0f;
    bIsFleeing             = false;

    // Sleep defaults
    bIsSleeping            = false;
    SleepStartHour         = 22.0f;
    SleepEndHour           = 6.0f;

    // Foraging defaults
    ForageRadius           = 1200.0f;
    bIsForaging            = false;

    // Timers
    StateEntryTime         = 0.0f;
    LastSocialTime         = 0.0f;
    LastForageTime         = 0.0f;
    RoutineUpdateInterval  = 5.0f;
    LastRoutineUpdateTime  = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner
    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogNPCBehavior, Warning, TEXT("NPCBehaviorComponent: No owner actor found"));
        return;
    }

    // Generate initial patrol points around spawn location
    GeneratePatrolPoints(OwnerActor->GetActorLocation(), PatrolRadius, 4);

    // Set initial state based on time of day
    float CurrentHour = GetCurrentHour();
    if (IsInSleepHours(CurrentHour))
    {
        TransitionToState(ENPC_BehaviorState::Sleep);
    }
    else
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }

    UE_LOG(LogNPCBehavior, Log, TEXT("NPCBehaviorComponent initialized on %s — state: %d"),
        *OwnerActor->GetName(), (int32)CurrentBehaviorState);
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerActor) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update memory — expire old entries
    UpdateMemory(CurrentTime);

    // Update threat detection
    UpdateThreatDetection();

    // Update daily routine at lower frequency
    if (CurrentTime - LastRoutineUpdateTime >= RoutineUpdateInterval)
    {
        UpdateDailyRoutine(CurrentTime);
        LastRoutineUpdateTime = CurrentTime;
    }

    // Update current state logic
    UpdateCurrentState(DeltaTime, CurrentTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentBehaviorState) return;

    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Reset state-specific flags
    bIsPatrolling   = (NewState == ENPC_BehaviorState::Patrol);
    bIsFleeing      = (NewState == ENPC_BehaviorState::Flee);
    bIsSocializing  = (NewState == ENPC_BehaviorState::Socialise);
    bIsForaging     = (NewState == ENPC_BehaviorState::Forage);
    bIsSleeping     = (NewState == ENPC_BehaviorState::Sleep);

    // Broadcast delegate
    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogNPCBehavior, Verbose, TEXT("%s: State %d → %d"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        (int32)OldState, (int32)NewState);
}

void UNPCBehaviorComponent::UpdateCurrentState(float DeltaTime, float CurrentTime)
{
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            UpdateIdleState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Forage:
            UpdateForageState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Alert:
            UpdateAlertState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Socialise:
            UpdateSocialiseState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Sleep:
            UpdateSleepState(DeltaTime, CurrentTime);
            break;
        case ENPC_BehaviorState::Dead:
            // No updates needed
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Individual State Updates
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateIdleState(float DeltaTime, float CurrentTime)
{
    // After 5-15 seconds in idle, transition to patrol or forage
    float TimeInState = CurrentTime - StateEntryTime;
    float IdleTimeout = 5.0f + (FMath::FRand() * 10.0f);

    if (TimeInState >= IdleTimeout)
    {
        // 60% chance to patrol, 40% chance to forage
        if (FMath::FRand() < 0.6f)
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            TransitionToState(ENPC_BehaviorState::Forage);
        }
    }
}

void UNPCBehaviorComponent::UpdatePatrolState(float DeltaTime, float CurrentTime)
{
    if (PatrolPoints.Num() == 0) return;

    // Check if we've reached the current patrol point
    FVector CurrentPos = OwnerActor->GetActorLocation();
    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
    float DistToTarget = FVector::Dist2D(CurrentPos, TargetPoint);

    if (DistToTarget < 150.0f)
    {
        // Reached patrol point — wait then move to next
        float TimeInState = CurrentTime - StateEntryTime;
        if (TimeInState >= PatrolWaitTime)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
            StateEntryTime = CurrentTime; // Reset timer for next point wait
        }
    }

    // After full patrol cycle, return to idle
    if (CurrentPatrolIndex == 0 && (CurrentTime - StateEntryTime) > PatrolWaitTime)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateForageState(float DeltaTime, float CurrentTime)
{
    // Forage for 20-40 seconds then return to idle
    float TimeInState = CurrentTime - StateEntryTime;
    float ForageTimeout = 20.0f + (FMath::FRand() * 20.0f);

    if (TimeInState >= ForageTimeout)
    {
        LastForageTime = CurrentTime;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateFleeState(float DeltaTime, float CurrentTime)
{
    // Check if threat is still present
    bool bThreatStillPresent = false;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatLevel >= ENPC_ThreatLevel::High)
        {
            float TimeSinceSeen = CurrentTime - Entry.LastSeenTimestamp;
            if (TimeSinceSeen < 10.0f) // Threat seen within last 10s
            {
                bThreatStillPresent = true;
                break;
            }
        }
    }

    if (!bThreatStillPresent)
    {
        // Threat gone — transition to alert (cautious)
        TransitionToState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::UpdateAlertState(float DeltaTime, float CurrentTime)
{
    // Stay alert for 15-30 seconds after threat disappears
    float TimeInState = CurrentTime - StateEntryTime;
    float AlertTimeout = 15.0f + (FMath::FRand() * 15.0f);

    if (TimeInState >= AlertTimeout)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::None;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateSocialiseState(float DeltaTime, float CurrentTime)
{
    // Socialise for 10-20 seconds
    float TimeInState = CurrentTime - StateEntryTime;
    float SocialTimeout = 10.0f + (FMath::FRand() * 10.0f);

    if (TimeInState >= SocialTimeout)
    {
        LastSocialTime = CurrentTime;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateSleepState(float DeltaTime, float CurrentTime)
{
    // Wake up if no longer in sleep hours
    float CurrentHour = GetCurrentHour();
    if (!IsInSleepHours(CurrentHour))
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Memory System
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AddMemoryEntry(AActor* ThreatActor, FVector LastKnownLocation, ENPC_ThreatLevel ThreatLevel)
{
    if (!ThreatActor) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Check if we already have this actor in memory — update if so
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.LastKnownLocation  = LastKnownLocation;
            Entry.LastSeenTimestamp  = CurrentTime;
            Entry.ThreatLevel        = ThreatLevel;
            Entry.bIsCurrentlyVisible = true;
            return;
        }
    }

    // New entry
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        // Remove oldest entry
        MemoryEntries.RemoveAt(0);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatActor          = ThreatActor;
    NewEntry.LastKnownLocation    = LastKnownLocation;
    NewEntry.LastSeenTimestamp    = CurrentTime;
    NewEntry.ThreatLevel          = ThreatLevel;
    NewEntry.bIsCurrentlyVisible  = true;
    MemoryEntries.Add(NewEntry);

    // Broadcast memory event
    OnThreatMemoryAdded.Broadcast(ThreatActor, ThreatLevel);

    UE_LOG(LogNPCBehavior, Verbose, TEXT("%s: Added memory for %s (ThreatLevel: %d)"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        *ThreatActor->GetName(), (int32)ThreatLevel);
}

void UNPCBehaviorComponent::UpdateMemory(float CurrentTime)
{
    // Mark actors as no longer visible if not recently updated
    // Expire entries older than MemoryRetentionTime
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; --i)
    {
        FNPC_MemoryEntry& Entry = MemoryEntries[i];
        float Age = CurrentTime - Entry.LastSeenTimestamp;

        if (Age > 2.0f)
        {
            Entry.bIsCurrentlyVisible = false;
        }

        if (Age > MemoryRetentionTime)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UNPCBehaviorComponent::ClearMemory()
{
    MemoryEntries.Empty();
    CurrentThreatLevel = ENPC_ThreatLevel::None;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetHighestThreatMemory() const
{
    FNPC_MemoryEntry HighestEntry;
    HighestEntry.ThreatLevel = ENPC_ThreatLevel::None;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatLevel > HighestEntry.ThreatLevel)
        {
            HighestEntry = Entry;
        }
    }
    return HighestEntry;
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat Detection
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateThreatDetection()
{
    if (!OwnerActor || !GetWorld()) return;

    FVector MyLocation = OwnerActor->GetActorLocation();

    // Get all pawns in detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    ENPC_ThreatLevel HighestThreat = ENPC_ThreatLevel::None;

    for (AActor* NearbyActor : NearbyActors)
    {
        if (NearbyActor == OwnerActor) continue;

        float Distance = FVector::Dist(MyLocation, NearbyActor->GetActorLocation());

        if (Distance <= ThreatDetectionRadius)
        {
            // Determine threat level based on distance and actor type
            ENPC_ThreatLevel ThreatLevel = EvaluateThreatLevel(NearbyActor, Distance);

            if (ThreatLevel > ENPC_ThreatLevel::None)
            {
                AddMemoryEntry(NearbyActor, NearbyActor->GetActorLocation(), ThreatLevel);

                if (ThreatLevel > HighestThreat)
                {
                    HighestThreat = ThreatLevel;
                }
            }
        }
    }

    // Update overall threat level and react
    if (HighestThreat != CurrentThreatLevel)
    {
        CurrentThreatLevel = HighestThreat;
        ReactToThreatLevel(HighestThreat);
    }
}

ENPC_ThreatLevel UNPCBehaviorComponent::EvaluateThreatLevel(AActor* ThreatActor, float Distance) const
{
    if (!ThreatActor) return ENPC_ThreatLevel::None;

    // Distance-based threat scaling
    // < 300 units = Critical
    // < 800 units = High
    // < 1500 units = Medium
    // < 2000 units = Low
    // > 2000 units = None

    if (Distance < 300.0f)  return ENPC_ThreatLevel::Critical;
    if (Distance < 800.0f)  return ENPC_ThreatLevel::High;
    if (Distance < 1500.0f) return ENPC_ThreatLevel::Medium;
    if (Distance < 2000.0f) return ENPC_ThreatLevel::Low;
    return ENPC_ThreatLevel::None;
}

void UNPCBehaviorComponent::ReactToThreatLevel(ENPC_ThreatLevel ThreatLevel)
{
    // Don't react if dead or sleeping (sleeping NPCs can be surprised though)
    if (CurrentBehaviorState == ENPC_BehaviorState::Dead) return;

    switch (ThreatLevel)
    {
        case ENPC_ThreatLevel::Critical:
        case ENPC_ThreatLevel::High:
            if (CurrentBehaviorState != ENPC_BehaviorState::Flee)
            {
                TransitionToState(ENPC_BehaviorState::Flee);
            }
            break;

        case ENPC_ThreatLevel::Medium:
            if (CurrentBehaviorState != ENPC_BehaviorState::Flee &&
                CurrentBehaviorState != ENPC_BehaviorState::Alert)
            {
                TransitionToState(ENPC_BehaviorState::Alert);
            }
            break;

        case ENPC_ThreatLevel::Low:
            if (CurrentBehaviorState == ENPC_BehaviorState::Idle ||
                CurrentBehaviorState == ENPC_BehaviorState::Forage ||
                CurrentBehaviorState == ENPC_BehaviorState::Socialise)
            {
                TransitionToState(ENPC_BehaviorState::Alert);
            }
            break;

        case ENPC_ThreatLevel::None:
            // Handled by UpdateAlertState timeout
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Daily Routine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateDailyRoutine(float CurrentTime)
{
    float CurrentHour = GetCurrentHour();

    // Check sleep schedule
    if (IsInSleepHours(CurrentHour))
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Sleep &&
            CurrentBehaviorState != ENPC_BehaviorState::Flee &&
            CurrentBehaviorState != ENPC_BehaviorState::Alert)
        {
            TransitionToState(ENPC_BehaviorState::Sleep);
        }
        return;
    }

    // Check if it's time to socialise (midday: 11-13h)
    if (CurrentHour >= 11.0f && CurrentHour <= 13.0f)
    {
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle &&
            (CurrentTime - LastSocialTime) > 300.0f) // 5 min cooldown
        {
            TransitionToState(ENPC_BehaviorState::Socialise);
        }
    }

    // Check if it's time to forage (morning: 7-10h, evening: 16-19h)
    bool bForageTime = (CurrentHour >= 7.0f && CurrentHour <= 10.0f) ||
                       (CurrentHour >= 16.0f && CurrentHour <= 19.0f);

    if (bForageTime && CurrentBehaviorState == ENPC_BehaviorState::Idle &&
        (CurrentTime - LastForageTime) > 600.0f) // 10 min cooldown
    {
        TransitionToState(ENPC_BehaviorState::Forage);
    }
}

float UNPCBehaviorComponent::GetCurrentHour() const
{
    // Default to midday if no world time system
    // In a full implementation this would query the day/night cycle system
    if (!GetWorld()) return 12.0f;

    // Use game time seconds as a proxy: 1 real second = 1 game minute
    // So 60 real seconds = 1 game hour, 1440 real seconds = 1 game day
    float GameSeconds = GetWorld()->GetTimeSeconds();
    float GameHour = FMath::Fmod(GameSeconds / 60.0f, 24.0f);
    return GameHour;
}

bool UNPCBehaviorComponent::IsInSleepHours(float Hour) const
{
    // Sleep from SleepStartHour (22:00) to SleepEndHour (6:00)
    if (SleepStartHour > SleepEndHour)
    {
        // Wraps midnight
        return Hour >= SleepStartHour || Hour < SleepEndHour;
    }
    return Hour >= SleepStartHour && Hour < SleepEndHour;
}

// ─────────────────────────────────────────────────────────────────────────────
// Patrol System
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::GeneratePatrolPoints(FVector Center, float Radius, int32 NumPoints)
{
    PatrolPoints.Empty();

    for (int32 i = 0; i < NumPoints; ++i)
    {
        float Angle = (360.0f / NumPoints) * i;
        float RadAngle = FMath::DegreesToRadians(Angle);

        // Vary radius slightly for natural-looking patrol
        float VaryRadius = Radius * (0.6f + FMath::FRand() * 0.4f);

        FVector Point = Center + FVector(
            FMath::Cos(RadAngle) * VaryRadius,
            FMath::Sin(RadAngle) * VaryRadius,
            0.0f
        );

        // Ground trace to place point on terrain
        FHitResult HitResult;
        FVector TraceStart = Point + FVector(0.0f, 0.0f, 500.0f);
        FVector TraceEnd   = Point - FVector(0.0f, 0.0f, 500.0f);

        if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            Point.Z = HitResult.ImpactPoint.Z;
        }

        PatrolPoints.Add(Point);
    }

    CurrentPatrolIndex = 0;
}

FVector UNPCBehaviorComponent::GetCurrentPatrolTarget() const
{
    if (PatrolPoints.Num() == 0) return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
    return PatrolPoints[CurrentPatrolIndex];
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::NotifyThreat(AActor* ThreatActor, FVector ThreatLocation, float ThreatIntensity)
{
    if (!ThreatActor) return;

    // Convert intensity (0-1) to threat level
    ENPC_ThreatLevel Level = ENPC_ThreatLevel::None;
    if (ThreatIntensity >= 0.9f)       Level = ENPC_ThreatLevel::Critical;
    else if (ThreatIntensity >= 0.7f)  Level = ENPC_ThreatLevel::High;
    else if (ThreatIntensity >= 0.5f)  Level = ENPC_ThreatLevel::Medium;
    else if (ThreatIntensity > 0.0f)   Level = ENPC_ThreatLevel::Low;

    AddMemoryEntry(ThreatActor, ThreatLocation, Level);
    ReactToThreatLevel(Level);
}

void UNPCBehaviorComponent::NotifyDeath()
{
    TransitionToState(ENPC_BehaviorState::Dead);
    PrimaryComponentTick.bCanEverTick = false;
    ClearMemory();
}

bool UNPCBehaviorComponent::IsAwareOfThreat() const
{
    return CurrentThreatLevel > ENPC_ThreatLevel::None;
}

bool UNPCBehaviorComponent::IsInDangerousState() const
{
    return CurrentBehaviorState == ENPC_BehaviorState::Flee ||
           CurrentBehaviorState == ENPC_BehaviorState::Alert;
}

FVector UNPCBehaviorComponent::GetFleeDirection() const
{
    if (!OwnerActor) return FVector::ForwardVector;

    // Flee away from highest threat
    FNPC_MemoryEntry HighestThreat = GetHighestThreatMemory();
    if (!HighestThreat.ThreatActor)
    {
        // No known threat — flee in a random direction
        float RandomAngle = FMath::FRand() * 360.0f;
        return FVector(FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
                       FMath::Sin(FMath::DegreesToRadians(RandomAngle)), 0.0f);
    }

    FVector ThreatLocation = HighestThreat.LastKnownLocation;
    FVector MyLocation     = OwnerActor->GetActorLocation();
    FVector FleeDir        = (MyLocation - ThreatLocation).GetSafeNormal2D();
    return FleeDir;
}
