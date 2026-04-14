#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Setup Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Setup Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Blackboard
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize state
    CurrentState = ECombat_AIState::Idle;
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    TimeSinceLastTargetSeen = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;

    // Initialize timers
    LastTacticalEvaluation = 0.0f;
    TacticalEvaluationInterval = 0.5f; // Evaluate tactics twice per second
    LastPackCommunication = 0.0f;
    PackCommunicationInterval = 1.0f; // Communicate with pack once per second
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }

    // Initialize blackboard values
    UpdateBlackboard();
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastTargetSeen += DeltaTime;

    // Periodic tactical evaluation
    if (GetWorld()->GetTimeSeconds() - LastTacticalEvaluation > TacticalEvaluationInterval)
    {
        EvaluateTacticalSituation();
        LastTacticalEvaluation = GetWorld()->GetTimeSeconds();
    }

    // Periodic pack communication
    if (GetWorld()->GetTimeSeconds() - LastPackCommunication > PackCommunicationInterval)
    {
        ProcessPackCommunication();
        LastPackCommunication = GetWorld()->GetTimeSeconds();
    }

    UpdateBlackboard();
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_AIState OldState = CurrentState;
        CurrentState = NewState;

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("AI %s: State changed from %d to %d"), 
               *GetPawn()->GetName(), (int32)OldState, (int32)NewState);

        // Handle state-specific logic
        switch (NewState)
        {
        case ECombat_AIState::Attacking:
            if (bIsPackLeader && PackMembers.Num() > 0)
            {
                CoordinatePackAttack();
            }
            break;
        case ECombat_AIState::Retreating:
            if (bIsPackLeader)
            {
                // Signal pack to retreat
                for (ACombatAIController* Member : PackMembers)
                {
                    if (Member && Member != this)
                    {
                        Member->SetAIState(ECombat_AIState::Retreating);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            TimeSinceLastTargetSeen = 0.0f;
            
            // Notify pack of new target
            if (bIsPackLeader)
            {
                CallPackToTarget(NewTarget);
            }
        }
    }
}

void ACombatAIController::LoseTarget()
{
    CurrentTarget = nullptr;
    SetAIState(ECombat_AIState::Investigating);
}

bool ACombatAIController::ShouldAttack() const
{
    if (!CurrentTarget || CurrentState == ECombat_AIState::Dead)
    {
        return false;
    }

    float DistanceToTarget = GetDistanceToTarget();
    bool bInRange = DistanceToTarget <= TacticalData.PreferredAttackDistance;
    bool bVisible = IsTargetVisible();
    bool bAggressive = TacticalData.AggressionLevel > 0.3f;

    return bInRange && bVisible && bAggressive;
}

bool ACombatAIController::ShouldFlank() const
{
    if (!TacticalData.bCanFlank || !CurrentTarget)
    {
        return false;
    }

    float DistanceToTarget = GetDistanceToTarget();
    bool bTooClose = DistanceToTarget < TacticalData.PreferredAttackDistance * 0.5f;
    bool bCanFlankFromDistance = DistanceToTarget < TacticalData.FlankingRadius;
    bool bTactical = TacticalData.CautiousLevel > 0.4f;

    return !bTooClose && bCanFlankFromDistance && bTactical;
}

bool ACombatAIController::ShouldRetreat() const
{
    if (!TacticalData.bCanRetreat || !CurrentTarget)
    {
        return false;
    }

    // Check health (assuming pawn has health component)
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return true;
    }

    // Simple health check - in real implementation, check actual health component
    float HealthPercentage = 1.0f; // Placeholder
    bool bLowHealth = HealthPercentage < 0.3f;
    
    // Check if outnumbered
    bool bOutnumbered = false;
    if (PackMembers.Num() == 0) // Lone wolf
    {
        bOutnumbered = true;
    }

    bool bHighCaution = TacticalData.CautiousLevel > 0.6f;

    return (bLowHealth || bOutnumbered) && bHighCaution;
}

bool ACombatAIController::ShouldCallForHelp() const
{
    if (!TacticalData.bCanCallForHelp || !CurrentTarget)
    {
        return false;
    }

    bool bHasNearbyAllies = PackMembers.Num() > 0;
    bool bInCombat = CurrentState == ECombat_AIState::Attacking || CurrentState == ECombat_AIState::Chasing;
    float DistanceToTarget = GetDistanceToTarget();
    bool bTargetClose = DistanceToTarget < TacticalData.PreferredAttackDistance * 1.5f;

    return bHasNearbyAllies && bInCombat && bTargetClose;
}

