#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for NPC AI

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;

    ThreatDetectionRadius = 2000.f;
    AlertBroadcastRadius = 3000.f;
    MemoryRetentionSeconds = 60.f;
    PatrolRadius = 1500.f;
    StateTimer = 0.f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        PatrolOrigin = Owner->GetActorLocation();
    }

    // Default daily routine: dawn patrol, midday rest, dusk social, night shelter
    FNPC_DailyRoutineSlot Dawn;
    Dawn.TimeOfDayHour = 6.f;
    Dawn.TargetState = ENPC_BehaviorState::Patrol;
    Dawn.ActivityDescription = TEXT("Morning patrol of territory");
    DailyRoutine.Add(Dawn);

    FNPC_DailyRoutineSlot Midday;
    Midday.TimeOfDayHour = 12.f;
    Midday.TargetState = ENPC_BehaviorState::Social;
    Midday.ActivityDescription = TEXT("Midday rest and social interaction");
    DailyRoutine.Add(Midday);

    FNPC_DailyRoutineSlot Dusk;
    Dusk.TimeOfDayHour = 18.f;
    Dusk.TargetState = ENPC_BehaviorState::Patrol;
    Dusk.ActivityDescription = TEXT("Dusk patrol before nightfall");
    DailyRoutine.Add(Dusk);

    FNPC_DailyRoutineSlot Night;
    Night.TimeOfDayHour = 21.f;
    Night.TargetState = ENPC_BehaviorState::Seek;
    Night.ActivityDescription = TEXT("Seek shelter for the night");
    DailyRoutine.Add(Night);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // Prune stale memories every 10 seconds
    if (FMath::Fmod(StateTimer, 10.f) < DeltaTime)
    {
        PruneOldMemories(MemoryRetentionSeconds);
        UpdateThreatFromMemory();
    }

    // State-specific tick
    switch (CurrentState)
    {
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

    // On entering FLEE state, immediately alert nearby NPCs
    if (NewState == ENPC_BehaviorState::Flee)
    {
        AlertNearbyNPCs(AlertBroadcastRadius);
    }
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level)
{
    if (!ThreatActor) return;

    FNPC_MemoryEntry Entry;
    Entry.ThreatLocation = ThreatActor->GetActorLocation();
    Entry.ThreatTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    Entry.ThreatLevel = Level;
    Entry.ThreatActorName = ThreatActor->GetName();

    MemoryLog.Add(Entry);

    // Escalate current threat level if new threat is higher
    if ((uint8)Level > (uint8)CurrentThreatLevel)
    {
        CurrentThreatLevel = Level;
    }

    // React to threat level
    if (Level >= ENPC_ThreatLevel::High)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (Level >= ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ClearThreatMemory()
{
    MemoryLog.Empty();
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void UNPCBehaviorComponent::PruneOldMemories(float MaxAgeSeconds)
{
    if (!GetWorld()) return;

    float Now = GetWorld()->GetTimeSeconds();
    MemoryLog.RemoveAll([Now, MaxAgeSeconds](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.ThreatTimestamp) > MaxAgeSeconds;
    });
}

void UNPCBehaviorComponent::AddRoutineSlot(FNPC_DailyRoutineSlot Slot)
{
    DailyRoutine.Add(Slot);
    // Sort by time of day
    DailyRoutine.Sort([](const FNPC_DailyRoutineSlot& A, const FNPC_DailyRoutineSlot& B)
    {
        return A.TimeOfDayHour < B.TimeOfDayHour;
    });
}

void UNPCBehaviorComponent::EvaluateDailyRoutine(float CurrentHour)
{
    // Only evaluate routine if not in threat state
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium) return;

    FNPC_DailyRoutineSlot* BestSlot = nullptr;
    float BestTime = -1.f;

    for (FNPC_DailyRoutineSlot& Slot : DailyRoutine)
    {
        if (Slot.TimeOfDayHour <= CurrentHour && Slot.TimeOfDayHour > BestTime)
        {
            BestTime = Slot.TimeOfDayHour;
            BestSlot = &Slot;
        }
    }

    if (BestSlot)
    {
        SetBehaviorState(BestSlot->TargetState);
        if (!BestSlot->TargetLocation.IsZero())
        {
            // Movement handled by AI Controller / Behavior Tree
        }
    }
}

FVector UNPCBehaviorComponent::GetNearestShelterLocation() const
{
    if (ShelterLocations.Num() == 0) return PatrolOrigin;

    AActor* Owner = GetOwner();
    if (!Owner) return ShelterLocations[0];

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector Nearest = ShelterLocations[0];
    float NearestDist = FVector::DistSquared(OwnerLoc, Nearest);

    for (const FVector& Shelter : ShelterLocations)
    {
        float Dist = FVector::DistSquared(OwnerLoc, Shelter);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            Nearest = Shelter;
        }
    }

    return Nearest;
}

void UNPCBehaviorComponent::AlertNearbyNPCs(float AlertRadius)
{
    if (!GetWorld() || !GetOwner()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > AlertRadius) continue;

        UNPCBehaviorComponent* OtherBehavior = Actor->FindComponentByClass<UNPCBehaviorComponent>();
        if (OtherBehavior && OtherBehavior->GetCurrentThreatLevel() < ENPC_ThreatLevel::High)
        {
            // Propagate alert — lower level than the source
            OtherBehavior->SetBehaviorState(ENPC_BehaviorState::Alert);
        }
    }
}

void UNPCBehaviorComponent::UpdateThreatFromMemory()
{
    if (MemoryLog.Num() == 0)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::None;
        return;
    }

    ENPC_ThreatLevel MaxLevel = ENPC_ThreatLevel::None;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if ((uint8)Entry.ThreatLevel > (uint8)MaxLevel)
        {
            MaxLevel = Entry.ThreatLevel;
        }
    }

    CurrentThreatLevel = MaxLevel;

    // De-escalate state if threat has faded
    if (MaxLevel == ENPC_ThreatLevel::None && CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Patrol logic is driven by Behavior Tree — this tick just validates origin
    if (GetOwner())
    {
        float DistFromOrigin = FVector::Dist(GetOwner()->GetActorLocation(), PatrolOrigin);
        if (DistFromOrigin > PatrolRadius * 1.5f)
        {
            // Too far from patrol origin — return
            // AI Controller handles actual movement
        }
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // After 30 seconds of fleeing, transition to Alert
    if (StateTimer > 30.f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // After 20 seconds of alert with no new threats, return to patrol
    if (StateTimer > 20.f && CurrentThreatLevel < ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}
