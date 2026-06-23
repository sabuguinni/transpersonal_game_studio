// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of NPC daily routines, memory decay, social alerting, and survival needs

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick at 10Hz for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default patrol waypoints around spawn location if none set
    if (PatrolWaypoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector Origin = Owner->GetActorLocation();
            // Create a simple 4-point patrol square
            PatrolWaypoints.Add(Origin + FVector(PatrolRadius * 0.5f, 0.0f, 0.0f));
            PatrolWaypoints.Add(Origin + FVector(PatrolRadius * 0.5f, PatrolRadius * 0.5f, 0.0f));
            PatrolWaypoints.Add(Origin + FVector(0.0f, PatrolRadius * 0.5f, 0.0f));
            PatrolWaypoints.Add(Origin);
        }
    }

    // Set initial routine based on tribe role
    switch (TribeRole)
    {
        case ENPC_TribeRole::Scout:
        case ENPC_TribeRole::Guard:
            SetRoutineState(ENPC_RoutineState::Patrolling);
            break;
        case ENPC_TribeRole::Hunter:
            SetRoutineState(ENPC_RoutineState::Hunting);
            break;
        case ENPC_TribeRole::Gatherer:
        case ENPC_TribeRole::Child:
            SetRoutineState(ENPC_RoutineState::Gathering);
            break;
        case ENPC_TribeRole::Elder:
            SetRoutineState(ENPC_RoutineState::Socializing);
            break;
        default:
            SetRoutineState(ENPC_RoutineState::Idle);
            break;
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay memory threat levels over time
    for (FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > 0.0f)
        {
            Entry.ThreatLevel = FMath::Max(0.0f, Entry.ThreatLevel - MemoryDecayRate * DeltaTime);
            if (Entry.ThreatLevel <= 0.0f)
            {
                Entry.bIsActive = false;
            }
        }
    }

    // Tick survival needs at interval
    TimeSinceLastNeedsTick += DeltaTime;
    if (TimeSinceLastNeedsTick >= NeedsTickInterval)
    {
        TickNeeds(NeedsTickInterval);
        TimeSinceLastNeedsTick = 0.0f;
    }

    // Re-evaluate routine priority based on current state
    ENPC_RoutineState Priority = EvaluatePriorityRoutine();
    if (Priority != CurrentRoutine)
    {
        SetRoutineState(Priority);
    }
}

void UNPCBehaviorComponent::AddMemoryEntry(FVector Location, float ThreatLevel, const FString& Tag)
{
    // Check if we already have a memory for this tag at a nearby location
    for (FNPC_MemoryEntry& Existing : MemoryLog)
    {
        if (Existing.EventTag == Tag && FVector::Dist(Existing.EventLocation, Location) < 500.0f)
        {
            // Update existing memory with stronger threat if applicable
            Existing.ThreatLevel = FMath::Max(Existing.ThreatLevel, ThreatLevel);
            Existing.EventLocation = Location;
            Existing.EventTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            Existing.bIsActive = true;
            return;
        }
    }

    // Add new memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.EventLocation = Location;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.EventTag = Tag;
    NewEntry.EventTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEntry.bIsActive = true;

    MemoryLog.Add(NewEntry);

    // Prune if over limit
    if (MemoryLog.Num() > MaxMemoryEntries)
    {
        PruneOldMemories();
    }

    // Immediately update fear based on threat level
    Needs.Fear = FMath::Clamp(Needs.Fear + ThreatLevel * 0.5f, 0.0f, 100.0f);
}

float UNPCBehaviorComponent::GetHighestThreatInMemory() const
{
    float MaxThreat = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsActive)
        {
            MaxThreat = FMath::Max(MaxThreat, Entry.ThreatLevel);
        }
    }
    return MaxThreat;
}

void UNPCBehaviorComponent::PruneOldMemories()
{
    // Remove inactive entries first
    MemoryLog.RemoveAll([](const FNPC_MemoryEntry& E) { return !E.bIsActive; });

    // If still over limit, remove lowest threat entries
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        int32 LowestIdx = 0;
        float LowestThreat = MemoryLog[0].ThreatLevel;
        for (int32 i = 1; i < MemoryLog.Num(); i++)
        {
            if (MemoryLog[i].ThreatLevel < LowestThreat)
            {
                LowestThreat = MemoryLog[i].ThreatLevel;
                LowestIdx = i;
            }
        }
        MemoryLog.RemoveAt(LowestIdx);
    }
}

