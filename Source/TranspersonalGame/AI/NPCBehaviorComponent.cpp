// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_013
// Implements daily routines, threat response, memory, and state transitions
// for all non-player characters in the prehistoric survival world.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    // Default state
    CurrentState       = ENPC_BehaviorState::Idle;
    PreviousState      = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    CurrentTimeOfDay   = ENPC_TimeOfDay::Morning;

    // Movement defaults
    PatrolRadius       = 2000.0f;
    PatrolSpeed        = 150.0f;
    FleeSpeed          = 400.0f;
    ChaseSpeed         = 350.0f;
    InteractRange      = 200.0f;
    ThreatDetectRange  = 1500.0f;

    // Memory defaults
    MemoryDuration     = 30.0f;
    LastKnownThreatAge = 0.0f;

    // Flags
    bIsAlerted         = false;
    bHasPatrolRoute    = false;
    PatrolIndex        = 0;

    HomeLocation       = FVector::ZeroVector;
    LastKnownThreatLocation = FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record home location at spawn
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Start daily routine evaluation every 5 seconds
    GetWorld()->GetTimerManager().SetTimer(
        DailyRoutineTimer,
        this,
        &UNPCBehaviorComponent::EvaluateDailyRoutine,
        5.0f,
        true
    );

    // Start threat scan every 0.5 seconds
    GetWorld()->GetTimerManager().SetTimer(
        ThreatScanTimer,
        this,
        &UNPCBehaviorComponent::ScanForThreats,
        0.5f,
        true
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Age the threat memory
    if (CurrentThreatLevel != ENPC_ThreatLevel::None)
    {
        LastKnownThreatAge += DeltaTime;
        if (LastKnownThreatAge >= MemoryDuration)
        {
            ForgetThreat();
        }
    }

    // Execute current state behaviour
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        case ENPC_BehaviorState::Seek:
            TickSeek(DeltaTime);
            break;
        case ENPC_BehaviorState::Investigate:
            TickInvestigate(DeltaTime);
            break;
        case ENPC_BehaviorState::Gather:
            TickGather(DeltaTime);
            break;
        case ENPC_BehaviorState::Idle:
        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Transitions
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return; // Dead NPCs stay dead

    PreviousState = CurrentState;
    CurrentState  = NewState;

    OnStateChanged.Broadcast(PreviousState, CurrentState);

    UE_LOG(LogTemp, Verbose, TEXT("NPC [%s] state: %d -> %d"),
        *GetOwner()->GetName(),
        (int32)PreviousState,
        (int32)CurrentState);
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatDistance)
{
    if (!ThreatActor) return;

    LastKnownThreatLocation = ThreatActor->GetActorLocation();
    LastKnownThreatAge      = 0.0f;
    bIsAlerted              = true;

    // Determine threat level from distance
    ENPC_ThreatLevel NewThreat = ENPC_ThreatLevel::None;
    if      (ThreatDistance < 300.0f)  NewThreat = ENPC_ThreatLevel::Critical;
    else if (ThreatDistance < 600.0f)  NewThreat = ENPC_ThreatLevel::High;
    else if (ThreatDistance < 900.0f)  NewThreat = ENPC_ThreatLevel::Medium;
    else if (ThreatDistance < 1500.0f) NewThreat = ENPC_ThreatLevel::Low;

    if (NewThreat != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreat;
        OnThreatLevelChanged.Broadcast(CurrentThreatLevel);
    }

    // React based on threat level
    switch (CurrentThreatLevel)
    {
        case ENPC_ThreatLevel::Critical:
        case ENPC_ThreatLevel::High:
            SetBehaviorState(ENPC_BehaviorState::Flee);
            break;
        case ENPC_ThreatLevel::Medium:
            SetBehaviorState(ENPC_BehaviorState::Investigate);
            break;
        case ENPC_ThreatLevel::Low:
            if (CurrentState != ENPC_BehaviorState::Flee)
                SetBehaviorState(ENPC_BehaviorState::Investigate);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::ForgetThreat()
{
    CurrentThreatLevel  = ENPC_ThreatLevel::None;
    LastKnownThreatAge  = 0.0f;
    bIsAlerted          = false;

    // Return to previous routine if threat is forgotten
    if (CurrentState == ENPC_BehaviorState::Flee ||
        CurrentState == ENPC_BehaviorState::Investigate)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Daily Routine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::EvaluateDailyRoutine()
{
    // Do not interrupt high-priority states
    if (CurrentState == ENPC_BehaviorState::Flee  ||
        CurrentState == ENPC_BehaviorState::Dead   ||
        CurrentThreatLevel >= ENPC_ThreatLevel::High)
    {
        return;
    }

    switch (CurrentTimeOfDay)
    {
        case ENPC_TimeOfDay::Dawn:
            // Dawn: NPCs wake and move toward water/food sources
            SetBehaviorState(ENPC_BehaviorState::Seek);
            break;

        case ENPC_TimeOfDay::Morning:
            // Morning: active gathering and patrol
            SetBehaviorState(ENPC_BehaviorState::Gather);
            break;

        case ENPC_TimeOfDay::Midday:
            // Midday: rest near shelter, minimal movement
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;

        case ENPC_TimeOfDay::Afternoon:
            // Afternoon: patrol and interact with tribe members
            SetBehaviorState(ENPC_BehaviorState::Patrol);
            break;

        case ENPC_TimeOfDay::Night:
            // Night: return home, stay idle or on guard
            ReturnToHome();
            break;
    }
}

void UNPCBehaviorComponent::SetTimeOfDay(ENPC_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    EvaluateDailyRoutine();
}

void UNPCBehaviorComponent::ReturnToHome()
{
    if (AActor* Owner = GetOwner())
    {
        float DistToHome = FVector::Dist(Owner->GetActorLocation(), HomeLocation);
        if (DistToHome > 100.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Seek);
            // The actual movement toward HomeLocation is handled in TickSeek
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat Scanning
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::ScanForThreats()
{
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Sphere overlap for nearby actors
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    World->OverlapMultiByChannel(
        Overlaps,
        Owner->GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ThreatDetectRange),
        Params
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor) continue;

        // Check if it's the player pawn
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (OtherActor == PlayerPawn)
        {
            float Dist = FVector::Dist(Owner->GetActorLocation(), OtherActor->GetActorLocation());
            RegisterThreat(OtherActor, Dist);
            return; // Player is highest priority threat
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Tick Implementations
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;
    if (!bHasPatrolRoute) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Target = PatrolPoints[PatrolIndex];
    float Dist = FVector::Dist(Owner->GetActorLocation(), Target);

    if (Dist < 150.0f)
    {
        // Reached waypoint — advance
        PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
    }
    else
    {
        // Move toward waypoint
        FVector Dir = (Target - Owner->GetActorLocation()).GetSafeNormal();
        Owner->AddActorWorldOffset(Dir * PatrolSpeed * DeltaTime, true);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Move directly away from last known threat
    FVector FleeDir = (Owner->GetActorLocation() - LastKnownThreatLocation).GetSafeNormal();
    Owner->AddActorWorldOffset(FleeDir * FleeSpeed * DeltaTime, true);
}

void UNPCBehaviorComponent::TickSeek(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Seek toward home location
    FVector Target = HomeLocation;
    FVector Dir    = (Target - Owner->GetActorLocation()).GetSafeNormal();
    float   Dist   = FVector::Dist(Owner->GetActorLocation(), Target);

    if (Dist > 100.0f)
    {
        Owner->AddActorWorldOffset(Dir * ChaseSpeed * DeltaTime, true);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickInvestigate(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Move toward last known threat location cautiously
    FVector Dir  = (LastKnownThreatLocation - Owner->GetActorLocation()).GetSafeNormal();
    float   Dist = FVector::Dist(Owner->GetActorLocation(), LastKnownThreatLocation);

    if (Dist > 300.0f)
    {
        // Approach at half patrol speed
        Owner->AddActorWorldOffset(Dir * (PatrolSpeed * 0.5f) * DeltaTime, true);
    }
    else
    {
        // Reached investigation point — if no active threat, return to routine
        if (CurrentThreatLevel <= ENPC_ThreatLevel::Low)
        {
            ForgetThreat();
        }
    }
}

void UNPCBehaviorComponent::TickGather(float DeltaTime)
{
    // Gathering: NPC stays near home, performs idle animation variation
    // In a full implementation this would move to resource nodes
    // For now, small wander around home location
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), HomeLocation);
    if (Dist > PatrolRadius * 0.3f)
    {
        // Wander back toward home
        FVector Dir = (HomeLocation - Owner->GetActorLocation()).GetSafeNormal();
        Owner->AddActorWorldOffset(Dir * PatrolSpeed * DeltaTime, true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public Utility
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints    = Points;
    bHasPatrolRoute = (Points.Num() > 0);
    PatrolIndex     = 0;

    if (bHasPatrolRoute && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::KillNPC()
{
    SetBehaviorState(ENPC_BehaviorState::Dead);
    GetWorld()->GetTimerManager().ClearTimer(DailyRoutineTimer);
    GetWorld()->GetTimerManager().ClearTimer(ThreatScanTimer);
    PrimaryComponentTick.bCanEverTick = false;
}

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentState() const
{
    return CurrentState;
}

ENPC_ThreatLevel UNPCBehaviorComponent::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}