FVector ACombatAIController::GetFlankingPosition() const
{
    if (!CurrentTarget)
    {
        return GetPawn()->GetActorLocation();
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the side
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomize left or right flanking
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    FVector FlankPosition = TargetLocation + (FlankDirection * TacticalData.FlankingRadius);
    
    // Try to find navigable position
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavResult;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavResult, FVector(500.0f)))
        {
            return NavResult.Location;
        }
    }
    
    return FlankPosition;
}

FVector ACombatAIController::GetRetreatPosition() const
{
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector RetreatDirection = FVector::ZeroVector;
    
    if (CurrentTarget)
    {
        // Retreat away from target
        RetreatDirection = (MyLocation - CurrentTarget->GetActorLocation()).GetSafeNormal();
    }
    else
    {
        // Random retreat direction
        RetreatDirection = FMath::VRand();
        RetreatDirection.Z = 0.0f;
        RetreatDirection.Normalize();
    }
    
    FVector RetreatPosition = MyLocation + (RetreatDirection * 1000.0f);
    
    // Try to find navigable position
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavResult;
        if (NavSys->ProjectPointToNavigation(RetreatPosition, NavResult, FVector(500.0f)))
        {
            return NavResult.Location;
        }
    }
    
    return RetreatPosition;
}

void ACombatAIController::JoinPack(ACombatAIController* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        
        // Add self to leader's pack
        Leader->PackMembers.AddUnique(this);
        
        // Leave current pack if any
        if (PackMembers.Num() > 0)
        {
            for (ACombatAIController* Member : PackMembers)
            {
                if (Member)
                {
                    Member->PackLeader = nullptr;
                }
            }
            PackMembers.Empty();
        }
    }
}

void ACombatAIController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    if (bIsPackLeader)
    {
        // Promote a new leader
        if (PackMembers.Num() > 0)
        {
            ACombatAIController* NewLeader = PackMembers[0];
            NewLeader->BecomePackLeader();
            
            for (int32 i = 1; i < PackMembers.Num(); i++)
            {
                if (PackMembers[i])
                {
                    PackMembers[i]->JoinPack(NewLeader);
                }
            }
        }
        
        PackMembers.Empty();
        bIsPackLeader = false;
    }
}

void ACombatAIController::BecomePackLeader()
{
    bIsPackLeader = true;
    PackLeader = nullptr;
    
    // Ensure tactical data supports leadership
    if (TacticalData.Role == ECombat_TacticalRole::Pack_Member)
    {
        TacticalData.Role = ECombat_TacticalRole::Pack_Leader;
    }
}

void ACombatAIController::CallPackToTarget(AActor* Target)
{
    if (!bIsPackLeader || !Target)
    {
        return;
    }
    
    for (ACombatAIController* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->SetTarget(Target);
            Member->SetAIState(ECombat_AIState::Chasing);
        }
    }
}

void ACombatAIController::CoordinatePackAttack()
{
    if (!bIsPackLeader || PackMembers.Num() == 0 || !CurrentTarget)
    {
        return;
    }
    
    // Assign roles based on pack size and member capabilities
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ACombatAIController* Member = PackMembers[i];
        if (!Member) continue;
        
        if (i % 2 == 0 && Member->TacticalData.bCanFlank)
        {
            // Flanking members
            Member->SetAIState(ECombat_AIState::Flanking);
        }
        else
        {
            // Direct attackers
            Member->SetAIState(ECombat_AIState::Attacking);
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;
        
        // Check if this is a potential target (player or enemy)
        if (Actor->IsA<APawn>() && Actor != GetPawn())
        {
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    // New target spotted
                    if (!CurrentTarget || CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrolling)
                    {
                        SetTarget(Actor);
                        SetAIState(ECombat_AIState::Investigating);
                    }
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == CurrentTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            LastKnownTargetLocation = Actor->GetActorLocation();
            TimeSinceLastTargetSeen = 0.0f;
        }
        else
        {
            // Lost sight of target
            if (TimeSinceLastTargetSeen > 5.0f) // Lost for 5 seconds
            {
                LoseTarget();
            }
        }
    }
}

