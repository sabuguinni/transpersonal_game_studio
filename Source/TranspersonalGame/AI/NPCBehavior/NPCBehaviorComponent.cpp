#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// UNPCBehaviorComponent — Implementation
// Agent #11 — NPC Behavior Agent
// Prehistoric survivor AI: daily routines, threat memory,
// alert escalation, survival stat decay.
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildDefaultSchedule();
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickThreatMemories(DeltaTime);
    TickSurvivalStats(DeltaTime);
    EvaluateAlertLevel();
}

// ---- Threat Registration ----

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatScore)
{
    if (!ThreatActor) return;

    // Update existing memory if we already know this threat
    for (FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.ThreatActor == ThreatActor)
        {
            Mem.LastKnownLocation = ThreatActor->GetActorLocation();
            Mem.ThreatScore = FMath::Max(Mem.ThreatScore, ThreatScore);
            Mem.TimeSinceSeen = 0.0f;
            Mem.bIsActive = true;
            return;
        }
    }

    // Add new memory — evict lowest-score entry if full
    if (ThreatMemories.Num() >= MaxThreatMemories)
    {
        int32 LowestIdx = 0;
        float LowestScore = ThreatMemories[0].ThreatScore;
        for (int32 i = 1; i < ThreatMemories.Num(); ++i)
        {
            if (ThreatMemories[i].ThreatScore < LowestScore)
            {
                LowestScore = ThreatMemories[i].ThreatScore;
                LowestIdx = i;
            }
        }
        ThreatMemories.RemoveAt(LowestIdx);
    }

    FNPC_ThreatMemory NewMem;
    NewMem.ThreatActor = ThreatActor;
    NewMem.LastKnownLocation = ThreatActor->GetActorLocation();
    NewMem.ThreatScore = ThreatScore;
    NewMem.TimeSinceSeen = 0.0f;
    NewMem.bIsActive = true;
    ThreatMemories.Add(NewMem);
}

void UNPCBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    if (!ThreatActor) return;
    ThreatMemories.RemoveAll([ThreatActor](const FNPC_ThreatMemory& Mem)
    {
        return Mem.ThreatActor == ThreatActor;
    });
}

bool UNPCBehaviorComponent::HasActiveThreats() const
{
    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.bIsActive && Mem.ThreatScore > 0.1f)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetHighestThreatLocation() const
{
    FVector BestLocation = FVector::ZeroVector;
    float BestScore = -1.0f;

    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.bIsActive && Mem.ThreatScore > BestScore)
        {
            BestScore = Mem.ThreatScore;
            BestLocation = Mem.LastKnownLocation;
        }
    }
    return BestLocation;
}

// ---- Daily Routine ----

void UNPCBehaviorComponent::AdvanceDailyPhase(ENPC_DailyPhase NewPhase)
{
    CurrentPhase = NewPhase;

    // Reset task index to find first task matching new phase
    for (int32 i = 0; i < DailySchedule.Num(); ++i)
    {
        if (DailySchedule[i].Phase == NewPhase && !DailySchedule[i].bCompleted)
        {
            CurrentTaskIndex = i;
            return;
        }
    }
    CurrentTaskIndex = 0;
}

FNPC_DailyTask UNPCBehaviorComponent::GetCurrentTask() const
{
    if (DailySchedule.IsValidIndex(CurrentTaskIndex))
    {
        return DailySchedule[CurrentTaskIndex];
    }
    return FNPC_DailyTask();
}

bool UNPCBehaviorComponent::AdvanceToNextTask()
{
    if (DailySchedule.IsValidIndex(CurrentTaskIndex))
    {
        DailySchedule[CurrentTaskIndex].bCompleted = true;
    }

    // Find next uncompleted task for current phase
    for (int32 i = CurrentTaskIndex + 1; i < DailySchedule.Num(); ++i)
    {
        if (DailySchedule[i].Phase == CurrentPhase && !DailySchedule[i].bCompleted)
        {
            CurrentTaskIndex = i;
            return true;
        }
    }

    // All tasks for this phase done
    return false;
}

// ---- Private Tick Methods ----

void UNPCBehaviorComponent::TickThreatMemories(float DeltaTime)
{
    for (FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        Mem.TimeSinceSeen += DeltaTime;

        // Update last known location if actor is still valid and visible
        if (IsValid(Mem.ThreatActor))
        {
            // Simple distance-based visibility — within detection radius = still tracking
            AActor* Owner = GetOwner();
            if (Owner)
            {
                float Dist = FVector::Dist(Owner->GetActorLocation(), Mem.ThreatActor->GetActorLocation());
                if (Dist <= ThreatDetectionRadius)
                {
                    Mem.LastKnownLocation = Mem.ThreatActor->GetActorLocation();
                    Mem.TimeSinceSeen = 0.0f;
                    Mem.bIsActive = true;
                }
                else
                {
                    // Threat out of range — memory fades
                    float FadeFraction = Mem.TimeSinceSeen / MemoryFadeTime;
                    Mem.ThreatScore = FMath::Lerp(Mem.ThreatScore, 0.0f, FadeFraction * DeltaTime);
                    if (Mem.ThreatScore < 0.05f)
                    {
                        Mem.bIsActive = false;
                    }
                }
            }
        }
        else
        {
            // Actor gone — fade memory
            Mem.bIsActive = false;
            Mem.ThreatScore = 0.0f;
        }
    }

    // Remove fully faded memories
    ThreatMemories.RemoveAll([](const FNPC_ThreatMemory& Mem)
    {
        return !Mem.bIsActive && Mem.ThreatScore <= 0.0f;
    });
}

