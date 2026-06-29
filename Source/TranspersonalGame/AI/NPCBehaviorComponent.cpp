// NPCBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent
// Implements the behavioral brain for all NPCs: daily routines, threat memory, state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f; // 4 Hz — enough for NPC logic

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    PatrolRadius = 2000.0f;
    DetectionRange = 1500.0f;
    FleeRange = 800.0f;
    AttackRange = 300.0f;
    MemoryDuration = 30.0f;
    bIsNocturnal = false;
    bCanFlee = true;
    bCanSocialise = true;
    HomeLocation = FVector::ZeroVector;
    bHomeLocationSet = false;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location at spawn
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
        bHomeLocationSet = true;
    }

    // Start daily routine tick
    GetWorld()->GetTimerManager().SetTimer(
        RoutineTimerHandle,
        this,
        &UNPCBehaviorComponent::TickDailyRoutine,
        5.0f,
        true
    );
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    PurgeExpiredMemories();
    UpdateThreatLevel();
    RunStateMachine(DeltaTime);
}

// ─── State Machine ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RunStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            State_Idle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            State_Patrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Forage:
            State_Forage(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            State_Flee(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            State_Alert(DeltaTime);
            break;
        case ENPC_BehaviorState::Socialise:
            State_Socialise(DeltaTime);
            break;
        case ENPC_BehaviorState::Sleep:
            State_Sleep(DeltaTime);
            break;
        case ENPC_BehaviorState::Dead:
            // No logic — dead NPCs don't tick
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::State_Idle(float DeltaTime)
{
    // After 3–8 seconds of idle, transition to patrol or forage
    IdleTimer += DeltaTime;
    float IdleThreshold = FMath::RandRange(3.0f, 8.0f);
    if (IdleTimer >= IdleThreshold)
    {
        IdleTimer = 0.0f;
        // Decide next activity based on threat level
        if (CurrentThreatLevel >= ENPC_ThreatLevel::Low)
        {
            TransitionToState(ENPC_BehaviorState::Alert);
        }
        else
        {
            bool bGoForage = FMath::RandBool();
            TransitionToState(bGoForage ? ENPC_BehaviorState::Forage : ENPC_BehaviorState::Patrol);
        }
    }
}

void UNPCBehaviorComponent::State_Patrol(float DeltaTime)
{
    // If threat detected, switch to Alert or Flee
    if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
    {
        TransitionToState(bCanFlee ? ENPC_BehaviorState::Flee : ENPC_BehaviorState::Alert);
        return;
    }
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Low)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
        return;
    }
    // Normal patrol — movement handled by BT/controller
}

void UNPCBehaviorComponent::State_Forage(float DeltaTime)
{
    // Foraging — look for food resources nearby
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium)
    {
        TransitionToState(bCanFlee ? ENPC_BehaviorState::Flee : ENPC_BehaviorState::Alert);
        return;
    }
    ForageTimer += DeltaTime;
    if (ForageTimer >= 15.0f)
    {
        ForageTimer = 0.0f;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::State_Flee(float DeltaTime)
{
    // Flee from highest-threat memory entry
    if (CurrentThreatLevel <= ENPC_ThreatLevel::None)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
        return;
    }
    // Flee logic: move away from last known threat position
    // Actual movement delegated to AIController/BT
}

void UNPCBehaviorComponent::State_Alert(float DeltaTime)
{
    AlertTimer += DeltaTime;
    if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
    {
        TransitionToState(bCanFlee ? ENPC_BehaviorState::Flee : ENPC_BehaviorState::Alert);
        return;
    }
    // After 10s of no escalation, return to patrol
    if (AlertTimer >= 10.0f)
    {
        AlertTimer = 0.0f;
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::State_Socialise(float DeltaTime)
{
    SocialTimer += DeltaTime;
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium)
    {
        TransitionToState(ENPC_BehaviorState::Alert);
        return;
    }
    if (SocialTimer >= 20.0f)
    {
        SocialTimer = 0.0f;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::State_Sleep(float DeltaTime)
{
    // Sleeping NPCs only wake on Critical threat
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Critical)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
    }
}

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState) return;

    ENPC_BehaviorState OldState = CurrentState;
    CurrentState = NewState;

    // Reset state timers
    IdleTimer = 0.0f;
    AlertTimer = 0.0f;
    ForageTimer = 0.0f;
    SocialTimer = 0.0f;

    OnStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Verbose, TEXT("[NPC %s] State: %d -> %d"),
        *GetOwner()->GetName(),
        (int32)OldState,
        (int32)NewState);
}

