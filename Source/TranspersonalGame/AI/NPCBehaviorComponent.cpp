// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260629_011
// Implements NPC daily routines, memory system, threat response, and pack behavior

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    PatrolRadius = 1500.0f;
    DetectionRadius = 800.0f;
    FleeRadius = 2000.0f;
    AttackRadius = 200.0f;
    MemoryDuration = 30.0f;
    MaxMemoryEntries = 16;
    bIsPackMember = false;
    PackLeader = nullptr;
    DailyRoutineIndex = 0;
    StateTimer = 0.0f;
    bDebugDraw = false;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location as spawn point
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        PatrolOrigin = HomeLocation;
    }

    // Start daily routine tick
    GetWorld()->GetTimerManager().SetTimer(
        RoutineTimerHandle,
        this,
        &UNPCBehaviorComponent::UpdateDailyRoutine,
        5.0f,
        true
    );

    // Memory cleanup timer
    GetWorld()->GetTimerManager().SetTimer(
        MemoryCleanupHandle,
        this,
        &UNPCBehaviorComponent::CleanExpiredMemories,
        10.0f,
        true
    );
}

void UNPCBehaviorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(RoutineTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(MemoryCleanupHandle);
    Super::EndPlay(EndPlayReason);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // Scan for threats every tick
    ScanForThreats();

    // Execute current behavior
    ExecuteCurrentBehavior(DeltaTime);

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugInfo();
    }
}

// ============================================================
// THREAT DETECTION
// ============================================================

void UNPCBehaviorComponent::ScanForThreats()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector OwnerLocation = Owner->GetActorLocation();

    // Get all actors in detection radius
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    World->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(DetectionRadius),
        QueryParams
    );

    ENPC_ThreatLevel NewThreatLevel = ENPC_ThreatLevel::None;
    AActor* HighestThreat = nullptr;
    float ClosestThreatDist = DetectionRadius;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor) continue;

        // Check if this actor is a known threat (tagged as Predator or Player)
        if (OtherActor->ActorHasTag(FName("Predator")) || OtherActor->ActorHasTag(FName("Player")))
        {
            float Dist = FVector::Dist(OwnerLocation, OtherActor->GetActorLocation());

            ENPC_ThreatLevel ThreatForActor = ENPC_ThreatLevel::Low;
            if (Dist < AttackRadius * 2.0f)
                ThreatForActor = ENPC_ThreatLevel::Critical;
            else if (Dist < DetectionRadius * 0.3f)
                ThreatForActor = ENPC_ThreatLevel::High;
            else if (Dist < DetectionRadius * 0.6f)
                ThreatForActor = ENPC_ThreatLevel::Medium;

            if ((int32)ThreatForActor > (int32)NewThreatLevel)
            {
                NewThreatLevel = ThreatForActor;
                HighestThreat = OtherActor;
                ClosestThreatDist = Dist;
            }

            // Record in memory
            RecordMemoryEvent(OtherActor->GetActorLocation(), ENPC_MemoryEventType::ThreatSeen, OtherActor);
        }
    }

    // Update threat level and transition state if needed
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        OnThreatLevelChanged(NewThreatLevel, HighestThreat);
    }
}

void UNPCBehaviorComponent::OnThreatLevelChanged(ENPC_ThreatLevel NewLevel, AActor* ThreatSource)
{
    switch (NewLevel)
    {
    case ENPC_ThreatLevel::None:
        if (CurrentBehaviorState == ENPC_BehaviorState::Flee)
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
        break;

    case ENPC_ThreatLevel::Low:
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle ||
            CurrentBehaviorState == ENPC_BehaviorState::Gather)
        {
            TransitionToState(ENPC_BehaviorState::Investigate);
        }
        break;

    case ENPC_ThreatLevel::Medium:
    case ENPC_ThreatLevel::High:
        TransitionToState(ENPC_BehaviorState::Flee);
        if (ThreatSource)
        {
            FleeTarget = HomeLocation + (HomeLocation - ThreatSource->GetActorLocation()).GetSafeNormal() * FleeRadius;
        }
        // Alert pack members
        AlertPackMembers(ThreatSource);
        break;

    case ENPC_ThreatLevel::Critical:
        TransitionToState(ENPC_BehaviorState::Flee);
        if (ThreatSource)
        {
            // Emergency flee — maximum distance from threat
            FVector AwayDir = (GetOwner()->GetActorLocation() - ThreatSource->GetActorLocation()).GetSafeNormal();
            FleeTarget = GetOwner()->GetActorLocation() + AwayDir * FleeRadius * 1.5f;
        }
        AlertPackMembers(ThreatSource);
        break;
    }
}

// ============================================================
// BEHAVIOR STATE MACHINE
// ============================================================

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateTimer = 0.0f;

    OnBehaviorStateChanged.Broadcast(OldState, NewState);
}

