#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC AI
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerActor = GetOwner();

    if (!OwnerActor)
    {
        SetComponentTickEnabled(false);
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: No owner actor — tick disabled"));
        return;
    }

    // Seed patrol route if none provided
    if (PatrolRoute.Num() == 0 && OwnerActor)
    {
        FVector Origin = OwnerActor->GetActorLocation();
        const int32 NumPoints = 4;
        for (int32 i = 0; i < NumPoints; ++i)
        {
            float Angle = (360.0f / NumPoints) * i;
            float Rad = FMath::DegreesToRadians(Angle);
            FNPC_PatrolPoint Point;
            Point.WorldLocation = Origin + FVector(
                FMath::Cos(Rad) * PatrolRadius,
                FMath::Sin(Rad) * PatrolRadius,
                0.0f
            );
            Point.WaitDuration = FMath::RandRange(1.5f, 4.0f);
            Point.bLookAround = true;
            PatrolRoute.Add(Point);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent initialized on %s with %d patrol points"),
        *OwnerActor->GetName(), PatrolRoute.Num());
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerActor) return;

    WorldTime += DeltaTime;
    PhaseTimer += DeltaTime;

    UpdateSurvivalStats(DeltaTime);
    UpdateAlertLevel(DeltaTime);
    UpdateRoutineFromStats();
    EvaluateThreatFromMemory();

    // Decay old memories periodically
    if (FMath::Fmod(WorldTime, 10.0f) < DeltaTime)
    {
        ClearOldMemories();
    }

    if (bDrawDebug && OwnerActor->GetWorld())
    {
        DrawDebugSphere(OwnerActor->GetWorld(), OwnerActor->GetActorLocation(),
            DetectionRadius, 16, FColor::Yellow, false, 0.15f);
        DrawDebugSphere(OwnerActor->GetWorld(), OwnerActor->GetActorLocation(),
            FleeRadius, 16, FColor::Red, false, 0.15f);
    }
}

void UNPCBehaviorComponent::UpdateAlertLevel(float DeltaTime)
{
    // Decay alert over time
    if (CurrentAlertValue > 0.0f)
    {
        CurrentAlertValue = FMath::Max(0.0f, CurrentAlertValue - AlertDecayRate * DeltaTime);
    }

    // Map alert value to enum
    if (CurrentAlertValue <= 0.0f)
        AlertLevel = ENPC_AlertLevel::Unaware;
    else if (CurrentAlertValue < 20.0f)
        AlertLevel = ENPC_AlertLevel::Curious;
    else if (CurrentAlertValue < 40.0f)
        AlertLevel = ENPC_AlertLevel::Suspicious;
    else if (CurrentAlertValue < 60.0f)
        AlertLevel = ENPC_AlertLevel::Alert;
    else if (CurrentAlertValue < 80.0f)
        AlertLevel = ENPC_AlertLevel::Combat;
    else
        AlertLevel = ENPC_AlertLevel::Fleeing;
}

void UNPCBehaviorComponent::UpdateSurvivalStats(float DeltaTime)
{
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * DeltaTime);
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * DeltaTime);

    // Fear decays slowly when no threat
    if (AlertLevel == ENPC_AlertLevel::Unaware)
    {
        Fear = FMath::Max(0.0f, Fear - 5.0f * DeltaTime);
    }
}

void UNPCBehaviorComponent::UpdateRoutineFromStats()
{
    // Survival needs override routine
    if (CurrentPhase == ENPC_DailyRoutinePhase::Dead) return;
    if (CurrentPhase == ENPC_DailyRoutinePhase::Fleeing) return;
    if (CurrentPhase == ENPC_DailyRoutinePhase::Attacking) return;

    // High alert → flee or investigate
    if (AlertLevel == ENPC_AlertLevel::Fleeing || AlertLevel == ENPC_AlertLevel::Combat)
    {
        if (Fear > 60.0f)
            SetPhase(ENPC_DailyRoutinePhase::Fleeing);
        else
            SetPhase(ENPC_DailyRoutinePhase::Attacking);
        return;
    }

    if (AlertLevel == ENPC_AlertLevel::Alert || AlertLevel == ENPC_AlertLevel::Suspicious)
    {
        SetPhase(ENPC_DailyRoutinePhase::Investigating);
        return;
    }

    // Survival needs
    if (IsHungry() || IsThirsty())
    {
        SetPhase(ENPC_DailyRoutinePhase::Foraging);
        return;
    }

    // Default: patrol
    if (CurrentPhase != ENPC_DailyRoutinePhase::Patrolling &&
        CurrentPhase != ENPC_DailyRoutinePhase::Resting &&
        CurrentPhase != ENPC_DailyRoutinePhase::Sleeping)
    {
        SetPhase(ENPC_DailyRoutinePhase::Patrolling);
    }
}

void UNPCBehaviorComponent::EvaluateThreatFromMemory()
{
    if (MemoryLog.Num() == 0) return;

    // Find highest recent threat
    float MaxThreat = 0.0f;
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        float Age = WorldTime - Entry.TimeStamp;
        if (Age < MemoryDecayTime)
        {
            float DecayedThreat = Entry.ThreatLevel * (1.0f - Age / MemoryDecayTime);
            MaxThreat = FMath::Max(MaxThreat, DecayedThreat);
        }
    }

    // Inject into alert value if memory threat exceeds current
    if (MaxThreat > CurrentAlertValue)
    {
        CurrentAlertValue = FMath::Lerp(CurrentAlertValue, MaxThreat, 0.3f);
    }
}

void UNPCBehaviorComponent::ReceiveThreat(FVector ThreatLocation, float ThreatLevel, bool bIsPlayer)
{
    // Boost alert
    CurrentAlertValue = FMath::Min(100.0f, CurrentAlertValue + ThreatLevel);

    // Boost fear for player threats
    if (bIsPlayer)
    {
        Fear = FMath::Min(100.0f, Fear + ThreatLevel * 0.5f);
    }

    // Log to memory
    AddMemoryEntry(ThreatLocation, ThreatLevel, bIsPlayer,
        bIsPlayer ? TEXT("Player detected") : TEXT("Unknown threat"));

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent [%s]: Threat received — Level=%.1f, Alert=%.1f, Fear=%.1f"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        ThreatLevel, CurrentAlertValue, Fear);
}

void UNPCBehaviorComponent::SetPhase(ENPC_DailyRoutinePhase NewPhase)
{
    if (CurrentPhase == NewPhase) return;

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent [%s]: Phase %d → %d"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        (int32)CurrentPhase, (int32)NewPhase);

    CurrentPhase = NewPhase;
    PhaseTimer = 0.0f;
}

void UNPCBehaviorComponent::AddMemoryEntry(FVector Location, float ThreatLevel, bool bIsPlayer, const FString& Description)
{
    FNPC_MemoryEntry Entry;
    Entry.Location = Location;
    Entry.TimeStamp = WorldTime;
    Entry.ThreatLevel = ThreatLevel;
    Entry.bIsPlayerRelated = bIsPlayer;
    Entry.EventDescription = Description;

    MemoryLog.Add(Entry);

    // Trim oldest entries if over limit
    while (MemoryLog.Num() > MaxMemoryEntries)
    {
        MemoryLog.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ClearOldMemories()
{
    MemoryLog.RemoveAll([this](const FNPC_MemoryEntry& Entry)
    {
        return (WorldTime - Entry.TimeStamp) > MemoryDecayTime;
    });
}
