#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
    CurrentHealth = MaxHealth;
    CurrentHunger = 100.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    PatrolHomeLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    CurrentState = ENPC_BehaviorState::Patrol;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == ENPC_BehaviorState::Dead) return;

    StateTimer += DeltaTime;

    // Hunger decay
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDecayRate * DeltaTime);

    // Periodic threat scan (every 0.5s)
    if (FMath::Fmod(StateTimer, 0.5f) < DeltaTime)
    {
        ScanForThreats();
        CleanOldMemories();
    }

    // State machine dispatch
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:    TickIdle(DeltaTime);    break;
        case ENPC_BehaviorState::Patrol:  TickPatrol(DeltaTime);  break;
        case ENPC_BehaviorState::Alert:   TickAlert(DeltaTime);   break;
        case ENPC_BehaviorState::Chase:   TickChase(DeltaTime);   break;
        case ENPC_BehaviorState::Attack:  TickAttack(DeltaTime);  break;
        case ENPC_BehaviorState::Flee:    TickFlee(DeltaTime);    break;
        case ENPC_BehaviorState::Feed:    TickFeed(DeltaTime);    break;
        case ENPC_BehaviorState::Rest:    TickRest(DeltaTime);    break;
        default: break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState) return;
    PreviousState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;
    bIsAlerted = (NewState == ENPC_BehaviorState::Alert || NewState == ENPC_BehaviorState::Chase || NewState == ENPC_BehaviorState::Attack);
}

void UNPCBehaviorComponent::OnThreatDetected(AActor* ThreatActor, float ThreatDistance)
{
    if (!ThreatActor) return;
    CurrentThreatTarget = ThreatActor;
    RecordThreatMemory(ThreatActor->GetActorLocation(), 1.0f - (ThreatDistance / BehaviorConfig.DetectionRange), true);

    if (ThreatDistance <= BehaviorConfig.AttackRange)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else if (ThreatDistance <= BehaviorConfig.DetectionRange)
    {
        if (CurrentState != ENPC_BehaviorState::Chase && CurrentState != ENPC_BehaviorState::Attack)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }
    }

    // Pack alert for pack hunters
    if (BehaviorConfig.bIsPackHunter)
    {
        AlertPackMembers(ThreatActor->GetActorLocation());
    }
}

void UNPCBehaviorComponent::OnThreatLost()
{
    CurrentThreatTarget = nullptr;
    if (CurrentState == ENPC_BehaviorState::Chase || CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::OnTakeDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Dead);
        return;
    }

    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (CurrentState == ENPC_BehaviorState::Patrol || CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::RecordThreatMemory(FVector Location, float ThreatLevel, bool bIsPlayer)
{
    FNPC_MemoryEntry Entry;
    Entry.LastKnownLocation = Location;
    Entry.ThreatLevel = ThreatLevel;
    Entry.bIsPlayerThreat = bIsPlayer;
    Entry.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Keep max 10 memories
    if (ThreatMemories.Num() >= 10)
    {
        ThreatMemories.RemoveAt(0);
    }
    ThreatMemories.Add(Entry);
}

bool UNPCBehaviorComponent::HasRecentThreatMemory(float WithinSeconds) const
{
    if (!GetWorld()) return false;
    float Now = GetWorld()->GetTimeSeconds();
    for (const FNPC_MemoryEntry& Mem : ThreatMemories)
    {
        if ((Now - Mem.TimeStamp) <= WithinSeconds)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    if (ThreatMemories.Num() == 0) return FVector::ZeroVector;
    return ThreatMemories.Last().LastKnownLocation;
}

void UNPCBehaviorComponent::AlertPackMembers(FVector ThreatLocation)
{
    if (!GetOwner() || !GetWorld()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), NearbyActors);

    for (AActor* PackMember : NearbyActors)
    {
        if (PackMember == GetOwner()) continue;
        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PackMember->GetActorLocation());
        if (Dist <= BehaviorConfig.PackCoordinationRadius)
        {
            UNPCBehaviorComponent* MemberBehavior = PackMember->FindComponentByClass<UNPCBehaviorComponent>();
            if (MemberBehavior)
            {
                MemberBehavior->OnPackAlertReceived(ThreatLocation, GetOwner());
            }
        }
    }
}

void UNPCBehaviorComponent::OnPackAlertReceived(FVector ThreatLocation, AActor* AlertSource)
{
    RecordThreatMemory(ThreatLocation, 0.8f, true);
    if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

// --- Private State Tick Implementations ---

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 5s idle, return to patrol
    if (StateTimer > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    // If hungry, go feed
    if (CurrentHunger < 20.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Feed);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Patrol logic handled by AIController/BehaviorTree
    // This component provides state data to the BT
    if (CurrentHunger < 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Feed);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Transition to chase if threat confirmed
    if (CurrentThreatTarget)
    {
        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreatTarget->GetActorLocation());
        if (Dist <= BehaviorConfig.DetectionRange)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
    }
    // Return to patrol after 10s if no threat confirmed
    if (StateTimer > 10.0f && !CurrentThreatTarget)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickChase(float DeltaTime)
{
    if (!CurrentThreatTarget)
    {
        OnThreatLost();
        return;
    }
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreatTarget->GetActorLocation());
    if (Dist <= BehaviorConfig.AttackRange)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else if (Dist > BehaviorConfig.DetectionRange * 1.5f)
    {
        OnThreatLost();
    }
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
}

void UNPCBehaviorComponent::TickAttack(float DeltaTime)
{
    if (!CurrentThreatTarget)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreatTarget->GetActorLocation());
    if (Dist > BehaviorConfig.AttackRange * 1.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
    }
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    // Flee for 15s then reassess
    if (StateTimer > 15.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickFeed(float DeltaTime)
{
    // Regenerate hunger while feeding
    CurrentHunger = FMath::Min(100.0f, CurrentHunger + 10.0f * DeltaTime);
    if (CurrentHunger >= 80.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Rest);
    }
}

void UNPCBehaviorComponent::TickRest(float DeltaTime)
{
    // Regenerate health while resting
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + 5.0f * DeltaTime);
    if (StateTimer > 20.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::ScanForThreats()
{
    if (!GetOwner() || !GetWorld()) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= BehaviorConfig.DetectionRange)
    {
        OnThreatDetected(PlayerPawn, Dist);
    }
    else if (CurrentThreatTarget == PlayerPawn)
    {
        OnThreatLost();
    }
}

void UNPCBehaviorComponent::CleanOldMemories(float MaxAge)
{
    if (!GetWorld()) return;
    float Now = GetWorld()->GetTimeSeconds();
    ThreatMemories.RemoveAll([Now, MaxAge](const FNPC_MemoryEntry& Mem)
    {
        return (Now - Mem.TimeStamp) > MaxAge;
    });
}

bool UNPCBehaviorComponent::ShouldFlee() const
{
    return (CurrentHealth / MaxHealth) <= BehaviorConfig.FleeHealthThreshold;
}
