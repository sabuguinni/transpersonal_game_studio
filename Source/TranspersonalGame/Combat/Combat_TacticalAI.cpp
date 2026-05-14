#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentTacticalState = ECombat_TacticalState::Idle;
    StateChangeTime = GetWorld()->GetTimeSeconds();
    LastTargetUpdateTime = 0.0f;
    
    // Initialize flanking positions around the owner
    UpdateFlankingPositions();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Update targets and tactical state
    UpdateTargets(DeltaTime);
    UpdateTacticalState(DeltaTime);
    
    // Update flanking positions periodically
    if (GetWorld()->GetTimeSeconds() - LastTargetUpdateTime > 2.0f)
    {
        UpdateFlankingPositions();
        LastTargetUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        ECombat_TacticalState PreviousState = CurrentTacticalState;
        CurrentTacticalState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("%s tactical state changed from %d to %d"), 
               *GetOwner()->GetName(), (int32)PreviousState, (int32)NewState);
    }
}

void UCombat_TacticalAI::AddTarget(AActor* Target, float ThreatLevel)
{
    if (!Target)
    {
        return;
    }
    
    // Check if target already exists
    FCombat_TacticalTarget* ExistingTarget = FindTarget(Target);
    if (ExistingTarget)
    {
        ExistingTarget->ThreatLevel = ThreatLevel;
        ExistingTarget->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingTarget->LastKnownPosition = Target->GetActorLocation();
        ExistingTarget->bIsVisible = true;
        return;
    }
    
    // Add new target
    FCombat_TacticalTarget NewTarget;
    NewTarget.TargetActor = Target;
    NewTarget.LastKnownPosition = Target->GetActorLocation();
    NewTarget.ThreatLevel = ThreatLevel;
    NewTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewTarget.bIsVisible = true;
    
    KnownTargets.Add(NewTarget);
    
    UE_LOG(LogTemp, Log, TEXT("%s added target %s with threat level %.2f"), 
           *GetOwner()->GetName(), *Target->GetName(), ThreatLevel);
}

void UCombat_TacticalAI::RemoveTarget(AActor* Target)
{
    KnownTargets.RemoveAll([Target](const FCombat_TacticalTarget& TargetData)
    {
        return TargetData.TargetActor == Target;
    });
}

AActor* UCombat_TacticalAI::GetPrimaryTarget() const
{
    if (KnownTargets.Num() == 0)
    {
        return nullptr;
    }
    
    // Find target with highest threat level that's still valid
    const FCombat_TacticalTarget* BestTarget = nullptr;
    float HighestThreat = 0.0f;
    
    for (const FCombat_TacticalTarget& Target : KnownTargets)
    {
        if (Target.TargetActor && Target.ThreatLevel > HighestThreat)
        {
            BestTarget = &Target;
            HighestThreat = Target.ThreatLevel;
        }
    }
    
    return BestTarget ? BestTarget->TargetActor : nullptr;
}

FVector UCombat_TacticalAI::GetTargetLastKnownPosition(AActor* Target) const
{
    const FCombat_TacticalTarget* TargetData = FindTarget(Target);
    return TargetData ? TargetData->LastKnownPosition : FVector::ZeroVector;
}

FVector UCombat_TacticalAI::FindBestFlankingPosition(AActor* Target, float MinDistance, float MaxDistance)
{
    if (!Target || !GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Generate potential flanking positions in a circle around the target
    TArray<FVector> CandidatePositions;
    int32 NumPositions = 8; // Check 8 positions around the target
    
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = (2.0f * PI * i) / NumPositions;
        float Distance = FMath::RandRange(MinDistance, MaxDistance);
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FVector CandidatePos = TargetLocation + Offset;
        CandidatePositions.Add(CandidatePos);
    }
    
    // Evaluate each position and find the best one
    FVector BestPosition = FVector::ZeroVector;
    float BestScore = -1.0f;
    
    for (const FVector& Position : CandidatePositions)
    {
        if (IsValidFlankingPosition(Position, Target))
        {
            float Score = CalculateFlankingScore(Position, Target);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestPosition = Position;
            }
        }
    }
    
    return BestPosition;
}

