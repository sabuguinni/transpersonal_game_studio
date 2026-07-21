#include "AI/Combat/RaptorPackAI.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

URaptorPackAI::URaptorPackAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance
}

void URaptorPackAI::BeginPlay()
{
    Super::BeginPlay();
    PackState = ECombat_PackState::Idle;
    bFlankPositionsAssigned = false;
}

void URaptorPackAI::RegisterPackMember(AActor* RaptorActor, ECombat_RaptorRole Role)
{
    if (!RaptorActor) return;

    FCombat_RaptorMember Member;
    Member.RaptorActor = RaptorActor;
    Member.Role = Role;
    Member.Health = 100.0f;
    Member.bIsAlive = true;
    PackMembers.Add(Member);

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Registered member: %s as role %d"),
        *RaptorActor->GetName(), (int32)Role);
}

void URaptorPackAI::SetPackTarget(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    bFlankPositionsAssigned = false;
    TransitionTo(ECombat_PackState::Stalking);
    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Pack target set: %s"), *Target->GetName());
}

void URaptorPackAI::AssignFlankPositions()
{
    if (!CurrentTarget.IsValid()) return;

    const FVector TargetLoc = CurrentTarget->GetActorLocation();

    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (!Member.bIsAlive) continue;
        Member.AssignedFlankPosition = GetFlankPosition(Member.Role, TargetLoc);
    }

    bFlankPositionsAssigned = true;
    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Flank positions assigned for %d members"), PackMembers.Num());
}

FVector URaptorPackAI::GetFlankPosition(ECombat_RaptorRole Role, const FVector& TargetLocation) const
{
    const float Radius = PackConfig.FlankRadius;

    switch (Role)
    {
    case ECombat_RaptorRole::Alpha:
        // Alpha approaches from the front — draws attention
        return TargetLocation + FVector(Radius, 0.0f, 0.0f);

    case ECombat_RaptorRole::LeftFlanker:
        // Left flanker circles 120 degrees
        return TargetLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(120.0f)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(120.0f)) * Radius,
            0.0f
        );

    case ECombat_RaptorRole::RightFlanker:
        // Right flanker circles 240 degrees
        return TargetLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(240.0f)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(240.0f)) * Radius,
            0.0f
        );

    case ECombat_RaptorRole::Ambusher:
        // Ambusher approaches from behind
        return TargetLocation + FVector(-Radius, 0.0f, 0.0f);

    case ECombat_RaptorRole::Scout:
        // Scout stays at detection perimeter
        return TargetLocation + FVector(Radius * 1.5f, 0.0f, 0.0f);

    default:
        return TargetLocation + FVector(Radius, 0.0f, 0.0f);
    }
}

void URaptorPackAI::ExecutePackAttack()
{
    if (!CurrentTarget.IsValid()) return;

    const float Distance = GetDistanceToTarget();
    if (Distance > PackConfig.AttackRange) return;

    // Apply damage to target
    AActor* Target = CurrentTarget.Get();
    if (Target)
    {
        UGameplayStatics::ApplyDamage(
            Target,
            PackConfig.AttackDamage,
            nullptr,
            GetOwner(),
            nullptr
        );
        TimeSinceLastAttack = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Pack attack hit %s for %.1f damage"),
            *Target->GetName(), PackConfig.AttackDamage);
    }
}

void URaptorPackAI::OnPackMemberDied(AActor* DeadMember)
{
    if (!DeadMember) return;

    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.RaptorActor.Get() == DeadMember)
        {
            Member.bIsAlive = false;
            UE_LOG(LogTemp, Warning, TEXT("[RaptorPackAI] Pack member died: %s. Alive: %d"),
                *DeadMember->GetName(), GetAliveCount());
            break;
        }
    }

    if (ShouldRetreat())
    {
        TransitionTo(ECombat_PackState::Retreating);
    }
}

bool URaptorPackAI::ShouldRetreat() const
{
    const int32 Alive = GetAliveCount();
    const int32 Total = PackMembers.Num();
    if (Total == 0) return true;

    const float SurvivalRatio = (float)Alive / (float)Total;
    return SurvivalRatio <= PackConfig.RetreatHealthThreshold;
}

int32 URaptorPackAI::GetAliveCount() const
{
    int32 Count = 0;
    for (const FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.bIsAlive) Count++;
    }
    return Count;
}

void URaptorPackAI::TransitionTo(ECombat_PackState NewState)
{
    if (PackState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] State: %d -> %d"), (int32)PackState, (int32)NewState);
    PackState = NewState;

    if (NewState == ECombat_PackState::Flanking)
    {
        AssignFlankPositions();
    }
}

void URaptorPackAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TimeSinceLastAttack += DeltaTime;
    UpdatePackAI(DeltaTime);
}

void URaptorPackAI::UpdatePackAI(float DeltaTime)
{
    switch (PackState)
    {
    case ECombat_PackState::Idle:
        // Detection handled by Behavior Tree — this component responds to SetPackTarget()
        break;

    case ECombat_PackState::Stalking:
        UpdateStalking(DeltaTime);
        break;

    case ECombat_PackState::Flanking:
        UpdateFlanking(DeltaTime);
        break;

    case ECombat_PackState::Attacking:
        UpdateAttacking(DeltaTime);
        break;

    case ECombat_PackState::Retreating:
        // Behavior Tree handles retreat movement
        // Clear target after 5 seconds
        break;

    default:
        break;
    }
}

void URaptorPackAI::UpdateStalking(float DeltaTime)
{
    if (!CurrentTarget.IsValid()) return;

    const float Distance = GetDistanceToTarget();

    // Transition to flanking when close enough
    if (Distance < PackConfig.DetectionRadius * 0.5f)
    {
        TransitionTo(ECombat_PackState::Flanking);
    }
}

void URaptorPackAI::UpdateFlanking(float DeltaTime)
{
    if (!CurrentTarget.IsValid()) return;

    if (!bFlankPositionsAssigned)
    {
        AssignFlankPositions();
    }

    // Check if all alive members are near their flank positions
    int32 InPosition = 0;
    int32 AliveCount = GetAliveCount();

    for (const FCombat_RaptorMember& Member : PackMembers)
    {
        if (!Member.bIsAlive || !Member.RaptorActor.IsValid()) continue;

        const float DistToFlank = FVector::Dist(
            Member.RaptorActor->GetActorLocation(),
            Member.AssignedFlankPosition
        );

        if (DistToFlank < 200.0f) InPosition++;
    }

    // When majority are in position, attack
    if (AliveCount > 0 && InPosition >= FMath::CeilToInt(AliveCount * 0.66f))
    {
        TransitionTo(ECombat_PackState::Attacking);
    }
}

void URaptorPackAI::UpdateAttacking(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        TransitionTo(ECombat_PackState::Idle);
        return;
    }

    if (ShouldRetreat())
    {
        TransitionTo(ECombat_PackState::Retreating);
        return;
    }

    // Attack on cooldown
    if (TimeSinceLastAttack >= PackConfig.AttackCooldown)
    {
        if (IsTargetInRange(PackConfig.AttackRange))
        {
            ExecutePackAttack();
        }
    }
}

bool URaptorPackAI::IsTargetInRange(float Range) const
{
    return GetDistanceToTarget() <= Range;
}

float URaptorPackAI::GetDistanceToTarget() const
{
    if (!CurrentTarget.IsValid() || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}
