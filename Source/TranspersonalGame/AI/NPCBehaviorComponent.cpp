#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for NPC AI
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;

    // Start patrol if patrol points are configured
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayThreat(DeltaTime);
    ForgetOldMemories(MemoryDuration);

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:   TickIdle(DeltaTime);   break;
        case ENPC_BehaviorState::Patrol: TickPatrol(DeltaTime); break;
        case ENPC_BehaviorState::Flee:   TickFlee(DeltaTime);   break;
        case ENPC_BehaviorState::Alert:  TickAlert(DeltaTime);  break;
        default: break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    // Adjust movement speed based on state
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (!OwnerChar) return;

    UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewState)
    {
        case ENPC_BehaviorState::Patrol:
        case ENPC_BehaviorState::Forage:
            MoveComp->MaxWalkSpeed = PatrolSpeed;
            break;
        case ENPC_BehaviorState::Flee:
        case ENPC_BehaviorState::Seek:
            MoveComp->MaxWalkSpeed = FleeSpeed;
            break;
        case ENPC_BehaviorState::Idle:
        case ENPC_BehaviorState::Rest:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = PatrolSpeed;
            break;
    }
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatScore, bool bIsPlayer)
{
    if (!ThreatActor) return;

    ActiveThreatActor = ThreatActor;
    ThreatDecayTimer = 0.0f;

    // Add to memory log
    FNPC_MemoryEntry Entry;
    Entry.LastKnownLocation = ThreatActor->GetActorLocation();
    Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Entry.ThreatScore = ThreatScore;
    Entry.bIsPlayer = bIsPlayer;
    MemoryLog.Add(Entry);

    // Cap memory log at 20 entries
    if (MemoryLog.Num() > 20)
    {
        MemoryLog.RemoveAt(0);
    }

    CurrentThreatLevel = ScoreToThreatLevel(ThreatScore);

    // Escalate behavior based on threat level
    if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (CurrentThreatLevel == ENPC_ThreatLevel::Low)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::ClearThreat()
{
    ActiveThreatActor = nullptr;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    ThreatDecayTimer = 0.0f;

    // Return to patrol or idle
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FNPC_PatrolPoint>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;

    if (PatrolPoints.Num() > 0 && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

bool UNPCBehaviorComponent::HasMemoryOfPlayer() const
{
    for (const FNPC_MemoryEntry& Entry : MemoryLog)
    {
        if (Entry.bIsPlayer) return true;
    }
    return false;
}

void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    if (!GetWorld()) return;
    float Now = GetWorld()->GetTimeSeconds();

    MemoryLog.RemoveAll([Now, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (Now - Entry.Timestamp) > MaxAge;
    });
}

void UNPCBehaviorComponent::UpdateDailyRoutine(float TimeOfDay)
{
    // TimeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    // Primitive humans: active dawn (0.2) to dusk (0.8), rest at night

    if (TimeOfDay < 0.2f || TimeOfDay > 0.8f)
    {
        // Night — rest
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Rest);
        }
    }
    else if (TimeOfDay >= 0.2f && TimeOfDay < 0.35f)
    {
        // Dawn — forage
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Forage);
        }
    }
    else if (TimeOfDay >= 0.35f && TimeOfDay < 0.65f)
    {
        // Midday — patrol/active
        if (CurrentThreatLevel == ENPC_ThreatLevel::None && PatrolPoints.Num() > 0)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
    else
    {
        // Late afternoon — return to camp (idle near origin)
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

// --- Private Tick Helpers ---

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // Scan for threats in detection radius
    AActor* Owner = GetOwner();
    if (!Owner || !GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer < DetectionRadius)
    {
        float Score = FMath::GetMappedRangeValueClamped(
            FVector2D(DetectionRadius, 0.0f),
            FVector2D(0.0f, 100.0f),
            DistToPlayer
        );
        RegisterThreat(PlayerPawn, Score, true);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || PatrolPoints.Num() == 0) return;

    // Check for threats while patrolling
    TickIdle(DeltaTime);
    if (CurrentState != ENPC_BehaviorState::Patrol) return;

    const FNPC_PatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    float DistToTarget = FVector::Dist2D(Owner->GetActorLocation(), Target.Location);

    if (DistToTarget < 150.0f)
    {
        // Reached waypoint — wait then advance
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer >= Target.WaitTime)
        {
            PatrolWaitTimer = 0.0f;
            AdvancePatrolPoint();
        }
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Update memory with latest threat location
    if (ActiveThreatActor)
    {
        if (MemoryLog.Num() > 0)
        {
            MemoryLog.Last().LastKnownLocation = ActiveThreatActor->GetActorLocation();
        }
    }

    // Check if threat is gone (out of flee radius)
    AActor* Owner = GetOwner();
    if (!Owner || !ActiveThreatActor) return;

    float DistToThreat = FVector::Dist(Owner->GetActorLocation(), ActiveThreatActor->GetActorLocation());
    if (DistToThreat > FleeRadius)
    {
        ClearThreat();
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: NPC has detected something but not fleeing yet
    // Scan more aggressively
    TickIdle(DeltaTime);

    // If no fresh threat after 5 seconds, return to patrol
    ThreatDecayTimer += DeltaTime;
    if (ThreatDecayTimer > 5.0f && CurrentThreatLevel <= ENPC_ThreatLevel::Low)
    {
        ClearThreat();
    }
}

void UNPCBehaviorComponent::DecayThreat(float DeltaTime)
{
    if (CurrentThreatLevel == ENPC_ThreatLevel::None) return;

    ThreatDecayTimer += DeltaTime;

    // Threat decays over 30 seconds if no new stimulus
    float DecayThreshold = 30.0f;
    if (ThreatDecayTimer > DecayThreshold)
    {
        uint8 ThreatInt = static_cast<uint8>(CurrentThreatLevel);
        if (ThreatInt > 0)
        {
            CurrentThreatLevel = static_cast<ENPC_ThreatLevel>(ThreatInt - 1);
            ThreatDecayTimer = 0.0f;
        }
        else
        {
            ClearThreat();
        }
    }
}

ENPC_ThreatLevel UNPCBehaviorComponent::ScoreToThreatLevel(float Score) const
{
    if (Score >= 80.0f) return ENPC_ThreatLevel::Critical;
    if (Score >= 60.0f) return ENPC_ThreatLevel::High;
    if (Score >= 40.0f) return ENPC_ThreatLevel::Medium;
    if (Score >= 20.0f) return ENPC_ThreatLevel::Low;
    return ENPC_ThreatLevel::None;
}
