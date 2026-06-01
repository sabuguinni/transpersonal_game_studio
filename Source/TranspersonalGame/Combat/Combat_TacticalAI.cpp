#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI decisions
    
    CurrentState = ECombat_TacticalState::Patrol;
    CurrentPattern = ECombat_AttackPattern::Direct;
    PrimaryTarget = nullptr;
    LastTargetDistance = 0.0f;
    TimeSinceLastSighting = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;
    StateChangeTime = 0.0f;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
    
    // Default tactical data
    TacticalData.AggressionLevel = 0.5f;
    TacticalData.FearThreshold = 0.3f;
    TacticalData.AttackRange = 500.0f;
    TacticalData.FleeRange = 200.0f;
    TacticalData.PreferredPattern = ECombat_AttackPattern::Direct;
    TacticalData.bCanCoordinate = false;
    TacticalData.PackSize = 1;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    StateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Auto-detect pack members nearby
    if (TacticalData.bCanCoordinate && TacticalData.PackSize > 1)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != GetOwner())
            {
                UCombat_TacticalAI* OtherAI = Actor->FindComponentByClass<UCombat_TacticalAI>();
                if (OtherAI && PackMembers.Num() < TacticalData.PackSize - 1)
                {
                    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                    if (Distance < 2000.0f) // Pack formation range
                    {
                        RegisterPackMember(OtherAI);
                        OtherAI->RegisterPackMember(this);
                    }
                }
            }
        }
        
        // Become pack leader if no one else is
        if (PackMembers.Num() > 0)
        {
            bool HasLeader = false;
            for (UCombat_TacticalAI* Member : PackMembers)
            {
                if (Member->bIsPackLeader)
                {
                    HasLeader = true;
                    PackLeader = Member;
                    break;
                }
            }
            
            if (!HasLeader)
            {
                bIsPackLeader = true;
                for (UCombat_TacticalAI* Member : PackMembers)
                {
                    Member->PackLeader = this;
                }
            }
        }
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
        return;
    
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Execute current behavior
    ExecuteCombatBehavior();
    
    // Update timing
    TimeSinceLastSighting += DeltaTime;
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Broadcast state change to pack if leader
        if (bIsPackLeader)
        {
            BroadcastStateToPackMembers();
        }
    }
}

void UCombat_TacticalAI::UpdateThreatAssessment()
{
    if (!GetWorld())
        return;
    
    // Find nearest player as primary threat
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        
        // Check if player is in detection range
        if (DistanceToPlayer <= TacticalData.AttackRange * 2.0f)
        {
            if (CanSeeTarget())
            {
                SetPrimaryTarget(PlayerPawn);
                TimeSinceLastSighting = 0.0f;
                LastTargetDistance = DistanceToPlayer;
            }
        }
        else if (DistanceToPlayer > TacticalData.AttackRange * 3.0f)
        {
            // Too far, lose target
            if (PrimaryTarget == PlayerPawn)
            {
                PrimaryTarget = nullptr;
            }
        }
    }
    
    // Evaluate state transitions based on threat level
    if (PrimaryTarget)
    {
        float ThreatLevel = 1.0f - (LastTargetDistance / (TacticalData.AttackRange * 2.0f));
        ThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
        
        if (ThreatLevel > TacticalData.AggressionLevel && CurrentState == ECombat_TacticalState::Patrol)
        {
            SetTacticalState(ECombat_TacticalState::Alert);
        }
        else if (ThreatLevel > 0.8f && CurrentState == ECombat_TacticalState::Alert)
        {
            SetTacticalState(ECombat_TacticalState::Hunting);
        }
        else if (IsTargetInRange() && CurrentState == ECombat_TacticalState::Hunting)
        {
            SetTacticalState(ECombat_TacticalState::Attacking);
        }
        else if (ShouldRetreat())
        {
            SetTacticalState(ECombat_TacticalState::Retreating);
        }
    }
    else if (TimeSinceLastSighting > 5.0f && CurrentState != ECombat_TacticalState::Patrol)
    {
        // Lost target, return to patrol
        SetTacticalState(ECombat_TacticalState::Patrol);
    }
}

void UCombat_TacticalAI::SelectAttackPattern()
{
    if (!PrimaryTarget)
        return;
    
    // Choose attack pattern based on situation
    if (PackMembers.Num() > 0 && TacticalData.bCanCoordinate)
    {
        CurrentPattern = ECombat_AttackPattern::PackHunt;
    }
    else if (LastTargetDistance > TacticalData.AttackRange * 0.8f)
    {
        CurrentPattern = ECombat_AttackPattern::Flanking;
    }
    else if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        CurrentPattern = ECombat_AttackPattern::Ambush;
    }
    else
    {
        CurrentPattern = TacticalData.PreferredPattern;
    }
}

