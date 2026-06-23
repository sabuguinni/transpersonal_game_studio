#include "CombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — performance friendly

    CurrentState = ECombat_AIState::Idle;
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    CurrentTarget = nullptr;
    LastAttackTime = -999.0f;
    StateEntryTime = 0.0f;
    FlankTargetLocation = FVector::ZeroVector;
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (CurrentState)
    {
    case ECombat_AIState::Idle:
        UpdateIdleState(DeltaTime);
        break;
    case ECombat_AIState::Patrolling:
        UpdatePatrolState(DeltaTime);
        break;
    case ECombat_AIState::Alerted:
        UpdateAlertedState(DeltaTime);
        break;
    case ECombat_AIState::Chasing:
        UpdateChasingState(DeltaTime);
        break;
    case ECombat_AIState::Attacking:
        UpdateAttackingState(DeltaTime);
        break;
    case ECombat_AIState::Flanking:
        UpdateFlankingState(DeltaTime);
        break;
    case ECombat_AIState::Retreating:
        UpdateRetreatingState(DeltaTime);
        break;
    case ECombat_AIState::Dead:
        // No tick logic when dead
        break;
    default:
        break;
    }

    // Periodic threat scan — detect player/threats in radius
    if (CurrentState != ECombat_AIState::Dead)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        AActor* Owner = GetOwner();
        if (!Owner) return;

        // Scan for player pawn
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            float Dist = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
            if (Dist <= Config.DetectionRadius)
            {
                if (CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrolling)
                {
                    OnThreatDetected(PlayerPawn, Dist);
                }
            }
            else if (CurrentTarget == PlayerPawn && Dist > Config.DetectionRadius * 1.5f)
            {
                OnThreatLost();
            }
        }
    }
}

void UCombatAIComponent::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: State -> %d"),
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
        (int32)NewState);
}

void UCombatAIComponent::OnThreatDetected(AActor* ThreatActor, float Distance)
{
    if (!ThreatActor) return;

    CurrentTarget = ThreatActor;
    CurrentThreatLevel = EvaluateThreat(Distance);

    // Record threat event
    FCombat_ThreatEvent Event;
    Event.ThreatActor = ThreatActor;
    Event.ThreatDistance = Distance;
    Event.ThreatLevel = CurrentThreatLevel;
    Event.ThreatLocation = ThreatActor->GetActorLocation();
    Event.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    ThreatHistory.Add(Event);

    // Cap threat history at 10 entries
    if (ThreatHistory.Num() > 10)
    {
        ThreatHistory.RemoveAt(0);
    }

    // Transition to alerted state
    SetCombatState(ECombat_AIState::Alerted);

    // Notify pack if pack hunter
    if (Config.bPackHunter)
    {
        NotifyPackMembers(ThreatActor);
    }

    UE_LOG(LogTemp, Warning, TEXT("CombatAI [%s]: THREAT DETECTED — %s at %.0f units"),
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
        *ThreatActor->GetActorLabel(),
        Distance);
}

void UCombatAIComponent::OnThreatLost()
{
    CurrentTarget = nullptr;
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    SetCombatState(ECombat_AIState::Patrolling);

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Threat lost — returning to patrol"), 
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"));
}

bool UCombatAIComponent::TryAttack(AActor* Target)
{
    if (!Target) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    float Now = World->GetTimeSeconds();
    if (Now - LastAttackTime < Config.AttackCooldown) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    if (Dist > Config.AttackRadius) return false;

    LastAttackTime = Now;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        Target,
        Config.AttackDamage,
        nullptr,
        Owner,
        nullptr
    );

    SetCombatState(ECombat_AIState::Attacking);

    UE_LOG(LogTemp, Warning, TEXT("CombatAI [%s]: ATTACK on %s — %.0f damage"),
        *Owner->GetActorLabel(),
        *Target->GetActorLabel(),
        Config.AttackDamage);

    return true;
}

void UCombatAIComponent::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target || !Config.bCanFlanking) return;

    FlankTargetLocation = CalculateFlankPosition(Target);
    SetCombatState(ECombat_AIState::Flanking);

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Flanking maneuver to (%.0f, %.0f, %.0f)"),
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
        FlankTargetLocation.X, FlankTargetLocation.Y, FlankTargetLocation.Z);
}

void UCombatAIComponent::NotifyPackMembers(AActor* ThreatActor)
{
    if (!ThreatActor) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find nearby actors of same class within pack radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, Owner->GetClass(), NearbyActors);

    FCombat_ThreatEvent PackEvent;
    PackEvent.ThreatActor = ThreatActor;
    PackEvent.ThreatLocation = ThreatActor->GetActorLocation();
    PackEvent.ThreatLevel = CurrentThreatLevel;
    PackEvent.Timestamp = World->GetTimeSeconds();

    int32 Notified = 0;
    for (AActor* PackMember : NearbyActors)
    {
        if (PackMember == Owner) continue;

        float Dist = FVector::Dist(Owner->GetActorLocation(), PackMember->GetActorLocation());
        if (Dist > Config.DetectionRadius * 2.0f) continue;

        UCombatAIComponent* MemberAI = PackMember->FindComponentByClass<UCombatAIComponent>();
        if (MemberAI)
        {
            MemberAI->ReceivePackAlert(PackEvent);
            Notified++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Pack alert sent to %d members"),
        *Owner->GetActorLabel(), Notified);
}

