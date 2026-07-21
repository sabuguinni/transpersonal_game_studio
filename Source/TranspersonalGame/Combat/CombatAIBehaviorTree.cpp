#include "CombatAIBehaviorTree.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UCombat_AIBehaviorTree::UCombat_AIBehaviorTree()
{
    CurrentCombatState = ECombat_AIState::Idle;
    AggressionLevel = 0.5f;
    FearLevel = 0.3f;
    TacticalIntelligence = 0.7f;
    PrimaryTarget = nullptr;
    DetectionRange = 2000.0f;
    AttackRange = 300.0f;
    LastCombatUpdateTime = 0.0f;
    CombatStateTimer = 0.0f;
    OwnerPawn = nullptr;
    BlackboardComp = nullptr;
}

void UCombat_AIBehaviorTree::InitializeCombatBehavior(APawn* InOwnerPawn)
{
    OwnerPawn = InOwnerPawn;
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAI: No owner pawn provided"));
        return;
    }

    // Get AI Controller and Blackboard
    if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
    {
        BlackboardComp = AIController->GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
            BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), FearLevel);
        }
    }

    CurrentCombatState = ECombat_AIState::Patrol;
    UE_LOG(LogTemp, Log, TEXT("CombatAI: Initialized for %s"), *OwnerPawn->GetName());
}

void UCombat_AIBehaviorTree::UpdateCombatState(float DeltaTime)
{
    if (!OwnerPawn)
        return;

    LastCombatUpdateTime += DeltaTime;
    CombatStateTimer += DeltaTime;

    // Update every 0.5 seconds for performance
    if (LastCombatUpdateTime < 0.5f)
        return;

    LastCombatUpdateTime = 0.0f;

    // Calculate dynamic values
    CalculateAggressionLevel();
    CalculateFearLevel();
    UpdateTargetPriorities();

    // State machine logic
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Idle:
            if (PrimaryTarget && GetDistanceToTarget(PrimaryTarget) < DetectionRange)
            {
                CurrentCombatState = ECombat_AIState::Alert;
                CombatStateTimer = 0.0f;
            }
            break;

        case ECombat_AIState::Patrol:
            if (PrimaryTarget && IsTargetVisible(PrimaryTarget))
            {
                CurrentCombatState = ECombat_AIState::Alert;
                CombatStateTimer = 0.0f;
            }
            break;

        case ECombat_AIState::Alert:
            if (PrimaryTarget)
            {
                float DistanceToTarget = GetDistanceToTarget(PrimaryTarget);
                if (DistanceToTarget < AttackRange && ShouldEngageTarget(PrimaryTarget))
                {
                    CurrentCombatState = ECombat_AIState::Attacking;
                    CombatStateTimer = 0.0f;
                }
                else if (ShouldRetreat())
                {
                    CurrentCombatState = ECombat_AIState::Fleeing;
                    CombatStateTimer = 0.0f;
                }
            }
            break;

        case ECombat_AIState::Attacking:
            if (!PrimaryTarget || ShouldRetreat())
            {
                CurrentCombatState = ECombat_AIState::Fleeing;
                CombatStateTimer = 0.0f;
            }
            else if (GetDistanceToTarget(PrimaryTarget) > AttackRange * 1.5f)
            {
                CurrentCombatState = ECombat_AIState::Pursuing;
                CombatStateTimer = 0.0f;
            }
            break;

        case ECombat_AIState::Pursuing:
            if (PrimaryTarget)
            {
                float DistanceToTarget = GetDistanceToTarget(PrimaryTarget);
                if (DistanceToTarget < AttackRange)
                {
                    CurrentCombatState = ECombat_AIState::Attacking;
                    CombatStateTimer = 0.0f;
                }
                else if (DistanceToTarget > DetectionRange * 2.0f)
                {
                    CurrentCombatState = ECombat_AIState::Patrol;
                    PrimaryTarget = nullptr;
                    CombatStateTimer = 0.0f;
                }
            }
            break;

        case ECombat_AIState::Fleeing:
            if (CombatStateTimer > 5.0f) // Flee for 5 seconds
            {
                CurrentCombatState = ECombat_AIState::Patrol;
                CombatStateTimer = 0.0f;
            }
            break;
    }

    // Update blackboard
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        BlackboardComp->SetValueAsObject(TEXT("PrimaryTarget"), PrimaryTarget);
        BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), FearLevel);
    }
}

void UCombat_AIBehaviorTree::SetPrimaryTarget(AActor* NewTarget)
{
    PrimaryTarget = NewTarget;
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(TEXT("PrimaryTarget"), PrimaryTarget);
    }
}

bool UCombat_AIBehaviorTree::ShouldEngageTarget(AActor* Target)
{
    if (!Target || !OwnerPawn)
        return false;

    float DistanceToTarget = GetDistanceToTarget(Target);
    
    // Engage if close enough and aggression is high
    bool bShouldEngage = (DistanceToTarget < AttackRange) && 
                        (AggressionLevel > FearLevel) &&
                        (AggressionLevel > 0.4f);

    return bShouldEngage;
}

bool UCombat_AIBehaviorTree::ShouldRetreat()
{
    if (!PrimaryTarget || !OwnerPawn)
        return false;

    // Retreat if fear is higher than aggression
    bool bShouldRetreat = (FearLevel > AggressionLevel * 1.2f) || (FearLevel > 0.8f);
    
    return bShouldRetreat;
}

