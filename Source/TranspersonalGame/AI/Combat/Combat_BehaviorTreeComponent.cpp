#include "Combat_BehaviorTreeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_BehaviorTreeComponent::UCombat_BehaviorTreeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentBehaviorState = ECombat_BehaviorState::Idle;
    CurrentTarget = nullptr;
    LastStateChangeTime = 0.0f;
    PackLeader = nullptr;
    BehaviorTreeComp = nullptr;
    BlackboardComp = nullptr;
}

void UCombat_BehaviorTreeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize behavior tree components
    if (AAIController* AIController = Cast<AAIController>(GetOwner()))
    {
        BehaviorTreeComp = AIController->GetBehaviorTreeComponent();
        BlackboardComp = AIController->GetBlackboardComponent();
    }
    
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
}

void UCombat_BehaviorTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
        
    UpdateBehaviorState(DeltaTime);
    UpdateTarget();
    UpdatePackBehavior();
}

void UCombat_BehaviorTreeComponent::InitializeBehaviorTree(UBehaviorTree* BehaviorTree)
{
    if (!BehaviorTree || !BehaviorTreeComp)
        return;
        
    BehaviorTreeComp->StartTree(*BehaviorTree);
    
    // Set initial blackboard values
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), BehaviorConfig.AggressionLevel);
        BlackboardComp->SetValueAsFloat(TEXT("DetectionRange"), BehaviorConfig.DetectionRange);
        BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), BehaviorConfig.AttackRange);
    }
}

void UCombat_BehaviorTreeComponent::SetBehaviorState(ECombat_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Update blackboard
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s: Behavior state changed to %d"), 
               *GetOwner()->GetName(), static_cast<int32>(NewState));
    }
}

void UCombat_BehaviorTreeComponent::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        // Update blackboard
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsObject(TEXT("TargetActor"), NewTarget);
            
            if (NewTarget)
            {
                BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), NewTarget->GetActorLocation());
            }
        }
        
        if (NewTarget)
        {
            UE_LOG(LogTemp, Log, TEXT("%s: New target set - %s"), 
                   *GetOwner()->GetName(), *NewTarget->GetName());
        }
    }
}

bool UCombat_BehaviorTreeComponent::ShouldAttackTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Check if target is in attack range
    if (Distance > BehaviorConfig.AttackRange)
        return false;
        
    // Check aggression level vs target threat
    float ThreatLevel = IsTargetThreatening() ? 0.8f : 0.3f;
    
    return BehaviorConfig.AggressionLevel > ThreatLevel;
}

bool UCombat_BehaviorTreeComponent::ShouldFleeFromTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;
        
    // Check if target is too threatening
    if (!IsTargetThreatening())
        return false;
        
    // Check if we're below flee threshold
    float HealthRatio = 1.0f; // TODO: Get actual health ratio from health component
    
    return HealthRatio < BehaviorConfig.FleeThreshold;
}

FVector UCombat_BehaviorTreeComponent::GetTacticalPosition(AActor* Target) const
{
    if (!Target || !GetOwner())
        return GetOwner()->GetActorLocation();
        
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Calculate flanking position based on behavior state
    FVector TacticalOffset;
    
    switch (CurrentBehaviorState)
    {
        case ECombat_BehaviorState::Stalking:
            // Position behind target
            TacticalOffset = -DirectionToTarget * BehaviorConfig.AttackRange * 0.8f;
            break;
            
        case ECombat_BehaviorState::Attacking:
            // Close distance
            TacticalOffset = DirectionToTarget * (BehaviorConfig.AttackRange * 0.5f);
            break;
            
        case ECombat_BehaviorState::Defending:
            // Maintain distance
            TacticalOffset = -DirectionToTarget * BehaviorConfig.AttackRange;
            break;
            
        case ECombat_BehaviorState::Fleeing:
            // Run away
            TacticalOffset = -DirectionToTarget * BehaviorConfig.DetectionRange;
            break;
            
        default:
            // Circle around target
            FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
            TacticalOffset = RightVector * BehaviorConfig.AttackRange * 0.7f;
            break;
    }
    
    return TargetLocation + TacticalOffset;
}