bool UCombat_TacticalAI::CanFlankTarget(AActor* Target) const
{
    if (!bCanFlank || !Target || !GetOwner())
    {
        return false;
    }
    
    // Check if we have group members to coordinate with
    if (bIsGroupLeader && GroupMembers.Num() > 0)
    {
        return true;
    }
    
    // Check if target is distracted or engaged with another threat
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToTarget = FVector::Dist(TargetLocation, OwnerLocation);
    
    return DistanceToTarget > AttackRange * 1.5f; // Need some distance to flank effectively
}

void UCombat_TacticalAI::RegisterWithGroup(UCombat_TacticalAI* GroupLeader)
{
    if (GroupLeader && GroupLeader != this)
    {
        this->GroupLeader = GroupLeader;
        GroupLeader->AddGroupMember(this);
        bIsGroupLeader = false;
        
        UE_LOG(LogTemp, Log, TEXT("%s registered with group leader %s"), 
               *GetOwner()->GetName(), *GroupLeader->GetOwner()->GetName());
    }
}

void UCombat_TacticalAI::AddGroupMember(UCombat_TacticalAI* Member)
{
    if (Member && Member != this && !GroupMembers.Contains(Member))
    {
        GroupMembers.Add(Member);
        bIsGroupLeader = true;
        
        UE_LOG(LogTemp, Log, TEXT("%s added group member %s"), 
               *GetOwner()->GetName(), *Member->GetOwner()->GetName());
    }
}

void UCombat_TacticalAI::CoordinateGroupAttack(AActor* Target)
{
    if (!bIsGroupLeader || !Target || GroupMembers.Num() == 0)
    {
        return;
    }
    
    // Assign different roles to group members
    for (int32 i = 0; i < GroupMembers.Num(); i++)
    {
        UCombat_TacticalAI* Member = GroupMembers[i];
        if (!Member || !Member->GetOwner())
        {
            continue;
        }
        
        // Assign tactics based on position in group
        if (i % 2 == 0)
        {
            // Even members: direct attack
            Member->SetTacticalState(ECombat_TacticalState::Attacking);
        }
        else
        {
            // Odd members: flanking maneuver
            Member->SetTacticalState(ECombat_TacticalState::Flanking);
        }
        
        Member->AddTarget(Target, CalculateThreatLevel(Target));
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s coordinating group attack on %s with %d members"), 
           *GetOwner()->GetName(), *Target->GetName(), GroupMembers.Num());
}

float UCombat_TacticalAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return 0.0f;
    }
    
    float ThreatLevel = 0.0f;
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(TargetLocation, OwnerLocation);
    
    // Base threat based on distance (closer = more threatening)
    if (Distance < AttackRange)
    {
        ThreatLevel += 1.0f;
    }
    else if (Distance < SightRange)
    {
        ThreatLevel += 0.5f * (1.0f - (Distance / SightRange));
    }
    
    // Check if target is player
    if (Cast<APawn>(Target) && Cast<APawn>(Target)->IsPlayerControlled())
    {
        ThreatLevel += 0.5f; // Players are inherently more threatening
    }
    
    // Factor in target's current activity/state
    if (Target->GetVelocity().Size() > 100.0f)
    {
        ThreatLevel += 0.2f; // Moving targets are more concerning
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

ECombat_ThreatLevel UCombat_TacticalAI::GetOverallThreatLevel() const
{
    float TotalThreat = 0.0f;
    int32 ValidTargets = 0;
    
    for (const FCombat_TacticalTarget& Target : KnownTargets)
    {
        if (Target.TargetActor)
        {
            TotalThreat += Target.ThreatLevel;
            ValidTargets++;
        }
    }
    
    if (ValidTargets == 0)
    {
        return ECombat_ThreatLevel::None;
    }
    
    float AverageThreat = TotalThreat / ValidTargets;
    
    if (AverageThreat >= 0.8f)
    {
        return ECombat_ThreatLevel::Critical;
    }
    else if (AverageThreat >= 0.6f)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (AverageThreat >= 0.3f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (AverageThreat > 0.0f)
    {
        return ECombat_ThreatLevel::Low;
    }
    
    return ECombat_ThreatLevel::None;
}

bool UCombat_TacticalAI::ShouldEngageTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }
    
    float ThreatLevel = CalculateThreatLevel(Target);
    float Distance = FVector::Dist(Target->GetActorLocation(), GetOwner()->GetActorLocation());
    
    // Engage if target is close and threatening enough
    bool bShouldEngage = (ThreatLevel >= AggressionLevel * 0.5f) && (Distance <= SightRange);
    
    // Consider group support
    if (bIsGroupLeader && GroupMembers.Num() > 0)
    {
        bShouldEngage = bShouldEngage || (ThreatLevel >= AggressionLevel * 0.3f);
    }
    
    return bShouldEngage;
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    if (!bCanRetreat)
    {
        return false;
    }
    
    ECombat_ThreatLevel ThreatLevel = GetOverallThreatLevel();
    
    // Retreat if threat is too high
    if (ThreatLevel == ECombat_ThreatLevel::Critical)
    {
        return true;
    }
    
    // Retreat if outnumbered significantly
    if (KnownTargets.Num() > 3 && !bIsGroupLeader)
    {
        return true;
    }
    
    // Retreat if isolated from group
    if (GroupLeader && GroupMembers.Num() == 0)
    {
        float DistanceToLeader = FVector::Dist(
            GetOwner()->GetActorLocation(),
            GroupLeader->GetOwner()->GetActorLocation()
        );
        
        if (DistanceToLeader > FlankingRadius * 2.0f)
        {
            return true;
        }
    }
    
    return false;
}