FVector UCombat_AIBehaviorTree::GetTacticalPosition()
{
    if (!OwnerPawn || !PrimaryTarget)
        return OwnerPawn ? OwnerPawn->GetActorLocation() : FVector::ZeroVector;

    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();

    // Tactical positioning based on intelligence and current state
    FVector TacticalOffset = FVector::ZeroVector;
    
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Attacking:
            // Move towards target but slightly to the side
            TacticalOffset = DirectionToTarget * AttackRange * 0.8f;
            TacticalOffset += FVector(DirectionToTarget.Y, -DirectionToTarget.X, 0) * 200.0f * TacticalIntelligence;
            break;
            
        case ECombat_AIState::Fleeing:
            // Move away from target
            TacticalOffset = -DirectionToTarget * DetectionRange * 0.5f;
            break;
            
        default:
            // Circle around target
            TacticalOffset = DirectionToTarget * AttackRange * 1.2f;
            break;
    }

    return OwnerLocation + TacticalOffset;
}

void UCombat_AIBehaviorTree::ExecuteCombatAction(ECombat_ActionType ActionType)
{
    if (!OwnerPawn)
        return;

    switch (ActionType)
    {
        case ECombat_ActionType::MeleeAttack:
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing melee attack"));
            // Trigger melee attack animation/logic
            break;
            
        case ECombat_ActionType::RangedAttack:
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing ranged attack"));
            // Trigger ranged attack logic
            break;
            
        case ECombat_ActionType::Block:
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing block"));
            // Trigger defensive stance
            break;
            
        case ECombat_ActionType::Dodge:
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing dodge"));
            // Trigger dodge movement
            break;
            
        case ECombat_ActionType::Intimidate:
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing intimidation"));
            // Play intimidation animation/sound
            break;
    }
}

void UCombat_AIBehaviorTree::CalculateAggressionLevel()
{
    if (!OwnerPawn)
        return;

    // Base aggression modified by health, distance to target, and time in combat
    float BaseAggression = 0.5f;
    
    // Health factor - lower health = lower aggression
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn))
    {
        // Assuming health component exists
        BaseAggression *= 1.0f; // Placeholder for health ratio
    }
    
    // Distance factor - closer targets = higher aggression
    if (PrimaryTarget)
    {
        float Distance = GetDistanceToTarget(PrimaryTarget);
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / DetectionRange), 0.2f, 1.0f);
        BaseAggression *= DistanceFactor;
    }
    
    // Combat time factor - longer combat = lower aggression
    float CombatTimeFactor = FMath::Clamp(1.0f - (CombatStateTimer / 30.0f), 0.3f, 1.0f);
    BaseAggression *= CombatTimeFactor;
    
    AggressionLevel = FMath::Clamp(BaseAggression, 0.0f, 1.0f);
}

void UCombat_AIBehaviorTree::CalculateFearLevel()
{
    if (!OwnerPawn)
        return;

    // Base fear modified by target threat level and own health
    float BaseFear = 0.3f;
    
    // Target size/threat factor
    if (PrimaryTarget)
    {
        // Larger targets = more fear
        FVector TargetScale = PrimaryTarget->GetActorScale3D();
        float ThreatFactor = FMath::Clamp(TargetScale.Size() / 3.0f, 0.5f, 2.0f);
        BaseFear *= ThreatFactor;
    }
    
    // Multiple targets = more fear
    if (SecondaryTargets.Num() > 0)
    {
        BaseFear *= (1.0f + SecondaryTargets.Num() * 0.2f);
    }
    
    FearLevel = FMath::Clamp(BaseFear, 0.0f, 1.0f);
}

void UCombat_AIBehaviorTree::UpdateTargetPriorities()
{
    if (!OwnerPawn)
        return;

    // Find all potential targets in range
    TArray<AActor*> PotentialTargets;
    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
        return;

    // Get all pawns in detection range
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor == OwnerPawn)
            continue;
            
        float Distance = GetDistanceToTarget(Actor);
        if (Distance < DetectionRange && IsTargetVisible(Actor))
        {
            PotentialTargets.Add(Actor);
        }
    }
    
    // Sort by priority (distance and threat level)
    PotentialTargets.Sort([this](const AActor& A, const AActor& B) {
        float DistanceA = GetDistanceToTarget(&A);
        float DistanceB = GetDistanceToTarget(&B);
        return DistanceA < DistanceB;
    });
    
    // Update primary and secondary targets
    if (PotentialTargets.Num() > 0)
    {
        if (!PrimaryTarget || !PotentialTargets.Contains(PrimaryTarget))
        {
            SetPrimaryTarget(PotentialTargets[0]);
        }
        
        SecondaryTargets.Empty();
        for (int32 i = 1; i < FMath::Min(PotentialTargets.Num(), 4); i++)
        {
            SecondaryTargets.Add(PotentialTargets[i]);
        }
    }
    else if (PrimaryTarget && GetDistanceToTarget(PrimaryTarget) > DetectionRange * 1.5f)
    {
        PrimaryTarget = nullptr;
        SecondaryTargets.Empty();
    }
}

bool UCombat_AIBehaviorTree::IsTargetVisible(AActor* Target)
{
    if (!OwnerPawn || !Target)
        return false;

    // Simple line trace for visibility
    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
        return false;

    FVector StartLocation = OwnerPawn->GetActorLocation();
    FVector EndLocation = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Target is visible if no obstacles hit
}

float UCombat_AIBehaviorTree::GetDistanceToTarget(AActor* Target)
{
    if (!OwnerPawn || !Target)
        return 999999.0f;

    return FVector::Dist(OwnerPawn->GetActorLocation(), Target->GetActorLocation());
}