void UCombat_BehaviorTreeComponent::JoinPack(UCombat_BehaviorTreeComponent* NewPackLeader)
{
    if (!NewPackLeader || NewPackLeader == this)
        return;
        
    // Leave current pack first
    LeavePack();
    
    // Join new pack
    PackLeader = NewPackLeader;
    NewPackLeader->PackMembers.AddUnique(this);
    
    UE_LOG(LogTemp, Log, TEXT("%s: Joined pack led by %s"), 
           *GetOwner()->GetName(), *NewPackLeader->GetOwner()->GetName());
}

void UCombat_BehaviorTreeComponent::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // Transfer leadership if this was the leader
    if (PackMembers.Num() > 0)
    {
        UCombat_BehaviorTreeComponent* NewLeader = PackMembers[0];
        for (UCombat_BehaviorTreeComponent* Member : PackMembers)
        {
            if (Member != NewLeader)
            {
                Member->PackLeader = NewLeader;
                NewLeader->PackMembers.Add(Member);
            }
        }
        PackMembers.Empty();
    }
}

void UCombat_BehaviorTreeComponent::UpdateBehaviorState(float DeltaTime)
{
    if (!GetOwner())
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastChange = CurrentTime - LastStateChangeTime;
    
    // Don't change states too frequently
    if (TimeSinceLastChange < 1.0f)
        return;
        
    ECombat_BehaviorState NewState = CurrentBehaviorState;
    
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget();
        
        if (ShouldFleeFromTarget(CurrentTarget))
        {
            NewState = ECombat_BehaviorState::Fleeing;
        }
        else if (IsTargetInAttackRange())
        {
            NewState = ECombat_BehaviorState::Attacking;
        }
        else if (DistanceToTarget < BehaviorConfig.DetectionRange)
        {
            NewState = ECombat_BehaviorState::Stalking;
        }
        else
        {
            NewState = ECombat_BehaviorState::Hunting;
        }
    }
    else
    {
        // No target - patrol or idle
        NewState = (FMath::RandRange(0.0f, 1.0f) < 0.3f) ? ECombat_BehaviorState::Patrolling : ECombat_BehaviorState::Idle;
    }
    
    SetBehaviorState(NewState);
}

void UCombat_BehaviorTreeComponent::UpdateTarget()
{
    if (!GetOwner())
        return;
        
    // Find nearby potential targets
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    AActor* BestTarget = nullptr;
    float BestTargetDistance = BehaviorConfig.DetectionRange;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
            
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance < BestTargetDistance)
        {
            // Check if this is a valid target (different species, player, etc.)
            if (Actor->Tags.Contains("Player") || Actor->Tags.Contains("Dinosaur"))
            {
                BestTarget = Actor;
                BestTargetDistance = Distance;
            }
        }
    }
    
    SetTarget(BestTarget);
}

void UCombat_BehaviorTreeComponent::UpdatePackBehavior()
{
    if (!BehaviorConfig.bCanFormPacks)
        return;
        
    // Pack formation logic for pack hunters like velociraptors
    if (!IsInPack() && CurrentBehaviorState == ECombat_BehaviorState::Hunting)
    {
        // Look for nearby pack members
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), NearbyActors);
        
        for (AActor* Actor : NearbyActors)
        {
            if (!Actor || Actor == GetOwner())
                continue;
                
            UCombat_BehaviorTreeComponent* OtherBehavior = Actor->FindComponentByClass<UCombat_BehaviorTreeComponent>();
            if (OtherBehavior && OtherBehavior->BehaviorConfig.bCanFormPacks)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance < 1000.0f) // Pack formation range
                {
                    if (!OtherBehavior->IsInPack())
                    {
                        JoinPack(OtherBehavior);
                        break;
                    }
                    else if (OtherBehavior->PackMembers.Num() < BehaviorConfig.MaxPackSize)
                    {
                        JoinPack(OtherBehavior->PackLeader ? OtherBehavior->PackLeader : OtherBehavior);
                        break;
                    }
                }
            }
        }
    }
}

float UCombat_BehaviorTreeComponent::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return 0.0f;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UCombat_BehaviorTreeComponent::IsTargetInAttackRange() const
{
    return GetDistanceToTarget() <= BehaviorConfig.AttackRange;
}

bool UCombat_BehaviorTreeComponent::IsTargetThreatening() const
{
    if (!CurrentTarget)
        return false;
        
    // Simple threat assessment - players and large dinosaurs are threatening
    return CurrentTarget->Tags.Contains("Player") || 
           CurrentTarget->Tags.Contains("apex_predator") ||
           CurrentTarget->GetActorScale3D().Size() > 2.0f;
}