#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentRoutine = ENPC_DailyRoutine::Forage;

    PatrolRadius = 1500.f;
    SightRange = 2000.f;
    HearingRange = 800.f;
    FleeThreshold = 0.6f;

    MaxMemoryEntries = 10;
    MemoryDecayTime = 120.f;

    StateTimer = 0.f;
    PatrolWaitTimer = 0.f;
    bWaitingAtPatrolPoint = false;
    CurrentPatrolIndex = 0;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Generate patrol points around spawn location
    GeneratePatrolPoints();

    // Start in patrol by default
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // Decay old memories periodically
    if (FMath::Fmod(StateTimer, 10.f) < DeltaTime)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            PurgeOldMemories(World->GetTimeSeconds());
        }
    }

    // Check for threats first
    float ThreatLevel = 0.f;
    bool bThreatNearby = IsThreatNearby(ThreatLevel);

    if (bThreatNearby && ThreatLevel >= FleeThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // Tick current state
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        TickIdle(DeltaTime);
        break;
    case ENPC_BehaviorState::Patrol:
        TickPatrol(DeltaTime);
        break;
    case ENPC_BehaviorState::Flee:
        TickFlee(DeltaTime);
        break;
    case ENPC_BehaviorState::Alert:
        TickAlert(DeltaTime);
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.f;

    UE_LOG(LogTemp, Log, TEXT("NPC [%s] state -> %d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
        (int32)NewState);
}

void UNPCBehaviorComponent::SetDailyRoutine(ENPC_DailyRoutine NewRoutine)
{
    CurrentRoutine = NewRoutine;
}

void UNPCBehaviorComponent::RecordMemoryEvent(FVector EventLocation, float ThreatLevel, const FString& EventTag)
{
    UWorld* World = GetWorld();
    if (!World) return;

    FNPC_MemoryEntry Entry;
    Entry.Location = EventLocation;
    Entry.Timestamp = World->GetTimeSeconds();
    Entry.ThreatLevel = ThreatLevel;
    Entry.EventTag = EventTag;

    MemoryLog.Add(Entry);

    // Trim if over capacity
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::PurgeOldMemories(float CurrentTime)
{
    MemoryLog.RemoveAll([this, CurrentTime](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > MemoryDecayTime;
    });
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex].WorldLocation;
}

bool UNPCBehaviorComponent::IsPlayerInSightRange() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return false;

    float Dist = FVector::Dist(Owner->GetActorLocation(), PlayerChar->GetActorLocation());
    return Dist <= SightRange;
}

bool UNPCBehaviorComponent::IsThreatNearby(float& OutThreatLevel) const
{
    OutThreatLevel = 0.f;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    // Check memory for recent high-threat events
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        float DistToEvent = FVector::Dist(Owner->GetActorLocation(), Entry.Location);
        if (DistToEvent < SightRange && Entry.ThreatLevel > OutThreatLevel)
        {
            OutThreatLevel = Entry.ThreatLevel;
        }
    }

    // Check player proximity as potential threat
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (PlayerChar)
    {
        float PlayerDist = FVector::Dist(Owner->GetActorLocation(), PlayerChar->GetActorLocation());
        if (PlayerDist < HearingRange)
        {
            // Player close = mild threat for skittish NPCs
            OutThreatLevel = FMath::Max(OutThreatLevel, 0.3f);
        }
    }

    return OutThreatLevel > 0.f;
}

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5 seconds idle, resume patrol
    if (StateTimer > 5.f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FNPC_PatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist2D(OwnerLoc, Target.WorldLocation);

    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer >= Target.WaitDuration)
        {
            bWaitingAtPatrolPoint = false;
            PatrolWaitTimer = 0.f;
            GetNextPatrolPoint();
        }
        return;
    }

    if (DistToTarget < 150.f)
    {
        // Arrived at patrol point
        bWaitingAtPatrolPoint = true;
        PatrolWaitTimer = 0.f;

        // Record patrol visit in memory
        RecordMemoryEvent(OwnerLoc, 0.f, TEXT("PatrolVisit"));
    }
    else
    {
        // Move toward patrol point - move the actor directly
        FVector Direction = (Target.WorldLocation - OwnerLoc).GetSafeNormal();
        float MoveSpeed = 200.f * DeltaTime;
        FVector NewLoc = OwnerLoc + Direction * MoveSpeed;
        Owner->SetActorLocation(NewLoc, true);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);

    // Compute flee direction (away from player)
    if (PlayerChar)
    {
        FVector AwayDir = (Owner->GetActorLocation() - PlayerChar->GetActorLocation()).GetSafeNormal();
        FleeDestination = Owner->GetActorLocation() + AwayDir * 2000.f;
    }

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToFlee = FVector::Dist2D(OwnerLoc, FleeDestination);

    if (DistToFlee > 100.f)
    {
        FVector Direction = (FleeDestination - OwnerLoc).GetSafeNormal();
        float FleeSpeed = 450.f * DeltaTime;
        FVector NewLoc = OwnerLoc + Direction * FleeSpeed;
        Owner->SetActorLocation(NewLoc, true);
    }
    else
    {
        // Reached flee destination - go alert
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }

    // After 15 seconds fleeing, calm down
    if (StateTimer > 15.f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Stay alert for 8 seconds then resume patrol
    if (StateTimer > 8.f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::GeneratePatrolPoints()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Origin = Owner->GetActorLocation();
    PatrolPoints.Empty();

    // Generate 4 patrol points in a rough square around origin
    TArray<FVector> Offsets = {
        FVector(PatrolRadius, 0.f, 0.f),
        FVector(0.f, PatrolRadius, 0.f),
        FVector(-PatrolRadius, 0.f, 0.f),
        FVector(0.f, -PatrolRadius, 0.f)
    };

    for (const FVector& Offset : Offsets)
    {
        // Add some randomness
        FVector RandomOffset = FVector(
            FMath::RandRange(-300.f, 300.f),
            FMath::RandRange(-300.f, 300.f),
            0.f
        );

        FNPC_PatrolPoint Point;
        Point.WorldLocation = Origin + Offset + RandomOffset;
        Point.WaitDuration = FMath::RandRange(1.5f, 4.f);
        PatrolPoints.Add(Point);
    }
}