void UCombatAIComponent::ReceivePackAlert(const FCombat_ThreatEvent& ThreatEvent)
{
    if (!ThreatEvent.ThreatActor) return;

    if (CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrolling)
    {
        CurrentTarget = ThreatEvent.ThreatActor;
        CurrentThreatLevel = ThreatEvent.ThreatLevel;
        SetCombatState(ECombat_AIState::Alerted);

        UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Pack alert received — alerted to %s"),
            GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
            *ThreatEvent.ThreatActor->GetActorLabel());
    }
}

bool UCombatAIComponent::IsInCombat() const
{
    return CurrentState == ECombat_AIState::Chasing
        || CurrentState == ECombat_AIState::Attacking
        || CurrentState == ECombat_AIState::Flanking;
}

float UCombatAIComponent::GetThreatLevel() const
{
    switch (CurrentThreatLevel)
    {
    case ECombat_ThreatLevel::None:    return 0.0f;
    case ECombat_ThreatLevel::Low:     return 0.25f;
    case ECombat_ThreatLevel::Medium:  return 0.5f;
    case ECombat_ThreatLevel::High:    return 0.75f;
    case ECombat_ThreatLevel::Extreme: return 1.0f;
    default: return 0.0f;
    }
}

// --- Private State Updates ---

void UCombatAIComponent::UpdateIdleState(float DeltaTime)
{
    // Idle: stand still, scan for threats (handled in TickComponent)
    float TimeInState = GetWorld() ? GetWorld()->GetTimeSeconds() - StateEntryTime : 0.0f;
    if (TimeInState > 5.0f)
    {
        SetCombatState(ECombat_AIState::Patrolling);
    }
}

void UCombatAIComponent::UpdatePatrolState(float DeltaTime)
{
    // Patrol: move around home radius (movement handled by AIController/BT)
    // Here we just maintain state logic
}

void UCombatAIComponent::UpdateAlertedState(float DeltaTime)
{
    if (!CurrentTarget) 
    {
        SetCombatState(ECombat_AIState::Patrolling);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist <= Config.AttackRadius)
    {
        SetCombatState(ECombat_AIState::Attacking);
    }
    else if (Dist <= Config.FlankRadius && Config.bCanFlanking)
    {
        ExecuteFlankingManeuver(CurrentTarget);
    }
    else if (Dist <= Config.DetectionRadius)
    {
        SetCombatState(ECombat_AIState::Chasing);
    }
    else
    {
        OnThreatLost();
    }
}

void UCombatAIComponent::UpdateChasingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Patrolling);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist <= Config.AttackRadius)
    {
        TryAttack(CurrentTarget);
    }
    else if (Dist > Config.DetectionRadius * 1.5f)
    {
        OnThreatLost();
    }
}

void UCombatAIComponent::UpdateAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Idle);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist > Config.AttackRadius)
    {
        SetCombatState(ECombat_AIState::Chasing);
        return;
    }

    TryAttack(CurrentTarget);
}

void UCombatAIComponent::UpdateFlankingState(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToFlankPos = FVector::Dist(Owner->GetActorLocation(), FlankTargetLocation);

    if (DistToFlankPos < 100.0f)
    {
        // Reached flank position — attack
        SetCombatState(ECombat_AIState::Attacking);
    }
    else if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Patrolling);
    }
}

void UCombatAIComponent::UpdateRetreatingState(float DeltaTime)
{
    float TimeInState = GetWorld() ? GetWorld()->GetTimeSeconds() - StateEntryTime : 0.0f;
    if (TimeInState > 8.0f)
    {
        SetCombatState(ECombat_AIState::Idle);
    }
}

// --- Private Helpers ---

FVector UCombatAIComponent::CalculateFlankPosition(AActor* Target) const
{
    if (!Target || !GetOwner()) return FVector::ZeroVector;

    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();

    // Perpendicular flank direction (90 degrees to the right)
    FVector FlankDir = FVector(-ToTarget.Y, ToTarget.X, 0.0f).GetSafeNormal();

    // Flank position: 45 degrees off-axis, at attack radius distance
    FVector FlankOffset = (ToTarget + FlankDir).GetSafeNormal() * Config.FlankRadius * 0.5f;
    return TargetLoc + FlankOffset;
}

ECombat_ThreatLevel UCombatAIComponent::EvaluateThreat(float Distance) const
{
    float Ratio = Distance / Config.DetectionRadius;

    if (Ratio < 0.2f)  return ECombat_ThreatLevel::Extreme;
    if (Ratio < 0.4f)  return ECombat_ThreatLevel::High;
    if (Ratio < 0.6f)  return ECombat_ThreatLevel::Medium;
    if (Ratio < 0.8f)  return ECombat_ThreatLevel::Low;
    return ECombat_ThreatLevel::None;
}

bool UCombatAIComponent::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetOwner()) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50.0f);
    FVector End = Target->GetActorLocation() + FVector(0, 0, 50.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    // Can see if no blocking hit, or hit target directly
    return !bHit || Hit.GetActor() == Target;
}
