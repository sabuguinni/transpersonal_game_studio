#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentAlertLevel = ENPC_AlertLevel::Unaware;
    CurrentRoutinePhase = ENPC_RoutinePhase::Foraging;
    CurrentThreatAccumulator = 0.0f;
    CurrentPatrolIndex = 0;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayThreat(DeltaTime);
    DecayMemories(DeltaTime);
    UpdateAlertLevel();
    UpdateRoutineFromAlert();
}

// --- Alert System ---

void UNPCBehaviorComponent::ReceiveThreatStimulus(FVector StimulusLocation, float ThreatMagnitude)
{
    CurrentThreatAccumulator = FMath::Min(CurrentThreatAccumulator + ThreatMagnitude, 100.0f);
    RecordMemory(StimulusLocation, ThreatMagnitude);
    UpdateAlertLevel();
}

void UNPCBehaviorComponent::ClearAlert()
{
    CurrentThreatAccumulator = 0.0f;
    CurrentAlertLevel = ENPC_AlertLevel::Unaware;
}

void UNPCBehaviorComponent::UpdateAlertLevel()
{
    if (CurrentThreatAccumulator >= ThreatThresholdPanicked)
    {
        CurrentAlertLevel = ENPC_AlertLevel::Panicked;
    }
    else if (CurrentThreatAccumulator >= ThreatThresholdAlarmed)
    {
        CurrentAlertLevel = ENPC_AlertLevel::Alarmed;
    }
    else if (CurrentThreatAccumulator >= ThreatThresholdSuspicious)
    {
        CurrentAlertLevel = ENPC_AlertLevel::Suspicious;
    }
    else if (CurrentThreatAccumulator >= ThreatThresholdCurious)
    {
        CurrentAlertLevel = ENPC_AlertLevel::Curious;
    }
    else
    {
        CurrentAlertLevel = ENPC_AlertLevel::Unaware;
    }
}

void UNPCBehaviorComponent::DecayThreat(float DeltaTime)
{
    if (CurrentThreatAccumulator > 0.0f)
    {
        CurrentThreatAccumulator = FMath::Max(0.0f,
            CurrentThreatAccumulator - (AlertDecayRate * DeltaTime));
    }
}

void UNPCBehaviorComponent::UpdateRoutineFromAlert()
{
    // Alert overrides routine — NPC drops daily activity when threatened
    switch (CurrentAlertLevel)
    {
    case ENPC_AlertLevel::Unaware:
        // Restore normal routine — keep whatever was set externally
        break;
    case ENPC_AlertLevel::Curious:
        SetRoutinePhase(ENPC_RoutinePhase::Investigating);
        break;
    case ENPC_AlertLevel::Suspicious:
        SetRoutinePhase(ENPC_RoutinePhase::Investigating);
        break;
    case ENPC_AlertLevel::Alarmed:
        SetRoutinePhase(ENPC_RoutinePhase::Fleeing);
        break;
    case ENPC_AlertLevel::Panicked:
        SetRoutinePhase(ENPC_RoutinePhase::Fleeing);
        break;
    default:
        break;
    }
}

// --- Memory System ---

void UNPCBehaviorComponent::RecordMemory(FVector Location, float Threat)
{
    // Find existing inactive slot or oldest memory
    int32 SlotIndex = -1;
    float OldestTime = TNumericLimits<float>::Max();

    for (int32 i = 0; i < MemoryBank.Num(); ++i)
    {
        if (!MemoryBank[i].bIsActive)
        {
            SlotIndex = i;
            break;
        }
        if (MemoryBank[i].TimeStamp < OldestTime)
        {
            OldestTime = MemoryBank[i].TimeStamp;
            SlotIndex = i;
        }
    }

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    FNPC_MemoryEntry NewMemory;
    NewMemory.LastKnownLocation = Location;
    NewMemory.ThreatLevel = Threat;
    NewMemory.TimeStamp = CurrentTime;
    NewMemory.DecayRate = 30.0f;
    NewMemory.bIsActive = true;

    if (SlotIndex >= 0 && SlotIndex < MemoryBank.Num())
    {
        MemoryBank[SlotIndex] = NewMemory;
    }
    else if (MemoryBank.Num() < FMath::FloorToInt(MemoryCapacity))
    {
        MemoryBank.Add(NewMemory);
    }
}

void UNPCBehaviorComponent::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FNPC_MemoryEntry& Memory : MemoryBank)
    {
        if (Memory.bIsActive)
        {
            float Age = CurrentTime - Memory.TimeStamp;
            if (Age >= Memory.DecayRate)
            {
                Memory.bIsActive = false;
                Memory.ThreatLevel = 0.0f;
            }
        }
    }
}

bool UNPCBehaviorComponent::HasActiveMemory() const
{
    for (const FNPC_MemoryEntry& Memory : MemoryBank)
    {
        if (Memory.bIsActive) return true;
    }
    return false;
}

FNPC_MemoryEntry UNPCBehaviorComponent::GetMostRecentMemory() const
{
    FNPC_MemoryEntry Best;
    float BestTime = -1.0f;

    for (const FNPC_MemoryEntry& Memory : MemoryBank)
    {
        if (Memory.bIsActive && Memory.TimeStamp > BestTime)
        {
            BestTime = Memory.TimeStamp;
            Best = Memory;
        }
    }
    return Best;
}

// --- Routine System ---

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_RoutinePhase NewPhase)
{
    if (CurrentRoutinePhase != NewPhase)
    {
        CurrentRoutinePhase = NewPhase;
    }
}

// --- Patrol System ---

void UNPCBehaviorComponent::AddPatrolPoint(FVector Location, float WaitTime, bool bLook)
{
    FNPC_PatrolPoint Point;
    Point.Location = Location;
    Point.WaitDuration = WaitTime;
    Point.bLookAround = bLook;
    PatrolPoints.Add(Point);
}

FVector UNPCBehaviorComponent::GetNextPatrolDestination()
{
    if (PatrolPoints.Num() == 0)
    {
        // No waypoints — return a random point within patrol radius
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector Origin = Owner->GetActorLocation();
            float Angle = FMath::FRandRange(0.0f, 360.0f);
            float Dist = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
            return Origin + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
                0.0f
            );
        }
        return FVector::ZeroVector;
    }

    // Advance to next waypoint (loop)
    FVector Destination = PatrolPoints[CurrentPatrolIndex].Location;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return Destination;
}