FVector UCombat_TacticalAI::GetRetreatPosition() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // If we have a group leader, retreat towards them
    if (GroupLeader && GroupLeader->GetOwner())
    {
        FVector LeaderLocation = GroupLeader->GetOwner()->GetActorLocation();
        FVector Direction = (LeaderLocation - OwnerLocation).GetSafeNormal();
        return OwnerLocation + (Direction * FlankingRadius);
    }
    
    // Otherwise, retreat away from primary target
    AActor* PrimaryTarget = GetPrimaryTarget();
    if (PrimaryTarget)
    {
        FVector TargetLocation = PrimaryTarget->GetActorLocation();
        FVector Direction = (OwnerLocation - TargetLocation).GetSafeNormal();
        return OwnerLocation + (Direction * FlankingRadius * 1.5f);
    }
    
    // Fallback: retreat to a random safe position
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    return OwnerLocation + (RandomDirection * FlankingRadius);
}

void UCombat_TacticalAI::UpdateTargets(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update existing targets
    for (FCombat_TacticalTarget& Target : KnownTargets)
    {
        if (Target.TargetActor)
        {
            Target.LastKnownPosition = Target.TargetActor->GetActorLocation();
            Target.ThreatLevel = CalculateThreatLevel(Target.TargetActor);
            
            // Check if target is still visible (simplified check)
            float Distance = FVector::Dist(Target.LastKnownPosition, GetOwner()->GetActorLocation());
            Target.bIsVisible = Distance <= SightRange;
            
            if (Target.bIsVisible)
            {
                Target.LastSeenTime = CurrentTime;
            }
        }
    }
    
    // Remove old or invalid targets
    CleanupInvalidTargets();
}

void UCombat_TacticalAI::UpdateTacticalState(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    ECombat_TacticalState NewState = CurrentTacticalState;
    
    // State transition logic
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Idle:
        {
            if (KnownTargets.Num() > 0)
            {
                NewState = ECombat_TacticalState::Investigating;
            }
            break;
        }
        
        case ECombat_TacticalState::Investigating:
        {
            AActor* PrimaryTarget = GetPrimaryTarget();
            if (PrimaryTarget && ShouldEngageTarget(PrimaryTarget))
            {
                if (CanFlankTarget(PrimaryTarget))
                {
                    NewState = ECombat_TacticalState::Flanking;
                }
                else
                {
                    NewState = ECombat_TacticalState::Attacking;
                }
            }
            else if (KnownTargets.Num() == 0)
            {
                NewState = ECombat_TacticalState::Idle;
            }
            break;
        }
        
        case ECombat_TacticalState::Flanking:
        case ECombat_TacticalState::Attacking:
        {
            if (ShouldRetreat())
            {
                NewState = ECombat_TacticalState::Retreating;
            }
            else if (KnownTargets.Num() == 0)
            {
                NewState = ECombat_TacticalState::Idle;
            }
            break;
        }
        
        case ECombat_TacticalState::Retreating:
        {
            if (TimeSinceStateChange > 3.0f && !ShouldRetreat())
            {
                if (bIsGroupLeader && GroupMembers.Num() > 0)
                {
                    NewState = ECombat_TacticalState::Regrouping;
                }
                else
                {
                    NewState = ECombat_TacticalState::Idle;
                }
            }
            break;
        }
        
        case ECombat_TacticalState::Regrouping:
        {
            if (TimeSinceStateChange > 5.0f)
            {
                NewState = ECombat_TacticalState::Patrolling;
            }
            break;
        }
        
        case ECombat_TacticalState::Patrolling:
        {
            if (KnownTargets.Num() > 0)
            {
                NewState = ECombat_TacticalState::Investigating;
            }
            break;
        }
    }
    
    if (NewState != CurrentTacticalState)
    {
        SetTacticalState(NewState);
    }
}