void UCombat_TacticalAI::ExecuteCombatBehavior()
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Patrol:
            UpdatePatrolBehavior();
            break;
        case ECombat_TacticalState::Alert:
            UpdateAlertBehavior();
            break;
        case ECombat_TacticalState::Hunting:
            UpdateHuntingBehavior();
            break;
        case ECombat_TacticalState::Attacking:
            UpdateAttackingBehavior();
            break;
        case ECombat_TacticalState::Retreating:
            UpdateRetreatingBehavior();
            break;
        case ECombat_TacticalState::Coordinating:
            UpdateCoordinatingBehavior();
            break;
    }
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    if (!PrimaryTarget)
        return false;
    
    // Retreat if health is low (if we had a health component)
    // For now, retreat based on fear threshold and distance
    float ThreatLevel = 1.0f - (LastTargetDistance / TacticalData.AttackRange);
    return ThreatLevel > (1.0f - TacticalData.FearThreshold) && LastTargetDistance < TacticalData.FleeRange;
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;
    
    // Simple pack coordination - all members follow leader's state
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member->GetCurrentState() != CurrentState)
        {
            Member->ReceivePackCommand(CurrentState);
        }
    }
}

void UCombat_TacticalAI::SetPrimaryTarget(AActor* NewTarget)
{
    PrimaryTarget = NewTarget;
    if (PrimaryTarget)
    {
        LastTargetDistance = GetDistanceToTarget();
    }
}

float UCombat_TacticalAI::GetDistanceToTarget() const
{
    if (!PrimaryTarget || !GetOwner())
        return 0.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
}

void UCombat_TacticalAI::RegisterPackMember(UCombat_TacticalAI* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
    }
}

void UCombat_TacticalAI::RemovePackMember(UCombat_TacticalAI* Member)
{
    PackMembers.Remove(Member);
}

// Private helper functions
void UCombat_TacticalAI::UpdatePatrolBehavior()
{
    // Basic patrol - could be enhanced with waypoints
    // For now, just idle behavior
}

void UCombat_TacticalAI::UpdateAlertBehavior()
{
    // Look around, increase awareness
    // Could rotate towards last known target position
}

void UCombat_TacticalAI::UpdateHuntingBehavior()
{
    if (PrimaryTarget)
    {
        SelectAttackPattern();
        
        // Move towards target based on attack pattern
        FVector TargetLocation = PrimaryTarget->GetActorLocation();
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        
        switch (CurrentPattern)
        {
            case ECombat_AttackPattern::Flanking:
                TargetLocation = CalculateFlankingPosition();
                break;
            case ECombat_AttackPattern::Ambush:
                TargetLocation = CalculateAmbushPosition();
                break;
            case ECombat_AttackPattern::PackHunt:
                CoordinateWithPack();
                break;
        }
        
        // Simple movement towards target (would need proper pathfinding in real implementation)
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        // Apply movement to pawn if it has movement component
    }
}

void UCombat_TacticalAI::UpdateAttackingBehavior()
{
    if (PrimaryTarget && IsTargetInRange() && !IsAttackOnCooldown())
    {
        // Execute attack
        LastAttackTime = GetWorld()->GetTimeSeconds();
        
        // Could trigger attack animation/damage here
        // For now, just log the attack
        UE_LOG(LogTemp, Warning, TEXT("Combat AI executing attack on target"));
    }
}

void UCombat_TacticalAI::UpdateRetreatingBehavior()
{
    if (PrimaryTarget)
    {
        // Move away from target
        FVector TargetLocation = PrimaryTarget->GetActorLocation();
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector FleeDirection = (CurrentLocation - TargetLocation).GetSafeNormal();
        
        // Apply retreat movement
        FVector FleeTarget = CurrentLocation + (FleeDirection * TacticalData.FleeRange);
        // Apply movement to pawn
    }
}

void UCombat_TacticalAI::UpdateCoordinatingBehavior()
{
    // Pack coordination behavior
    CoordinateWithPack();
}

bool UCombat_TacticalAI::CanSeeTarget() const
{
    if (!PrimaryTarget || !GetOwner() || !GetWorld())
        return false;
    
    // Simple line trace to check visibility
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = PrimaryTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(PrimaryTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstruction
}

bool UCombat_TacticalAI::IsTargetInRange() const
{
    return LastTargetDistance <= TacticalData.AttackRange;
}

bool UCombat_TacticalAI::IsAttackOnCooldown() const
{
    if (!GetWorld())
        return true;
    
    return (GetWorld()->GetTimeSeconds() - LastAttackTime) < AttackCooldown;
}

FVector UCombat_TacticalAI::CalculateFlankingPosition() const
{
    if (!PrimaryTarget || !GetOwner())
        return GetOwner()->GetActorLocation();
    
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Calculate a position to the side of the target
    FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Choose left or right flank randomly
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankOffset = RightVector * FlankDirection * TacticalData.AttackRange * 0.7f;
    
    return TargetLocation + FlankOffset;
}

FVector UCombat_TacticalAI::CalculateAmbushPosition() const
{
    if (!PrimaryTarget || !GetOwner())
        return GetOwner()->GetActorLocation();
    
    // Find a position behind cover near the target
    // For now, just return a position slightly behind and to the side
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector BehindTarget = TargetLocation - (ToTarget * TacticalData.AttackRange * 0.5f);
    
    return BehindTarget;
}

void UCombat_TacticalAI::BroadcastStateToPackMembers()
{
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member)
        {
            Member->ReceivePackCommand(CurrentState);
        }
    }
}

void UCombat_TacticalAI::ReceivePackCommand(ECombat_TacticalState CommandState)
{
    if (!bIsPackLeader && PackLeader)
    {
        // Follow pack leader's command
        SetTacticalState(CommandState);
        if (PackLeader->GetPrimaryTarget())
        {
            SetPrimaryTarget(PackLeader->GetPrimaryTarget());
        }
    }
}