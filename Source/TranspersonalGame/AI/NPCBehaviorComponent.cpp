// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements full NPC behavioral AI: patrol, memory, threat response, daily routines

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
//  Constructor
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for NPC AI

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel   = ENPC_ThreatLevel::None;

    PatrolRadius         = 2000.0f;
    DetectionRadius      = 1500.0f;
    FleeRadius           = 800.0f;
    AttackRadius         = 250.0f;
    MemoryDuration       = 30.0f;
    MaxMemoryEntries     = 16;

    bCanFlee             = true;
    bCanAttack           = false; // Most NPCs are non-combatants
    bHasDailyRoutine     = true;
    bIsAlive             = true;

    CurrentHunger        = 50.0f;
    CurrentFear          = 0.0f;
    CurrentFatigue       = 0.0f;
    HomeLocation         = FVector::ZeroVector;
    bHomeLocationSet     = false;

    // Daily routine defaults (hours in 0-24 range)
    WakeHour             = 6.0f;
    SleepHour            = 20.0f;
    GatherHour           = 8.0f;
    RestHour             = 13.0f;
}

// ============================================================
//  BeginPlay
// ============================================================

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner && !bHomeLocationSet)
    {
        HomeLocation     = Owner->GetActorLocation();
        bHomeLocationSet = true;
    }

    // Stagger memory cleanup timer to avoid all NPCs ticking simultaneously
    float CleanupDelay = FMath::FRandRange(5.0f, 15.0f);
    GetWorld()->GetTimerManager().SetTimer(
        MemoryCleanupTimer,
        this,
        &UNPCBehaviorComponent::CleanupExpiredMemories,
        10.0f,
        true,
        CleanupDelay
    );

    // Stagger daily routine evaluation
    float RoutineDelay = FMath::FRandRange(0.5f, 3.0f);
    GetWorld()->GetTimerManager().SetTimer(
        DailyRoutineTimer,
        this,
        &UNPCBehaviorComponent::EvaluateDailyRoutine,
        5.0f,
        true,
        RoutineDelay
    );
}

// ============================================================
//  TickComponent
// ============================================================

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    // Decay fear over time (fear reduces 5 units/sec when no threat present)
    if (CurrentThreatLevel == ENPC_ThreatLevel::None && CurrentFear > 0.0f)
    {
        CurrentFear = FMath::Max(0.0f, CurrentFear - (5.0f * DeltaTime));
    }

    // Accumulate fatigue over time
    CurrentFatigue = FMath::Clamp(CurrentFatigue + (0.5f * DeltaTime), 0.0f, 100.0f);

    // Increase hunger over time
    CurrentHunger = FMath::Clamp(CurrentHunger + (0.2f * DeltaTime), 0.0f, 100.0f);

    // Evaluate threat from nearby predators
    EvaluateThreat();

    // Update behavior state machine
    UpdateBehaviorState();
}

// ============================================================
//  Threat Evaluation
// ============================================================

