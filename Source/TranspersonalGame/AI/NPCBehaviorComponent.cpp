#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        PatrolTarget = HomeLocation;
    }

    SetBehaviorState(ENPC_BehaviorState::Idle);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // Decay old memories periodically
    if (FMath::Fmod(StateTimer, 10.f) < DeltaTime)
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
        ForgetOldMemories(CurrentTime);
    }

    // State machine tick
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:    TickIdle(DeltaTime);    break;
        case ENPC_BehaviorState::Patrol:  TickPatrol(DeltaTime);  break;
        case ENPC_BehaviorState::Forage:  TickForage(DeltaTime);  break;
        case ENPC_BehaviorState::Flee:    TickFlee(DeltaTime);    break;
        case ENPC_BehaviorState::Alert:   TickAlert(DeltaTime);   break;
        default: break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.f;

    // Adjust movement speed based on state
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
        {
            switch (NewState)
            {
                case ENPC_BehaviorState::Flee:
                    MoveComp->MaxWalkSpeed = Config.RunSpeed;
                    break;
                case ENPC_BehaviorState::Patrol:
                case ENPC_BehaviorState::Forage:
                    MoveComp->MaxWalkSpeed = Config.WalkSpeed;
                    break;
                case ENPC_BehaviorState::Alert:
                    MoveComp->MaxWalkSpeed = Config.WalkSpeed * 0.5f;
                    break;
                default:
                    MoveComp->MaxWalkSpeed = Config.WalkSpeed;
                    break;
            }
        }
    }
}

void UNPCBehaviorComponent::RecordThreat(AActor* ThreatActor, float ThreatLevel, bool bHostile)
{
    if (!ThreatActor) return;

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    // Check if we already have a record for this actor
    for (FNPC_MemoryRecord& Record : MemoryRecords)
    {
        if (Record.Subject == ThreatActor)
        {
            Record.LastKnownLocation = ThreatActor->GetActorLocation();
            Record.ThreatLevel = FMath::Max(Record.ThreatLevel, ThreatLevel);
            Record.TimeStamp = CurrentTime;
            Record.bIsHostile = bHostile;

            // React based on threat level and personality
            if (ThreatLevel > 0.7f)
            {
                if (Config.Personality == ENPC_PersonalityTrait::Timid ||
                    Config.Personality == ENPC_PersonalityTrait::Cautious)
                {
                    SetBehaviorState(ENPC_BehaviorState::Flee);
                }
                else
                {
                    SetBehaviorState(ENPC_BehaviorState::Alert);
                }
            }
            return;
        }
    }

    // New memory record
    FNPC_MemoryRecord NewRecord;
    NewRecord.Subject = ThreatActor;
    NewRecord.LastKnownLocation = ThreatActor->GetActorLocation();
    NewRecord.ThreatLevel = ThreatLevel;
    NewRecord.TimeStamp = CurrentTime;
    NewRecord.bIsHostile = bHostile;
    MemoryRecords.Add(NewRecord);

    // Immediate reaction to new threat
    if (ThreatLevel > 0.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ForgetOldMemories(float CurrentTime)
{
    MemoryRecords.RemoveAll([this, CurrentTime](const FNPC_MemoryRecord& Record)
    {
        return (CurrentTime - Record.TimeStamp) > Config.MemoryDecayTime;
    });
}

void UNPCBehaviorComponent::AddRoutineEntry(const FNPC_DailyRoutineEntry& Entry)
{
    DailyRoutine.Add(Entry);
    // Sort by time of day
    DailyRoutine.Sort([](const FNPC_DailyRoutineEntry& A, const FNPC_DailyRoutineEntry& B)
    {
        return A.TimeOfDayHour < B.TimeOfDayHour;
    });
}

void UNPCBehaviorComponent::EvaluateRoutine(float CurrentHour)
{
    if (DailyRoutine.Num() == 0) return;

    // Find the most recent routine entry for this time of day
    FNPC_DailyRoutineEntry* BestEntry = nullptr;
    for (FNPC_DailyRoutineEntry& Entry : DailyRoutine)
    {
        if (Entry.TimeOfDayHour <= CurrentHour)
        {
            BestEntry = &Entry;
        }
    }

    if (BestEntry && CurrentState != ENPC_BehaviorState::Flee &&
        CurrentState != ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(BestEntry->TargetState);
        PatrolTarget = BestEntry->TargetLocation;
    }
}

bool UNPCBehaviorComponent::IsAwareOfThreat() const
{
    return MemoryRecords.ContainsByPredicate([](const FNPC_MemoryRecord& R)
    {
        return R.ThreatLevel > 0.3f;
    });
}

float UNPCBehaviorComponent::GetHighestThreatLevel() const
{
    float MaxThreat = 0.f;
    for (const FNPC_MemoryRecord& Record : MemoryRecords)
    {
        MaxThreat = FMath::Max(MaxThreat, Record.ThreatLevel);
    }
    return MaxThreat;
}

// --- Private State Tick Functions ---

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5-10 seconds of idle, start patrolling
    float IdleTimeout = FMath::RandRange(5.f, 10.f);
    if (StateTimer > IdleTimeout)
    {
        ChooseNewPatrolTarget();
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), PatrolTarget);

    // Reached patrol target — wait then pick new one
    if (DistToTarget < 150.f)
    {
        if (StateTimer > PatrolWaitTime)
        {
            // 30% chance to forage instead of continuing patrol
            if (FMath::RandRange(0.f, 1.f) < 0.3f)
            {
                SetBehaviorState(ENPC_BehaviorState::Forage);
            }
            else
            {
                ChooseNewPatrolTarget();
                StateTimer = 0.f;
            }
        }
    }
}

void UNPCBehaviorComponent::TickForage(float DeltaTime)
{
    // Forage for 8-15 seconds then return to patrol
    float ForageDuration = FMath::RandRange(8.f, 15.f);
    if (StateTimer > ForageDuration)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // After fleeing for 8 seconds, check if threats are gone
    if (StateTimer > 8.f)
    {
        if (!IsAwareOfThreat())
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        else
        {
            // Keep fleeing — reset timer to re-evaluate in 4 more seconds
            StateTimer = 4.f;
        }
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Stay alert for 6 seconds, then decide
    if (StateTimer > 6.f)
    {
        float ThreatLevel = GetHighestThreatLevel();
        if (ThreatLevel > 0.7f)
        {
            SetBehaviorState(ENPC_BehaviorState::Flee);
        }
        else if (ThreatLevel > 0.3f)
        {
            StateTimer = 0.f; // Stay alert longer
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
}

void UNPCBehaviorComponent::ChooseNewPatrolTarget()
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-Config.PatrolRadius, Config.PatrolRadius),
        FMath::RandRange(-Config.PatrolRadius, Config.PatrolRadius),
        0.f
    );
    PatrolTarget = HomeLocation + RandomOffset;
    PatrolWaitTime = FMath::RandRange(2.f, 5.f);
}