void ACombatAIController::UpdateBlackboard()
{
    if (!Blackboard) return;
    
    // Update blackboard with current state
    Blackboard->SetValueAsEnum(TEXT("AIState"), (uint8)CurrentState);
    
    if (CurrentTarget)
    {
        Blackboard->SetValueAsObject(TEXT("Target"), CurrentTarget);
        Blackboard->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        Blackboard->SetValueAsObject(TEXT("Target"), nullptr);
    }
    
    Blackboard->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
    Blackboard->SetValueAsBool(TEXT("HasTarget"), CurrentTarget != nullptr);
    Blackboard->SetValueAsBool(TEXT("ShouldAttack"), ShouldAttack());
    Blackboard->SetValueAsBool(TEXT("ShouldFlank"), ShouldFlank());
    Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), ShouldRetreat());
    Blackboard->SetValueAsVector(TEXT("FlankingPosition"), GetFlankingPosition());
    Blackboard->SetValueAsVector(TEXT("RetreatPosition"), GetRetreatPosition());
    Blackboard->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    Blackboard->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
}

void ACombatAIController::EvaluateTacticalSituation()
{
    if (!CurrentTarget) return;
    
    // Evaluate current tactical situation and adjust state accordingly
    if (CurrentState == ECombat_AIState::Chasing)
    {
        if (ShouldAttack())
        {
            SetAIState(ECombat_AIState::Attacking);
        }
        else if (ShouldFlank())
        {
            SetAIState(ECombat_AIState::Flanking);
        }
        else if (ShouldRetreat())
        {
            SetAIState(ECombat_AIState::Retreating);
        }
    }
    else if (CurrentState == ECombat_AIState::Attacking)
    {
        if (ShouldRetreat())
        {
            SetAIState(ECombat_AIState::Retreating);
        }
        else if (!IsInAttackRange())
        {
            SetAIState(ECombat_AIState::Chasing);
        }
    }
    else if (CurrentState == ECombat_AIState::Flanking)
    {
        if (ShouldAttack())
        {
            SetAIState(ECombat_AIState::Attacking);
        }
        else if (ShouldRetreat())
        {
            SetAIState(ECombat_AIState::Retreating);
        }
    }
}

void ACombatAIController::ProcessPackCommunication()
{
    if (!bIsPackLeader) return;
    
    // Update pack formation and coordination
    UpdatePackFormation();
    
    // Check if pack should change tactics
    if (CurrentTarget && PackMembers.Num() > 1)
    {
        bool bAllMembersEngaged = true;
        for (ACombatAIController* Member : PackMembers)
        {
            if (Member && Member->CurrentTarget != CurrentTarget)
            {
                bAllMembersEngaged = false;
                break;
            }
        }
        
        if (!bAllMembersEngaged)
        {
            CallPackToTarget(CurrentTarget);
        }
    }
}

bool ACombatAIController::IsInAttackRange() const
{
    return GetDistanceToTarget() <= TacticalData.PreferredAttackDistance;
}

bool ACombatAIController::IsTargetVisible() const
{
    if (!CurrentTarget || !AIPerceptionComponent) return false;
    
    FAIStimulus Stimulus;
    if (AIPerceptionComponent->GetActorsPerception(CurrentTarget, Stimulus))
    {
        return Stimulus.WasSuccessfullySensed();
    }
    
    return false;
}

float ACombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return FLT_MAX;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ACombatAIController::UpdatePackFormation()
{
    if (!bIsPackLeader || PackMembers.Num() == 0) return;
    
    // Simple formation update - could be expanded with more complex formations
    FVector LeaderLocation = GetPawn()->GetActorLocation();
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ACombatAIController* Member = PackMembers[i];
        if (!Member) continue;
        
        // Calculate formation position relative to leader
        float Angle = (360.0f / PackMembers.Num()) * i;
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * 300.0f,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * 300.0f,
            0.0f
        );
        
        FVector FormationPosition = LeaderLocation + Offset;
        
        // Update member's blackboard with formation position
        if (Member->Blackboard)
        {
            Member->Blackboard->SetValueAsVector(TEXT("FormationPosition"), FormationPosition);
        }
    }
}