void UNPCBehaviorComponent::TickSurvivalStats(float DeltaTime)
{
    Hunger = FMath::Clamp(Hunger - HungerDrainRate * DeltaTime, 0.0f, 1.0f);
    Thirst = FMath::Clamp(Thirst - ThirstDrainRate * DeltaTime, 0.0f, 1.0f);

    // Fatigue increases when hunger or thirst are critical
    if (Hunger < 0.2f || Thirst < 0.2f)
    {
        Fatigue = FMath::Clamp(Fatigue + 0.001f * DeltaTime, 0.0f, 1.0f);
    }
    else
    {
        // Slow fatigue recovery when needs are met
        Fatigue = FMath::Clamp(Fatigue - 0.0005f * DeltaTime, 0.0f, 1.0f);
    }
}

void UNPCBehaviorComponent::EvaluateAlertLevel()
{
    if (!HasActiveThreats())
    {
        // Calm down gradually
        if (AlertLevel > ENPC_AlertLevel::Calm)
        {
            int32 CurrentLevel = static_cast<int32>(AlertLevel);
            AlertLevel = static_cast<ENPC_AlertLevel>(FMath::Max(0, CurrentLevel - 1));
        }
        return;
    }

    // Find highest threat score
    float MaxScore = 0.0f;
    float ClosestDist = ThreatDetectionRadius;
    AActor* Owner = GetOwner();

    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (!Mem.bIsActive) continue;
        MaxScore = FMath::Max(MaxScore, Mem.ThreatScore);
        if (Owner)
        {
            float Dist = FVector::Dist(Owner->GetActorLocation(), Mem.LastKnownLocation);
            ClosestDist = FMath::Min(ClosestDist, Dist);
        }
    }

    // Escalate alert based on score and distance
    if (ClosestDist < 500.0f || MaxScore > 0.9f)
    {
        AlertLevel = ENPC_AlertLevel::Fleeing;
    }
    else if (ClosestDist < 1000.0f || MaxScore > 0.7f)
    {
        AlertLevel = ENPC_AlertLevel::Alarmed;
    }
    else if (ClosestDist < 1800.0f || MaxScore > 0.5f)
    {
        AlertLevel = ENPC_AlertLevel::Suspicious;
    }
    else if (MaxScore > 0.2f)
    {
        AlertLevel = ENPC_AlertLevel::Aware;
    }
}

void UNPCBehaviorComponent::BuildDefaultSchedule()
{
    DailySchedule.Empty();

    AActor* Owner = GetOwner();
    FVector Base = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

    // Dawn — wake, move to water source
    FNPC_DailyTask DawnTask;
    DawnTask.Phase = ENPC_DailyPhase::Dawn;
    DawnTask.TaskTag = FName("FetchWater");
    DawnTask.TargetLocation = Base + FVector(800.0f, 200.0f, 0.0f);
    DawnTask.Duration = 45.0f;
    DailySchedule.Add(DawnTask);

    // Morning — gather food near camp
    FNPC_DailyTask MorningTask;
    MorningTask.Phase = ENPC_DailyPhase::Morning;
    MorningTask.TaskTag = FName("GatherFood");
    MorningTask.TargetLocation = Base + FVector(1200.0f, -400.0f, 0.0f);
    MorningTask.Duration = 120.0f;
    DailySchedule.Add(MorningTask);

    // Midday — rest in shade
    FNPC_DailyTask MiddayTask;
    MiddayTask.Phase = ENPC_DailyPhase::Midday;
    MiddayTask.TaskTag = FName("Rest");
    MiddayTask.TargetLocation = Base + FVector(100.0f, 100.0f, 0.0f);
    MiddayTask.Duration = 90.0f;
    DailySchedule.Add(MiddayTask);

    // Afternoon — patrol perimeter (scouts/guards) or gather (others)
    FNPC_DailyTask AfternoonTask;
    AfternoonTask.Phase = ENPC_DailyPhase::Afternoon;
    AfternoonTask.TaskTag = (SocialRole == ENPC_SocialRole::Scout || SocialRole == ENPC_SocialRole::Guard)
        ? FName("Patrol") : FName("GatherFood");
    AfternoonTask.TargetLocation = Base + FVector(-600.0f, 900.0f, 0.0f);
    AfternoonTask.Duration = 100.0f;
    DailySchedule.Add(AfternoonTask);

    // Dusk — return to camp
    FNPC_DailyTask DuskTask;
    DuskTask.Phase = ENPC_DailyPhase::Dusk;
    DuskTask.TaskTag = FName("ReturnToCamp");
    DuskTask.TargetLocation = Base;
    DuskTask.Duration = 30.0f;
    DailySchedule.Add(DuskTask);

    // Night — sleep / guard rotation
    FNPC_DailyTask NightTask;
    NightTask.Phase = ENPC_DailyPhase::Night;
    NightTask.TaskTag = (SocialRole == ENPC_SocialRole::Guard) ? FName("NightGuard") : FName("Sleep");
    NightTask.TargetLocation = Base + FVector(50.0f, 50.0f, 0.0f);
    NightTask.Duration = 240.0f;
    DailySchedule.Add(NightTask);
}