void UNPCBehaviorComponent::EvaluateThreat()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector OwnerLocation = Owner->GetActorLocation();

    // Scan for pawns within detection radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    World->OverlapMultiByChannel(Overlaps, OwnerLocation, FQuat::Identity,
        ECC_Pawn, Sphere, Params);

    float HighestThreat = 0.0f;
    AActor* ClosestThreat = nullptr;
    float ClosestDist = FLT_MAX;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor) continue;

        // Check if this actor is tagged as a predator
        if (!OtherActor->ActorHasTag(FName("Predator")) &&
            !OtherActor->ActorHasTag(FName("Dinosaur"))) continue;

        float Dist = FVector::Dist(OwnerLocation, OtherActor->GetActorLocation());
        float ThreatScore = 1.0f - FMath::Clamp(Dist / DetectionRadius, 0.0f, 1.0f);

        // Large predators (TRex tag) amplify threat
        if (OtherActor->ActorHasTag(FName("TRex")))
        {
            ThreatScore *= 2.5f;
        }
        else if (OtherActor->ActorHasTag(FName("Raptor")))
        {
            ThreatScore *= 1.5f;
        }

        if (ThreatScore > HighestThreat)
        {
            HighestThreat = ThreatScore;
        }

        if (Dist < ClosestDist)
        {
            ClosestDist  = Dist;
            ClosestThreat = OtherActor;
        }
    }

    // Map threat score to enum
    ENPC_ThreatLevel NewThreat = ENPC_ThreatLevel::None;
    if      (HighestThreat >= 1.8f) NewThreat = ENPC_ThreatLevel::Critical;
    else if (HighestThreat >= 1.0f) NewThreat = ENPC_ThreatLevel::High;
    else if (HighestThreat >= 0.5f) NewThreat = ENPC_ThreatLevel::Medium;
    else if (HighestThreat >  0.0f) NewThreat = ENPC_ThreatLevel::Low;

    // Raise fear when threat detected
    if (NewThreat != ENPC_ThreatLevel::None)
    {
        float FearIncrease = static_cast<float>(NewThreat) * 15.0f;
        CurrentFear = FMath::Min(100.0f, CurrentFear + FearIncrease);

        // Add memory entry for this threat
        if (ClosestThreat)
        {
            AddMemoryEntry(ClosestThreat, ClosestThreat->GetActorLocation(),
                ENPC_ThreatLevel::High, TEXT("Predator detected"));
        }
    }

    // Broadcast if threat level changed
    if (NewThreat != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreat;
        OnThreatLevelChanged.Broadcast(CurrentThreatLevel);
    }
}

// ============================================================
//  Behavior State Machine
// ============================================================

void UNPCBehaviorComponent::UpdateBehaviorState()
{
    if (!bIsAlive) return;

    ENPC_BehaviorState NewState = CurrentBehaviorState;

    // Priority order: Critical threat > High threat > Hunger > Fatigue > Daily routine
    switch (CurrentThreatLevel)
    {
        case ENPC_ThreatLevel::Critical:
        case ENPC_ThreatLevel::High:
            NewState = bCanFlee ? ENPC_BehaviorState::Flee : ENPC_BehaviorState::Idle;
            break;

        case ENPC_ThreatLevel::Medium:
            NewState = ENPC_BehaviorState::Flee;
            break;

        case ENPC_ThreatLevel::Low:
            NewState = ENPC_BehaviorState::Investigate;
            break;

        case ENPC_ThreatLevel::None:
        default:
            // No threat — evaluate survival needs and routine
            if (CurrentHunger >= 80.0f)
            {
                NewState = ENPC_BehaviorState::Gather;
            }
            else if (CurrentFatigue >= 80.0f)
            {
                NewState = ENPC_BehaviorState::Idle; // Rest
            }
            else
            {
                NewState = ENPC_BehaviorState::Patrol;
            }
            break;
    }

    if (NewState != CurrentBehaviorState)
    {
        SetBehaviorState(NewState);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    OnBehaviorStateChanged.Broadcast(OldState, NewState);
}

// ============================================================
//  Memory System
// ============================================================

void UNPCBehaviorComponent::AddMemoryEntry(AActor* SourceActor, FVector Location,
    ENPC_ThreatLevel Threat, const FString& Context)
{
    if (!SourceActor) return;

    // Check if we already have a memory for this actor — update it
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.SourceActor == SourceActor)
        {
            Entry.LastKnownLocation = Location;
            Entry.ThreatLevel       = Threat;
            Entry.ContextNote       = Context;
            Entry.Timestamp         = GetWorld()->GetTimeSeconds();
            Entry.bIsActive         = true;
            return;
        }
    }

    // New memory entry
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        // Remove oldest entry
        float OldestTime = FLT_MAX;
        int32 OldestIdx  = 0;
        for (int32 i = 0; i < MemoryEntries.Num(); ++i)
        {
            if (MemoryEntries[i].Timestamp < OldestTime)
            {
                OldestTime = MemoryEntries[i].Timestamp;
                OldestIdx  = i;
            }
        }
        MemoryEntries.RemoveAt(OldestIdx);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.SourceActor       = SourceActor;
    NewEntry.LastKnownLocation = Location;
    NewEntry.ThreatLevel       = Threat;
    NewEntry.ContextNote       = Context;
    NewEntry.Timestamp         = GetWorld()->GetTimeSeconds();
    NewEntry.bIsActive         = true;

    MemoryEntries.Add(NewEntry);
}

