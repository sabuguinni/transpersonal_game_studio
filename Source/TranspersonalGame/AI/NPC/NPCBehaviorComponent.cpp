#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for NPC logic
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Idle;
    ThreatLevel  = ENPC_ThreatLevel::None;
    CurrentPhase = ENPC_DailyPhase::Morning;
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastAlert += DeltaTime;
    StateTimer         += DeltaTime;

    DecayMemories(DeltaTime);

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Patrol:  TickPatrol(DeltaTime); break;
    case ENPC_BehaviorState::Flee:    TickFlee(DeltaTime);   break;
    case ENPC_BehaviorState::Idle:    TickIdle(DeltaTime);   break;
    default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetBehaviorState
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer   = 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterThreat
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, float ThreatScore,
                                           const FString& ThreatTag)
{
    // Clamp memory log size
    if (MemoryLog.Num() >= MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }

    FNPC_MemoryEntry Entry;
    Entry.Location    = ThreatLocation;
    Entry.Timestamp   = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    Entry.ThreatScore = ThreatScore;
    Entry.Tag         = ThreatTag;
    MemoryLog.Add(Entry);

    // Update threat level based on score
    if (ThreatScore >= 0.9f)       ThreatLevel = ENPC_ThreatLevel::Critical;
    else if (ThreatScore >= 0.7f)  ThreatLevel = ENPC_ThreatLevel::High;
    else if (ThreatScore >= 0.4f)  ThreatLevel = ENPC_ThreatLevel::Medium;
    else if (ThreatScore > 0.f)    ThreatLevel = ENPC_ThreatLevel::Low;

    // Auto-transition to Flee on high threat
    if (ThreatLevel >= ENPC_ThreatLevel::High && TimeSinceLastAlert >= AlertCooldown)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        TimeSinceLastAlert = 0.f;
    }
    else if (ThreatLevel >= ENPC_ThreatLevel::Medium && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateDailyPhase
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateDailyPhase(float GameHour)
{
    // GameHour: 0–24
    if      (GameHour >= 5.f  && GameHour < 7.f)   CurrentPhase = ENPC_DailyPhase::Dawn;
    else if (GameHour >= 7.f  && GameHour < 11.f)  CurrentPhase = ENPC_DailyPhase::Morning;
    else if (GameHour >= 11.f && GameHour < 14.f)  CurrentPhase = ENPC_DailyPhase::Midday;
    else if (GameHour >= 14.f && GameHour < 18.f)  CurrentPhase = ENPC_DailyPhase::Afternoon;
    else if (GameHour >= 18.f && GameHour < 21.f)  CurrentPhase = ENPC_DailyPhase::Dusk;
    else                                             CurrentPhase = ENPC_DailyPhase::Night;

    // Adjust behavior based on phase
    if (CurrentPhase == ENPC_DailyPhase::Night && CurrentState == ENPC_BehaviorState::Patrol)
    {
        SetBehaviorState(ENPC_BehaviorState::Rest);
    }
    else if (CurrentPhase == ENPC_DailyPhase::Dawn && CurrentState == ENPC_BehaviorState::Rest)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// EvaluateNextState
// ─────────────────────────────────────────────────────────────────────────────

ENPC_BehaviorState UNPCBehaviorComponent::EvaluateNextState() const
{
    float HighestThreat = GetHighestThreatScore();

    if (HighestThreat >= 0.7f) return ENPC_BehaviorState::Flee;
    if (HighestThreat >= 0.4f) return ENPC_BehaviorState::Alert;

    if (CurrentPhase == ENPC_DailyPhase::Night) return ENPC_BehaviorState::Rest;
    if (CurrentPhase == ENPC_DailyPhase::Midday) return ENPC_BehaviorState::Forage;

    if (PatrolPoints.Num() > 0) return ENPC_BehaviorState::Patrol;

    return ENPC_BehaviorState::Idle;
}

// ─────────────────────────────────────────────────────────────────────────────
// ClearExpiredMemories
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::ClearExpiredMemories(float CurrentTime)
{
    const float ExpiryThreshold = 60.f; // memories older than 60s expire
    MemoryLog.RemoveAll([CurrentTime, ExpiryThreshold](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > ExpiryThreshold;
    });

    // Recalculate threat level after expiry
    float Highest = GetHighestThreatScore();
    if      (Highest >= 0.9f)  ThreatLevel = ENPC_ThreatLevel::Critical;
    else if (Highest >= 0.7f)  ThreatLevel = ENPC_ThreatLevel::High;
    else if (Highest >= 0.4f)  ThreatLevel = ENPC_ThreatLevel::Medium;
    else if (Highest > 0.f)    ThreatLevel = ENPC_ThreatLevel::Low;
    else                       ThreatLevel = ENPC_ThreatLevel::None;
}

// ─────────────────────────────────────────────────────────────────────────────
// HasActiveMemoryWithTag
// ─────────────────────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::HasActiveMemoryWithTag(const FString& Tag) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.Tag == Tag) return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetHighestThreatScore
// ─────────────────────────────────────────────────────────────────────────────

float UNPCBehaviorComponent::GetHighestThreatScore() const
{
    float Highest = 0.f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.ThreatScore > Highest) Highest = Entry.ThreatScore;
    }
    return Highest;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private tick helpers
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FNPC_PatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    float Dist = FVector::Dist(Owner->GetActorLocation(), Target.WorldLocation);

    if (Dist <= PatrolAcceptanceRadius)
    {
        // Reached waypoint — advance
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
    else
    {
        // Move toward waypoint via AIController
        AAIController* AIC = Cast<AAIController>(
            Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
        if (AIC)
        {
            AIC->MoveToLocation(Target.WorldLocation, PatrolAcceptanceRadius, true);
        }
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee logic: find direction away from highest-threat memory, move there
    if (MemoryLog.Num() == 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Find highest threat location
    FVector HighestThreatLoc = FVector::ZeroVector;
    float   HighestScore     = 0.f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.ThreatScore > HighestScore)
        {
            HighestScore     = Entry.ThreatScore;
            HighestThreatLoc = Entry.Location;
        }
    }

    // Direction away from threat
    FVector FleeDir = (Owner->GetActorLocation() - HighestThreatLoc).GetSafeNormal();
    FVector FleeTarget = Owner->GetActorLocation() + FleeDir * FleeThresholdDistance;

    AAIController* AIC = Cast<AAIController>(
        Cast<APawn>(Owner) ? Cast<APawn>(Owner)->GetController() : nullptr);
    if (AIC)
    {
        AIC->MoveToLocation(FleeTarget, 100.f, true);
    }
}

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5s idle, evaluate and potentially transition
    if (StateTimer >= 5.f)
    {
        ENPC_BehaviorState Next = EvaluateNextState();
        SetBehaviorState(Next);
    }
}

void UNPCBehaviorComponent::DecayMemories(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryLog)
    {
        Entry.ThreatScore = FMath::Max(0.f, Entry.ThreatScore - MemoryDecayRate * DeltaTime);
    }
}
