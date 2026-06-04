#include "Combat_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize Blackboard and Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Initialize combat parameters
    AttackRange = 300.0f;
    DetectionRange = 1500.0f;
    Aggressiveness = 70.0f;
    AIPersonality = ECombat_AIPersonality::Aggressive;
    CurrentCombatState = ECombat_AIState::Patrol;
    
    // Initialize pack behavior
    bIsPackHunter = false;
    PackLeader = nullptr;
    
    // Initialize timers
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
    ThreatAssessmentTimer = 0.0f;
    
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }
    
    // Start behavior tree if assigned
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
        SetCombatState(ECombat_AIState::Patrol);
    }
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatBehavior(DeltaTime);
    
    if (bIsPackHunter)
    {
        UpdatePackCoordination();
    }
}

void ACombat_AIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombat_AIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            SetCombatState(ECombat_AIState::Combat);
        }
        else
        {
            SetCombatState(ECombat_AIState::Patrol);
        }
    }
}

bool ACombat_AIController::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ACombat_AIController::ExecuteAttack()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return;
    }
    
    if (IsInAttackRange(CurrentTarget))
    {
        LastAttackTime = CurrentTime;
        
        // Trigger attack animation/behavior
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("ShouldAttack"), true);
        }
        
        // Coordinate pack attack if pack hunter
        if (bIsPackHunter && PackLeader == this)
        {
            CoordinatePackAttack(CurrentTarget);
        }
    }
}

void ACombat_AIController::JoinPack(ACombat_AIController* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        Leader->AddPackMember(this);
        bIsPackHunter = true;
    }
}

void ACombat_AIController::AddPackMember(ACombat_AIController* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        Member->PackLeader = this;
        Member->bIsPackHunter = true;
    }
}

void ACombat_AIController::CoordinatePackAttack(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    // Set target for all pack members
    for (ACombat_AIController* Member : PackMembers)
    {
        if (Member)
        {
            Member->SetTarget(Target);
            Member->SetCombatState(ECombat_AIState::Combat);
        }
    }
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a potential target (player character)
            if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
            {
                SetTarget(Actor);
                break;
            }
        }
    }
}

void ACombat_AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (Actor->IsA<APawn>() && (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player"))))
        {
            SetTarget(Actor);
        }
    }
    else if (Actor == CurrentTarget)
    {
        // Target lost
        LastKnownTargetLocation = Actor->GetActorLocation();
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
        }
        SetCombatState(ECombat_AIState::Searching);
    }
}

void ACombat_AIController::UpdateCombatBehavior(float DeltaTime)
{
    ThreatAssessmentTimer += DeltaTime;
    
    if (ThreatAssessmentTimer >= 1.0f) // Assess threats every second
    {
        ProcessThreatAssessment();
        ThreatAssessmentTimer = 0.0f;
    }
    
    // Execute tactical movement based on current state
    ExecuteTacticalMovement();
}

void ACombat_AIController::ProcessThreatAssessment()
{
    if (!GetPawn())
    {
        return;
    }
    
    // Evaluate current target threat level
    if (CurrentTarget)
    {
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (DistanceToTarget > DetectionRange * 1.5f)
        {
            // Target too far, return to patrol
            SetTarget(nullptr);
        }
        else if (DistanceToTarget <= AttackRange && CurrentCombatState != ECombat_AIState::Combat)
        {
            SetCombatState(ECombat_AIState::Combat);
        }
    }
}

void ACombat_AIController::ExecuteTacticalMovement()
{
    if (!CurrentTarget || !GetPawn() || !BlackboardComponent)
    {
        return;
    }
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Combat:
            if (bIsPackHunter && PackLeader != this)
            {
                // Pack member: execute flanking maneuver
                FVector FlankPosition = CalculateFlankingPosition(TargetLocation);
                BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), FlankPosition);
            }
            else
            {
                // Direct approach for solo hunters or pack leaders
                BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), TargetLocation);
            }
            break;
            
        case ECombat_AIState::Searching:
            // Move to last known target location
            BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), LastKnownTargetLocation);
            break;
            
        default:
            break;
    }
}

FVector ACombat_AIController::CalculateFlankingPosition(const FVector& TargetLocation)
{
    if (!GetPawn())
    {
        return TargetLocation;
    }
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the right or left
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f; // Random left or right flank
    }
    
    return TargetLocation + (FlankDirection * AttackRange * 0.8f);
}

void ACombat_AIController::UpdatePackCoordination()
{
    if (!bIsPackHunter)
    {
        return;
    }
    
    // Pack leader coordinates the hunt
    if (PackLeader == this && CurrentTarget)
    {
        // Ensure all pack members are targeting the same enemy
        for (ACombat_AIController* Member : PackMembers)
        {
            if (Member && Member->CurrentTarget != CurrentTarget)
            {
                Member->SetTarget(CurrentTarget);
            }
        }
    }
    
    // Pack member follows leader's commands
    if (PackLeader && PackLeader != this)
    {
        if (PackLeader->CurrentTarget && !CurrentTarget)
        {
            SetTarget(PackLeader->CurrentTarget);
        }
    }
}