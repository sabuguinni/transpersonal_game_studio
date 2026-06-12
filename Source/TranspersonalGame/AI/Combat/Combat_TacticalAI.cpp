#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI updates
    
    CurrentState = ECombat_TacticalState::Idle;
    CurrentFormation = ECombat_Formation::None;
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    StateTimer = 0.0f;
    LastCoordinationTime = 0.0f;
    CoordinationInterval = 2.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tactical data based on owner type
    if (AActor* Owner = GetOwner())
    {
        FString ActorName = Owner->GetName().ToLower();
        
        // Configure based on dinosaur type
        if (ActorName.Contains("trex"))
        {
            TacticalData.AggressionLevel = 0.9f;
            TacticalData.CoordinationSkill = 0.3f; // Solo hunter
            TacticalData.RetreatThreshold = 0.1f;
            TacticalData.MaxPackSize = 1;
        }
        else if (ActorName.Contains("veloci"))
        {
            TacticalData.AggressionLevel = 0.8f;
            TacticalData.CoordinationSkill = 0.9f; // Pack hunter
            TacticalData.RetreatThreshold = 0.4f;
            TacticalData.MaxPackSize = 6;
        }
        else if (ActorName.Contains("brachi"))
        {
            TacticalData.AggressionLevel = 0.2f;
            TacticalData.CoordinationSkill = 0.5f; // Herd behavior
            TacticalData.RetreatThreshold = 0.7f;
            TacticalData.MaxPackSize = 12;
        }
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    UpdateTacticalBehavior(DeltaTime);
    
    // Periodic pack coordination
    if (GetWorld()->GetTimeSeconds() - LastCoordinationTime > CoordinationInterval)
    {
        UpdatePackCoordination();
        LastCoordinationTime = GetWorld()->GetTimeSeconds();
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Debug logging
        if (GEngine && GetOwner())
        {
            FString StateName = UEnum::GetValueAsString(NewState);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("%s: Tactical State -> %s"), *GetOwner()->GetName(), *StateName));
        }
    }
}

void UCombat_TacticalAI::SetTarget(APawn* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        SetTacticalState(ECombat_TacticalState::Hunt);
    }
    else
    {
        SetTacticalState(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAI::JoinPack(UCombat_TacticalAI* NewPackLeader)
{
    if (NewPackLeader && NewPackLeader != this)
    {
        // Leave current pack first
        LeavePack();
        
        PackLeader = NewPackLeader;
        NewPackLeader->PackMembers.AddUnique(this);
        
        // Sync target with pack leader
        if (NewPackLeader->CurrentTarget)
        {
            SetTarget(NewPackLeader->CurrentTarget);
        }
    }
}

void UCombat_TacticalAI::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // If this was a pack leader, disband the pack
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackLeader = nullptr;
        }
    }
    PackMembers.Empty();
}

void UCombat_TacticalAI::ExecuteFlankingManeuver(FVector TargetPosition)
{
    if (!GetOwner() || !CurrentTarget)
        return;
        
    FVector FlankPosition = CalculateFlankingPosition();
    
    // Move towards flanking position
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (FlankPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 1.0f);
        
        SetTacticalState(ECombat_TacticalState::Flank);
    }
}

void UCombat_TacticalAI::CoordinateAttack()
{
    if (!IsInPack() || !CurrentTarget)
        return;
        
    // Pack leader coordinates the attack
    if (PackLeader == nullptr) // This is the pack leader
    {
        CurrentFormation = ECombat_Formation::Pincer;
        
        // Assign roles to pack members
        for (int32 i = 0; i < PackMembers.Num(); i++)
        {
            if (UCombat_TacticalAI* Member = PackMembers[i])
            {
                if (i % 2 == 0)
                {
                    Member->SetTacticalState(ECombat_TacticalState::Flank);
                }
                else
                {
                    Member->SetTacticalState(ECombat_TacticalState::Engage);
                }
            }
        }
    }
}

void UCombat_TacticalAI::UpdateTacticalBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Idle:
            // Look for targets or join nearby packs
            if (!CurrentTarget)
            {
                // Simple target acquisition - find player
                if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
                {
                    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
                    if (Distance < 3000.0f) // Detection range
                    {
                        SetTarget(Player);
                    }
                }
            }
            break;
            
        case ECombat_TacticalState::Hunt:
            ProcessHuntingBehavior();
            break;
            
        case ECombat_TacticalState::Engage:
            ProcessEngagementBehavior();
            break;
            
        case ECombat_TacticalState::Flank:
            ProcessFlankingBehavior();
            break;
            
        case ECombat_TacticalState::Retreat:
            ProcessRetreatBehavior();
            break;
    }
    
    // Check for retreat conditions
    if (ShouldRetreat() && CurrentState != ECombat_TacticalState::Retreat)
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
}