void UCombat_TacticalAI::UpdateFlankingPositions()
{
    if (!GetOwner())
    {
        return;
    }
    
    FlankingPositions.Empty();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Generate flanking positions in a circle around the owner
    int32 NumPositions = 6;
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = (2.0f * PI * i) / NumPositions;
        FVector Position = OwnerLocation + FVector(
            FMath::Cos(Angle) * FlankingRadius,
            FMath::Sin(Angle) * FlankingRadius,
            0.0f
        );
        
        FCombat_FlankingPosition FlankPos;
        FlankPos.Position = Position;
        FlankPos.Score = 0.5f; // Base score
        FlankPos.bIsOccupied = false;
        
        FlankingPositions.Add(FlankPos);
    }
}

FCombat_TacticalTarget* UCombat_TacticalAI::FindTarget(AActor* Target)
{
    for (FCombat_TacticalTarget& TargetData : KnownTargets)
    {
        if (TargetData.TargetActor == Target)
        {
            return &TargetData;
        }
    }
    return nullptr;
}

void UCombat_TacticalAI::CleanupInvalidTargets()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    KnownTargets.RemoveAll([CurrentTime](const FCombat_TacticalTarget& Target)
    {
        // Remove if actor is null or hasn't been seen for too long
        return !Target.TargetActor || 
               !IsValid(Target.TargetActor) ||
               (CurrentTime - Target.LastSeenTime) > 10.0f;
    });
}

bool UCombat_TacticalAI::IsValidFlankingPosition(const FVector& Position, AActor* Target) const
{
    if (!GetWorld() || !Target)
    {
        return false;
    }
    
    // Check if position is navigable (simplified check)
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (!NavSys->ProjectPointToNavigation(Position, NavLoc, FVector(100.0f, 100.0f, 100.0f)))
        {
            return false;
        }
    }
    
    // Check if position provides tactical advantage
    FVector TargetLocation = Target->GetActorLocation();
    float DistanceToTarget = FVector::Dist(Position, TargetLocation);
    
    return DistanceToTarget >= AttackRange * 0.8f && DistanceToTarget <= FlankingRadius * 1.5f;
}

float UCombat_TacticalAI::CalculateFlankingScore(const FVector& Position, AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return 0.0f;
    }
    
    float Score = 0.0f;
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Distance score (prefer optimal attack range)
    float DistanceToTarget = FVector::Dist(Position, TargetLocation);
    float OptimalDistance = AttackRange * 1.2f;
    float DistanceScore = 1.0f - FMath::Abs(DistanceToTarget - OptimalDistance) / OptimalDistance;
    Score += DistanceScore * 0.4f;
    
    // Angle score (prefer flanking from sides/behind)
    FVector TargetForward = Target->GetActorForwardVector();
    FVector ToFlankPosition = (Position - TargetLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(TargetForward, ToFlankPosition);
    float AngleScore = 1.0f - ((DotProduct + 1.0f) * 0.5f); // Prefer positions behind target
    Score += AngleScore * 0.4f;
    
    // Safety score (avoid positions too close to other threats)
    float SafetyScore = 1.0f;
    for (const FCombat_TacticalTarget& OtherTarget : KnownTargets)
    {
        if (OtherTarget.TargetActor && OtherTarget.TargetActor != Target)
        {
            float DistanceToOther = FVector::Dist(Position, OtherTarget.LastKnownPosition);
            if (DistanceToOther < AttackRange * 2.0f)
            {
                SafetyScore *= 0.5f;
            }
        }
    }
    Score += SafetyScore * 0.2f;
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}