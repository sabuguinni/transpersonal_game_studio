#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default config values
    AlertRadius     = 2000.0f;
    FleeRadius      = 800.0f;
    DialogueRadius  = 300.0f;
    PatrolSpeed     = 200.0f;
    FleeSpeed       = 500.0f;

    CurrentState       = ENPC_BehaviorState::Idle;
    CurrentPatrolIndex = 0;
    StateTimer         = 0.0f;
    MemoryDecayRate    = 0.1f;
    FleeTarget         = FVector::ZeroVector;

    // Default dialogue lines for tribal NPCs
    FNPC_DialogueLine Line1;
    Line1.SpeakerName    = TEXT("Scout");
    Line1.LineText       = TEXT("Watch out! T-Rex tracks near the northern ridge.");
    Line1.DisplayDuration = 4.0f;
    DialogueLines.Add(Line1);

    FNPC_DialogueLine Line2;
    Line2.SpeakerName    = TEXT("Scout");
    Line2.LineText       = TEXT("Stay downwind. They hunt by smell.");
    Line2.DisplayDuration = 3.5f;
    DialogueLines.Add(Line2);

    FNPC_DialogueLine Line3;
    Line3.SpeakerName    = TEXT("Elder");
    Line3.LineText       = TEXT("The raptors move in packs. Never face them alone.");
    Line3.DisplayDuration = 4.5f;
    DialogueLines.Add(Line3);
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Start in idle, transition to patrol if patrol points exist
    if (PatrolPoints.Num() > 0)
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

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            UpdateAlertBehavior(DeltaTime);
            break;
        default:
            break;
    }

    // Decay old memories over time
    ClearOldMemories(60.0f);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }
    CurrentState = NewState;
    StateTimer   = 0.0f;
}

void UNPCBehaviorComponent::TriggerDialogue(int32 LineIndex)
{
    if (DialogueLines.IsValidIndex(LineIndex))
    {
        CurrentDialogueLine = DialogueLines[LineIndex];
    }
}

void UNPCBehaviorComponent::RecordMemory(const FString& EventTag, const FVector& Location, float ThreatLevel)
{
    FNPC_MemoryEntry Entry;
    Entry.EventTag      = EventTag;
    Entry.EventLocation = Location;
    Entry.ThreatLevel   = ThreatLevel;

    UWorld* World = GetWorld();
    Entry.TimeStamp = World ? World->GetTimeSeconds() : 0.0f;

    MemoryLog.Add(Entry);

    // Cap memory log at 20 entries
    if (MemoryLog.Num() > 20)
    {
        MemoryLog.RemoveAt(0);
    }
}

bool UNPCBehaviorComponent::HasMemoryOfThreat(float MinThreatLevel) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.ThreatLevel >= MinThreatLevel)
        {
            return true;
        }
    }
    return false;
}

void UNPCBehaviorComponent::ClearOldMemories(float MaxAge)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float CurrentTime = World->GetTimeSeconds();
    MemoryLog.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.TimeStamp) > MaxAge;
    });
}

void UNPCBehaviorComponent::AddPatrolPoint(const FVector& Point)
{
    PatrolPoints.Add(Point);
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return NextPoint;
}

void UNPCBehaviorComponent::OnDinosaurDetected(const FVector& DinoLocation, float ThreatLevel)
{
    RecordMemory(TEXT("DinosaurSighting"), DinoLocation, ThreatLevel);

    if (ThreatLevel >= 0.7f)
    {
        // High threat — flee immediately
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector OwnerLoc = Owner->GetActorLocation();
            FVector AwayDir  = (OwnerLoc - DinoLocation).GetSafeNormal();
            FleeTarget       = OwnerLoc + AwayDir * 3000.0f;
        }
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else
    {
        // Moderate threat — go alert
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::OnPlayerApproach(float Distance)
{
    if (Distance <= DialogueRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Interact);

        // Trigger a contextual dialogue line based on memory
        if (HasMemoryOfThreat(0.5f))
        {
            TriggerDialogue(0); // Warning line about T-Rex
        }
        else
        {
            TriggerDialogue(FMath::RandRange(0, DialogueLines.Num() - 1));
        }
    }
    else if (Distance <= AlertRadius && HasMemoryOfThreat(0.3f))
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::UpdateIdleBehavior(float DeltaTime)
{
    // After 5 seconds idle, transition to patrol if points available
    if (StateTimer > 5.0f && PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || PatrolPoints.Num() == 0)
    {
        return;
    }

    FVector CurrentLoc  = Owner->GetActorLocation();
    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
    float   Distance    = FVector::Dist(CurrentLoc, TargetPoint);

    if (Distance < 150.0f)
    {
        // Reached waypoint — advance to next
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
}

void UNPCBehaviorComponent::UpdateFleeBehavior(float DeltaTime)
{
    // After 10 seconds of fleeing, go alert
    if (StateTimer > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::UpdateAlertBehavior(float DeltaTime)
{
    // After 15 seconds of alert without new threat, return to patrol
    if (StateTimer > 15.0f && !HasMemoryOfThreat(0.5f))
    {
        if (PatrolPoints.Num() > 0)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}