void UCombat_TacticalAI::ProcessHuntingBehavior()
{
    if (!CurrentTarget || !CanSeeTarget())
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget();
    
    if (DistanceToTarget < 800.0f) // Engagement range
    {
        if (IsInPack() && TacticalData.CoordinationSkill > 0.7f)
        {
            CoordinateAttack();
        }
        else
        {
            SetTacticalState(ECombat_TacticalState::Engage);
        }
    }
    else
    {
        // Move towards target
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            FVector Direction = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
            OwnerPawn->AddMovementInput(Direction, TacticalData.AggressionLevel);
        }
    }
}

void UCombat_TacticalAI::ProcessEngagementBehavior()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }
    
    // Direct attack behavior
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Direction = (CurrentTarget->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction, 1.0f);
        
        // Simple attack simulation
        float AttackRange = 200.0f;
        if (GetDistanceToTarget() < AttackRange)
        {
            // Attack logic would go here
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, 
                    FString::Printf(TEXT("%s: ATTACKING!"), *GetOwner()->GetName()));
            }
        }
    }
}

void UCombat_TacticalAI::ProcessFlankingBehavior()
{
    if (!CurrentTarget)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
        return;
    }
    
    FVector FlankPosition = CalculateFlankingPosition();
    
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        float DistanceToFlankPos = FVector::Dist(OwnerPawn->GetActorLocation(), FlankPosition);
        
        if (DistanceToFlankPos < 300.0f) // Reached flanking position
        {
            SetTacticalState(ECombat_TacticalState::Engage);
        }
        else
        {
            FVector Direction = (FlankPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
            OwnerPawn->AddMovementInput(Direction, 0.8f);
        }
    }
}

void UCombat_TacticalAI::ProcessRetreatBehavior()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (CurrentTarget)
        {
            // Retreat away from target
            FVector Direction = (OwnerPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
            OwnerPawn->AddMovementInput(Direction, 1.0f);
            
            // Stop retreating after some distance
            if (GetDistanceToTarget() > 2000.0f)
            {
                SetTacticalState(ECombat_TacticalState::Idle);
                CurrentTarget = nullptr;
            }
        }
    }
}

void UCombat_TacticalAI::UpdatePackCoordination()
{
    if (IsInPack() && CurrentTarget)
    {
        BroadcastTacticalInfo();
    }
}

void UCombat_TacticalAI::BroadcastTacticalInfo()
{
    if (!CurrentTarget)
        return;
        
    FVector TargetPos = CurrentTarget->GetActorLocation();
    
    // Share info with pack members
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->ReceiveTacticalInfo(TargetPos, CurrentState);
        }
    }
}

void UCombat_TacticalAI::ReceiveTacticalInfo(const FVector& TargetPos, ECombat_TacticalState SuggestedState)
{
    // Update target position if we don't have a target
    if (!CurrentTarget)
    {
        if (APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float Distance = FVector::Dist(TargetPos, Player->GetActorLocation());
            if (Distance < 500.0f) // Target position is close to player
            {
                SetTarget(Player);
            }
        }
    }
}

bool UCombat_TacticalAI::CanSeeTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return false;
        
    // Simple line of sight check
    FHitResult HitResult;
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
    
    return !bHit || HitResult.GetActor() == CurrentTarget;
}

float UCombat_TacticalAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return FLT_MAX;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

FVector UCombat_TacticalAI::CalculateFlankingPosition() const
{
    if (!CurrentTarget || !GetOwner())
        return GetOwner()->GetActorLocation();
        
    FVector TargetPos = CurrentTarget->GetActorLocation();
    FVector OwnerPos = GetOwner()->GetActorLocation();
    
    // Calculate perpendicular flanking position
    FVector ToTarget = (TargetPos - OwnerPos).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Alternate left/right based on pack position
    float FlankDirection = (PackMembers.Find(this) % 2 == 0) ? 1.0f : -1.0f;
    
    return TargetPos + (RightVector * TacticalData.FlankingRange * FlankDirection);
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    // Simple health-based retreat logic
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        // This would check actual health in a real implementation
        // For now, use a random chance based on aggression
        float RetreatChance = (1.0f - TacticalData.AggressionLevel) * 0.1f;
        return FMath::RandRange(0.0f, 1.0f) < RetreatChance;
    }
    
    return false;
}