// ─── Memory System ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AddThreatMemory(AActor* ThreatActor, FVector LastKnownLocation, ENPC_ThreatLevel ThreatLevel)
{
    if (!ThreatActor) return;

    // Update existing entry if already known
    for (FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.LastKnownLocation = LastKnownLocation;
            Entry.ThreatLevel = ThreatLevel;
            Entry.Timestamp = GetWorld()->GetTimeSeconds();
            return;
        }
    }

    // Add new entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.LastKnownLocation = LastKnownLocation;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.Timestamp = GetWorld()->GetTimeSeconds();
    ThreatMemory.Add(NewEntry);
}

void UNPCBehaviorComponent::PurgeExpiredMemories()
{
    float Now = GetWorld()->GetTimeSeconds();
    ThreatMemory.RemoveAll([&](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.Timestamp) > MemoryDuration;
    });
}

void UNPCBehaviorComponent::UpdateThreatLevel()
{
    ENPC_ThreatLevel MaxThreat = ENPC_ThreatLevel::None;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatLevel > MaxThreat)
        {
            MaxThreat = Entry.ThreatLevel;
        }
    }
    CurrentThreatLevel = MaxThreat;
}

FVector UNPCBehaviorComponent::GetFleeDestination() const
{
    if (ThreatMemory.Num() == 0) return HomeLocation;

    // Find highest-threat entry
    const FNPC_MemoryEntry* HighestThreat = nullptr;
    for (const FNPC_MemoryEntry& Entry : ThreatMemory)
    {
        if (!HighestThreat || Entry.ThreatLevel > HighestThreat->ThreatLevel)
        {
            HighestThreat = &Entry;
        }
    }

    if (!HighestThreat || !GetOwner()) return HomeLocation;

    // Flee direction = away from threat
    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector ThreatLoc = HighestThreat->LastKnownLocation;
    FVector FleeDir = (OwnerLoc - ThreatLoc).GetSafeNormal();
    return OwnerLoc + FleeDir * FleeRange;
}

FVector UNPCBehaviorComponent::GetPatrolDestination() const
{
    if (!bHomeLocationSet) return FVector::ZeroVector;

    // Random point within patrol radius
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Dist = FMath::RandRange(200.0f, PatrolRadius);
    FVector Offset(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.0f
    );
    return HomeLocation + Offset;
}

// ─── Daily Routine ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickDailyRoutine()
{
    if (DailyRoutine.Num() == 0) return;

    // Find current routine slot by game hour
    // Simplified: cycle through slots every 5 real seconds
    CurrentRoutineIndex = (CurrentRoutineIndex + 1) % DailyRoutine.Num();
    const FNPC_DailyRoutineSlot& Slot = DailyRoutine[CurrentRoutineIndex];

    // Only override if not in danger
    if (CurrentThreatLevel <= ENPC_ThreatLevel::Low)
    {
        TransitionToState(Slot.TargetState);
        RoutineDestination = Slot.Destination;
    }
}

void UNPCBehaviorComponent::SetDailyRoutine(const TArray<FNPC_DailyRoutineSlot>& NewRoutine)
{
    DailyRoutine = NewRoutine;
    CurrentRoutineIndex = 0;
}

// ─── Utility ─────────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::IsActorInDetectionRange(AActor* Target) const
{
    if (!Target || !GetOwner()) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Dist <= DetectionRange;
}

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentState() const
{
    return CurrentState;
}

ENPC_ThreatLevel UNPCBehaviorComponent::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}