void UNPCBehaviorComponent::SetRoutineState(ENPC_RoutineState NewState)
{
    if (CurrentRoutine != NewState)
    {
        CurrentRoutine = NewState;
    }
}

ENPC_RoutineState UNPCBehaviorComponent::EvaluatePriorityRoutine() const
{
    // Priority order: Fleeing > Attacking > Alerted > Investigating > Needs-driven > Role routine

    // Extreme fear = flee
    if (Needs.Fear >= FleeThreshold)
    {
        return ENPC_RoutineState::Fleeing;
    }

    // Moderate fear = alerted
    if (Needs.Fear >= AlertThreshold)
    {
        return ENPC_RoutineState::Alerted;
    }

    // Critical hunger = gather/hunt
    if (Needs.Hunger < 20.0f)
    {
        if (TribeRole == ENPC_TribeRole::Hunter)
            return ENPC_RoutineState::Hunting;
        return ENPC_RoutineState::Gathering;
    }

    // Critical energy = rest
    if (Needs.Energy < 15.0f)
    {
        return ENPC_RoutineState::Resting;
    }

    // Default: role-based routine
    switch (TribeRole)
    {
        case ENPC_TribeRole::Scout:
        case ENPC_TribeRole::Guard:
            return ENPC_RoutineState::Patrolling;
        case ENPC_TribeRole::Hunter:
            return ENPC_RoutineState::Hunting;
        case ENPC_TribeRole::Gatherer:
            return ENPC_RoutineState::Gathering;
        case ENPC_TribeRole::Elder:
            return ENPC_RoutineState::Socializing;
        default:
            return ENPC_RoutineState::Idle;
    }
}

void UNPCBehaviorComponent::TickNeeds(float DeltaTime)
{
    // Hunger decreases slowly over time
    Needs.Hunger = FMath::Max(0.0f, Needs.Hunger - 1.0f);

    // Thirst decreases faster
    Needs.Thirst = FMath::Max(0.0f, Needs.Thirst - 1.5f);

    // Fear naturally decays when no threats in memory
    float HighestThreat = GetHighestThreatInMemory();
    if (HighestThreat < AlertThreshold * 0.1f)
    {
        Needs.Fear = FMath::Max(0.0f, Needs.Fear - 5.0f);
    }

    // Energy recovers when resting, depletes when active
    if (CurrentRoutine == ENPC_RoutineState::Resting)
    {
        Needs.Energy = FMath::Min(100.0f, Needs.Energy + 10.0f);
    }
    else if (CurrentRoutine == ENPC_RoutineState::Fleeing || CurrentRoutine == ENPC_RoutineState::Hunting)
    {
        Needs.Energy = FMath::Max(0.0f, Needs.Energy - 3.0f);
    }
    else
    {
        Needs.Energy = FMath::Max(0.0f, Needs.Energy - 0.5f);
    }
}

void UNPCBehaviorComponent::AlertNearbyNPCs(FVector ThreatLocation, float ThreatLevel, const FString& ThreatTag)
{
    if (!bWillShareInformation) return;

    UWorld* World = GetWorld();
    if (!World) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Find all actors with NPCBehaviorComponent within share radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* OtherActor : AllActors)
    {
        if (OtherActor == Owner) continue;

        float Dist = FVector::Dist(Owner->GetActorLocation(), OtherActor->GetActorLocation());
        if (Dist > InformationShareRadius) continue;

        UNPCBehaviorComponent* OtherBehavior = OtherActor->FindComponentByClass<UNPCBehaviorComponent>();
        if (OtherBehavior)
        {
            // Share the threat memory — attenuated by distance
            float AttenuatedThreat = ThreatLevel * (1.0f - (Dist / InformationShareRadius) * 0.5f);
            OtherBehavior->AddMemoryEntry(ThreatLocation, AttenuatedThreat, ThreatTag);
        }
    }
}

FVector UNPCBehaviorComponent::GetNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
    }
    return PatrolWaypoints[CurrentWaypointIndex % PatrolWaypoints.Num()];
}

void UNPCBehaviorComponent::AdvanceWaypoint()
{
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    }
}