void UNPCBehaviorComponent::ExecuteCurrentBehavior(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
    case ENPC_BehaviorState::Idle:
        ExecuteIdle(DeltaTime);
        break;
    case ENPC_BehaviorState::Patrol:
        ExecutePatrol(DeltaTime);
        break;
    case ENPC_BehaviorState::Gather:
        ExecuteGather(DeltaTime);
        break;
    case ENPC_BehaviorState::Flee:
        ExecuteFlee(DeltaTime);
        break;
    case ENPC_BehaviorState::Seek:
        ExecuteSeek(DeltaTime);
        break;
    case ENPC_BehaviorState::Investigate:
        ExecuteInvestigate(DeltaTime);
        break;
    case ENPC_BehaviorState::Interact:
        ExecuteInteract(DeltaTime);
        break;
    case ENPC_BehaviorState::Dead:
        // No behavior when dead
        break;
    }
}

void UNPCBehaviorComponent::ExecuteIdle(float DeltaTime)
{
    // After idle for 5-15 seconds, transition to patrol
    if (StateTimer > FMath::RandRange(5.0f, 15.0f))
    {
        // Pick a random patrol point
        FVector RandomOffset = FMath::VRand() * PatrolRadius;
        RandomOffset.Z = 0.0f;
        CurrentPatrolTarget = PatrolOrigin + RandomOffset;
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::ExecutePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist2D(Owner->GetActorLocation(), CurrentPatrolTarget);

    if (DistToTarget < 150.0f)
    {
        // Reached patrol point — idle briefly then pick new target
        TransitionToState(ENPC_BehaviorState::Idle);
        return;
    }

    // Timeout — if stuck for too long, pick new target
    if (StateTimer > 20.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::ExecuteGather(float DeltaTime)
{
    // Gather resources near current location
    // After gathering period, return to patrol
    if (StateTimer > FMath::RandRange(8.0f, 20.0f))
    {
        RecordMemoryEvent(GetOwner()->GetActorLocation(), ENPC_MemoryEventType::ResourceFound, nullptr);
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::ExecuteFlee(float DeltaTime)
{
    // If threat is gone and we've been fleeing for a while, calm down
    if (CurrentThreatLevel == ENPC_ThreatLevel::None && StateTimer > 8.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }

    // Emergency flee timeout — after 30s always calm down
    if (StateTimer > 30.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::ExecuteSeek(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !SeekTarget) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), SeekTarget->GetActorLocation());
    if (DistToTarget < AttackRadius || StateTimer > 15.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::ExecuteInvestigate(float DeltaTime)
{
    // Investigate for 5-10 seconds then return to patrol
    if (StateTimer > FMath::RandRange(5.0f, 10.0f))
    {
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            TransitionToState(ENPC_BehaviorState::Flee);
        }
    }
}

void UNPCBehaviorComponent::ExecuteInteract(float DeltaTime)
{
    // Interaction completes after a set time
    if (StateTimer > 3.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ============================================================
// DAILY ROUTINE SYSTEM
// ============================================================

void UNPCBehaviorComponent::UpdateDailyRoutine()
{
    if (DailyRoutine.Num() == 0) return;
    if (CurrentBehaviorState == ENPC_BehaviorState::Flee ||
        CurrentBehaviorState == ENPC_BehaviorState::Dead) return;

    // Advance to next routine step
    DailyRoutineIndex = (DailyRoutineIndex + 1) % DailyRoutine.Num();
    const FNPC_DailyRoutineStep& Step = DailyRoutine[DailyRoutineIndex];

    // Move to routine location and execute routine behavior
    CurrentPatrolTarget = Step.Location;
    TransitionToState(Step.BehaviorAtLocation);
}

void UNPCBehaviorComponent::SetDailyRoutine(const TArray<FNPC_DailyRoutineStep>& NewRoutine)
{
    DailyRoutine = NewRoutine;
    DailyRoutineIndex = 0;
}

// ============================================================
// MEMORY SYSTEM
// ============================================================

void UNPCBehaviorComponent::RecordMemoryEvent(FVector Location, ENPC_MemoryEventType EventType, AActor* RelatedActor)
{
    FNPC_MemoryEntry NewEntry;
    NewEntry.Location = Location;
    NewEntry.EventType = EventType;
    NewEntry.RelatedActor = RelatedActor;
    NewEntry.Timestamp = GetWorld()->GetTimeSeconds();
    NewEntry.ImportanceScore = CalculateMemoryImportance(EventType);

    // Evict least important memory if at capacity
    if (MemoryLog.Num() >= MaxMemoryEntries)
    {
        int32 LeastImportantIdx = 0;
        float LowestScore = MemoryLog[0].ImportanceScore;
        for (int32 i = 1; i < MemoryLog.Num(); i++)
        {
            if (MemoryLog[i].ImportanceScore < LowestScore)
            {
                LowestScore = MemoryLog[i].ImportanceScore;
                LeastImportantIdx = i;
            }
        }
        MemoryLog.RemoveAt(LeastImportantIdx);
    }

    MemoryLog.Add(NewEntry);
}

float UNPCBehaviorComponent::CalculateMemoryImportance(ENPC_MemoryEventType EventType) const
{
    switch (EventType)
    {
    case ENPC_MemoryEventType::ThreatSeen:      return 10.0f;
    case ENPC_MemoryEventType::PackMemberDied:  return 9.0f;
    case ENPC_MemoryEventType::AttackReceived:  return 8.0f;
    case ENPC_MemoryEventType::ResourceFound:   return 5.0f;
    case ENPC_MemoryEventType::SoundHeard:      return 3.0f;
    case ENPC_MemoryEventType::PlayerSeen:      return 7.0f;
    default:                                    return 1.0f;
    }
}

void UNPCBehaviorComponent::CleanExpiredMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    MemoryLog.RemoveAll([&](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > MemoryDuration;
    });
}

bool UNPCBehaviorComponent::HasMemoryOf(ENPC_MemoryEventType EventType) const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.EventType == EventType && (CurrentTime - Entry.Timestamp) < MemoryDuration)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    float MostRecent = -1.0f;
    FVector LastLocation = FVector::ZeroVector;

    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.EventType == ENPC_MemoryEventType::ThreatSeen && Entry.Timestamp > MostRecent)
        {
            MostRecent = Entry.Timestamp;
            LastLocation = Entry.Location;
        }
    }
    return LastLocation;
}

// ============================================================
// PACK BEHAVIOR
// ============================================================

void UNPCBehaviorComponent::AlertPackMembers(AActor* ThreatSource)
{
    if (PackMembers.Num() == 0) return;

    for (TWeakObjectPtr<AActor> MemberPtr : PackMembers)
    {
        if (!MemberPtr.IsValid()) continue;
        AActor* Member = MemberPtr.Get();

        UNPCBehaviorComponent* MemberComp = Member->FindComponentByClass<UNPCBehaviorComponent>();
        if (MemberComp && MemberComp != this)
        {
            MemberComp->ReceivePackAlert(ThreatSource, GetOwner());
        }
    }
}

void UNPCBehaviorComponent::ReceivePackAlert(AActor* ThreatSource, AActor* AlertingMember)
{
    if (!ThreatSource) return;

    // Record the alert as a memory event
    RecordMemoryEvent(ThreatSource->GetActorLocation(), ENPC_MemoryEventType::ThreatSeen, ThreatSource);

    // Escalate threat level based on alert
    if ((int32)CurrentThreatLevel < (int32)ENPC_ThreatLevel::Medium)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Medium;
        OnThreatLevelChanged(ENPC_ThreatLevel::Medium, ThreatSource);
    }
}

void UNPCBehaviorComponent::JoinPack(AActor* Leader, const TArray<AActor*>& Members)
{
    PackLeader = Leader;
    bIsPackMember = true;
    PackMembers.Empty();
    for (AActor* Member : Members)
    {
        if (Member && Member != GetOwner())
        {
            PackMembers.Add(Member);
        }
    }
}

void UNPCBehaviorComponent::LeavePack()
{
    PackLeader = nullptr;
    bIsPackMember = false;
    PackMembers.Empty();
}

// ============================================================
// DEBUG
// ============================================================

void UNPCBehaviorComponent::DrawDebugInfo()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    FVector Loc = Owner->GetActorLocation();

    // Detection radius
    DrawDebugSphere(World, Loc, DetectionRadius, 16, FColor::Yellow, false, 0.15f);

    // Attack radius
    DrawDebugSphere(World, Loc, AttackRadius, 8, FColor::Red, false, 0.15f);

    // Patrol target
    if (CurrentBehaviorState == ENPC_BehaviorState::Patrol)
    {
        DrawDebugLine(World, Loc, CurrentPatrolTarget, FColor::Green, false, 0.15f, 0, 2.0f);
        DrawDebugSphere(World, CurrentPatrolTarget, 50.0f, 8, FColor::Green, false, 0.15f);
    }

    // Memory locations
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        FColor MemColor = Entry.EventType == ENPC_MemoryEventType::ThreatSeen ? FColor::Red : FColor::Blue;
        DrawDebugSphere(World, Entry.Location, 30.0f, 6, MemColor, false, 0.15f);
    }
}

// ============================================================
// BLUEPRINT CALLABLE UTILITIES
// ============================================================

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}

ENPC_ThreatLevel UNPCBehaviorComponent::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}

void UNPCBehaviorComponent::ForceState(ENPC_BehaviorState NewState)
{
    TransitionToState(NewState);
}

void UNPCBehaviorComponent::SetPatrolOrigin(FVector NewOrigin, float NewRadius)
{
    PatrolOrigin = NewOrigin;
    PatrolRadius = NewRadius;
}

int32 UNPCBehaviorComponent::GetMemoryCount() const
{
    return MemoryLog.Num();
}