void UNPCBehaviorComponent::CleanupExpiredMemories()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    MemoryEntries.RemoveAll([&](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > MemoryDuration;
    });
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Actor) const
{
    if (!Actor) return false;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.SourceActor == Actor && Entry.bIsActive)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownLocationOf(AActor* Actor) const
{
    if (!Actor) return FVector::ZeroVector;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.SourceActor == Actor)
        {
            return Entry.LastKnownLocation;
        }
    }
    return FVector::ZeroVector;
}

// ============================================================
//  Daily Routine
// ============================================================

void UNPCBehaviorComponent::EvaluateDailyRoutine()
{
    if (!bHasDailyRoutine || !bIsAlive) return;
    if (CurrentThreatLevel != ENPC_ThreatLevel::None) return; // Don't override threat response

    // Get game time of day (0-24 hours)
    // In absence of a game time system, use real elapsed time modulo 24
    float GameHour = FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.0f, 24.0f);

    if (GameHour >= SleepHour || GameHour < WakeHour)
    {
        // Sleeping — stay idle at home
        if (CurrentBehaviorState != ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
    else if (GameHour >= GatherHour && GameHour < RestHour)
    {
        // Gathering food
        if (CurrentHunger > 30.0f && CurrentBehaviorState != ENPC_BehaviorState::Gather)
        {
            SetBehaviorState(ENPC_BehaviorState::Gather);
        }
    }
    else if (GameHour >= RestHour && GameHour < SleepHour)
    {
        // Afternoon patrol / social time
        if (CurrentBehaviorState != ENPC_BehaviorState::Patrol)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
}

// ============================================================
//  Public API
// ============================================================

void UNPCBehaviorComponent::SetFearLevel(float NewFear)
{
    CurrentFear = FMath::Clamp(NewFear, 0.0f, 100.0f);
}

void UNPCBehaviorComponent::SetHunger(float NewHunger)
{
    CurrentHunger = FMath::Clamp(NewHunger, 0.0f, 100.0f);
}

void UNPCBehaviorComponent::SetFatigue(float NewFatigue)
{
    CurrentFatigue = FMath::Clamp(NewFatigue, 0.0f, 100.0f);
}

void UNPCBehaviorComponent::NotifyDeath()
{
    bIsAlive = false;
    CurrentBehaviorState = ENPC_BehaviorState::Dead;
    CurrentThreatLevel   = ENPC_ThreatLevel::None;

    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(MemoryCleanupTimer);
    GetWorld()->GetTimerManager().ClearTimer(DailyRoutineTimer);

    OnBehaviorStateChanged.Broadcast(ENPC_BehaviorState::Patrol, ENPC_BehaviorState::Dead);
}

void UNPCBehaviorComponent::ForceState(ENPC_BehaviorState ForcedState)
{
    SetBehaviorState(ForcedState);
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    // Random point within patrol radius around home location
    float Angle  = FMath::FRandRange(0.0f, 360.0f);
    float Radius = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
    float X = HomeLocation.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = HomeLocation.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, HomeLocation.Z);
}

FVector UNPCBehaviorComponent::GetFleeDirection() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ForwardVector;

    FVector OwnerLoc = Owner->GetActorLocation();

    // Flee away from the most recent high-threat memory
    FVector ThreatLocation = FVector::ZeroVector;
    float   HighestThreat  = 0.0f;

    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        float ThreatScore = static_cast<float>(Entry.ThreatLevel);
        if (ThreatScore > HighestThreat)
        {
            HighestThreat  = ThreatScore;
            ThreatLocation = Entry.LastKnownLocation;
        }
    }

    if (HighestThreat > 0.0f)
    {
        FVector AwayDir = (OwnerLoc - ThreatLocation).GetSafeNormal();
        return AwayDir;
    }

    // No memory — flee in a random direction
    return FVector(FMath::FRandRange(-1.0f, 1.0f),
                   FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
}
