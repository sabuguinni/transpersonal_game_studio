#include "Combat_TacticalAIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Combat_ThreatAssessmentSystem.h"
#include "NPC/NPC_DinosaurBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

const float ACombat_TacticalAIController::STATE_UPDATE_INTERVAL = 0.5f;
const float ACombat_TacticalAIController::THREAT_ASSESSMENT_INTERVAL = 0.2f;
const float ACombat_TacticalAIController::PACK_COORDINATION_INTERVAL = 1.0f;

ACombat_TacticalAIController::ACombat_TacticalAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    ThreatAssessment = CreateDefaultSubobject<UCombat_ThreatAssessmentSystem>(TEXT("ThreatAssessment"));

    // Configure AI Perception - Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure AI Perception - Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize tactical data
    TacticalData = FCombat_TacticalData();
    
    // Initialize timers
    StateUpdateTimer = 0.0f;
    ThreatAssessmentTimer = 0.0f;
    PackCoordinationTimer = 0.0f;
}

void ACombat_TacticalAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if available
    if (CombatBehaviorTree && CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Initialize blackboard values
    UpdateBlackboardValues();

    UE_LOG(LogTemp, Log, TEXT("Combat Tactical AI Controller initialized"));
}

void ACombat_TacticalAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update timers
    StateUpdateTimer += DeltaTime;
    ThreatAssessmentTimer += DeltaTime;
    PackCoordinationTimer += DeltaTime;

    // Update tactical state
    if (StateUpdateTimer >= STATE_UPDATE_INTERVAL)
    {
        UpdateTacticalState(DeltaTime);
        StateUpdateTimer = 0.0f;
    }

    // Process threat assessment
    if (ThreatAssessmentTimer >= THREAT_ASSESSMENT_INTERVAL)
    {
        ProcessThreatAssessment();
        ThreatAssessmentTimer = 0.0f;
    }

    // Coordinate pack behavior
    if (PackCoordinationTimer >= PACK_COORDINATION_INTERVAL)
    {
        CoordinatePackBehavior();
        PackCoordinationTimer = 0.0f;
    }
}

void ACombat_TacticalAIController::SetTacticalState(ECombat_TacticalState NewState)
{
    if (TacticalData.CurrentState != NewState)
    {
        ECombat_TacticalState PreviousState = TacticalData.CurrentState;
        TacticalData.CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Tactical state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), (uint8)NewState);
        }

        // Broadcast to pack if leader
        if (TacticalData.bIsPackLeader)
        {
            BroadcastTacticalCommand(FString::Printf(TEXT("State change to %d"), (int32)NewState));
        }
    }
}

ECombat_TacticalState ACombat_TacticalAIController::GetTacticalState() const
{
    return TacticalData.CurrentState;
}

void ACombat_TacticalAIController::SetFormation(ECombat_Formation NewFormation)
{
    TacticalData.Formation = NewFormation;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("Formation"), (uint8)NewFormation);
    }
}

void ACombat_TacticalAIController::SetPrimaryTarget(AActor* Target)
{
    TacticalData.PrimaryTarget = Target;
    
    if (Target)
    {
        TacticalData.LastKnownTargetLocation = Target->GetActorLocation();
    }
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("PrimaryTarget"), Target);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), TacticalData.LastKnownTargetLocation);
    }
}

AActor* ACombat_TacticalAIController::GetPrimaryTarget() const
{
    return TacticalData.PrimaryTarget;
}

void ACombat_TacticalAIController::AddPackMember(AActor* Member)
{
    if (Member && !TacticalData.PackMembers.Contains(Member))
    {
        TacticalData.PackMembers.Add(Member);
        UE_LOG(LogTemp, Log, TEXT("Pack member added: %s"), *Member->GetName());
    }
}

void ACombat_TacticalAIController::RemovePackMember(AActor* Member)
{
    if (Member)
    {
        TacticalData.PackMembers.Remove(Member);
        UE_LOG(LogTemp, Log, TEXT("Pack member removed: %s"), *Member->GetName());
    }
}

TArray<AActor*> ACombat_TacticalAIController::GetPackMembers() const
{
    return TacticalData.PackMembers;
}

bool ACombat_TacticalAIController::IsPackLeader() const
{
    return TacticalData.bIsPackLeader;
}

void ACombat_TacticalAIController::SetPackLeader(bool bLeader)
{
    TacticalData.bIsPackLeader = bLeader;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), bLeader);
    }
}

void ACombat_TacticalAIController::InitiateHunt(AActor* Target)
{
    SetPrimaryTarget(Target);
    SetTacticalState(ECombat_TacticalState::Hunt);
    
    if (TacticalData.bIsPackLeader)
    {
        SetFormation(ECombat_Formation::Wedge);
        BroadcastTacticalCommand(TEXT("Initiate hunt"));
    }
}

void ACombat_TacticalAIController::ExecuteFlankingManeuver()
{
    if (TacticalData.PrimaryTarget)
    {
        SetTacticalState(ECombat_TacticalState::Flank);
        
        FVector FlankPosition = CalculateFlankingPosition(TacticalData.PrimaryTarget);
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("FlankingPosition"), FlankPosition);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Executing flanking maneuver"));
    }
}

void ACombat_TacticalAIController::OrderRetreat()
{
    SetTacticalState(ECombat_TacticalState::Retreat);
    
    if (TacticalData.bIsPackLeader)
    {
        BroadcastTacticalCommand(TEXT("Retreat ordered"));
    }
}

void ACombat_TacticalAIController::RegroupPack()
{
    SetTacticalState(ECombat_TacticalState::Regroup);
    
    if (TacticalData.bIsPackLeader)
    {
        SetFormation(ECombat_Formation::Circle);
        BroadcastTacticalCommand(TEXT("Regroup"));
    }
}

void ACombat_TacticalAIController::SetupAmbush(const FVector& AmbushLocation)
{
    SetTacticalState(ECombat_TacticalState::Ambush);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("AmbushLocation"), AmbushLocation);
    }
    
    if (TacticalData.bIsPackLeader)
    {
        SetFormation(ECombat_Formation::Ambush);
        BroadcastTacticalCommand(TEXT("Setup ambush"));
    }
}

void ACombat_TacticalAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Check if this is a potential target
            if (Actor->IsA<APawn>() && !TacticalData.PackMembers.Contains(Actor))
            {
                if (!TacticalData.PrimaryTarget)
                {
                    SetPrimaryTarget(Actor);
                    InitiateHunt(Actor);
                }
            }
        }
    }
}

void ACombat_TacticalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == TacticalData.PrimaryTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            TacticalData.LastKnownTargetLocation = Actor->GetActorLocation();
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), TacticalData.LastKnownTargetLocation);
            }
        }
    }
}

void ACombat_TacticalAIController::UpdateTacticalState(float DeltaTime)
{
    if (!GetPawn()) return;

    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Patrol:
            // Look for targets
            if (TacticalData.PrimaryTarget)
            {
                InitiateHunt(TacticalData.PrimaryTarget);
            }
            break;

        case ECombat_TacticalState::Hunt:
            // Check if target is in engagement range
            if (TacticalData.PrimaryTarget)
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TacticalData.PrimaryTarget->GetActorLocation());
                if (Distance <= TacticalData.EngagementRange)
                {
                    SetTacticalState(ECombat_TacticalState::Engage);
                }
            }
            break;

        case ECombat_TacticalState::Engage:
            // Check if should retreat or flank
            if (ShouldRetreat())
            {
                OrderRetreat();
            }
            else if (TacticalData.PackMembers.Num() > 1)
            {
                ExecuteFlankingManeuver();
            }
            break;

        case ECombat_TacticalState::Retreat:
            // Check if safe to regroup
            if (TacticalData.PrimaryTarget)
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TacticalData.PrimaryTarget->GetActorLocation());
                if (Distance > TacticalData.EngagementRange * 2.0f)
                {
                    RegroupPack();
                }
            }
            break;
    }
}

void ACombat_TacticalAIController::ProcessThreatAssessment()
{
    if (ThreatAssessment && TacticalData.PrimaryTarget)
    {
        // Use threat assessment system to evaluate current situation
        // This would integrate with the Combat_ThreatAssessmentSystem
    }
}

void ACombat_TacticalAIController::CoordinatePackBehavior()
{
    if (TacticalData.bIsPackLeader && TacticalData.PackMembers.Num() > 0)
    {
        // Coordinate pack formation and tactics
        for (AActor* Member : TacticalData.PackMembers)
        {
            if (Member)
            {
                // Send formation commands to pack members
                // This would be implemented with a pack communication system
            }
        }
    }
}

void ACombat_TacticalAIController::UpdateBlackboardValues()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), (uint8)TacticalData.CurrentState);
        BlackboardComponent->SetValueAsEnum(TEXT("Formation"), (uint8)TacticalData.Formation);
        BlackboardComponent->SetValueAsObject(TEXT("PrimaryTarget"), TacticalData.PrimaryTarget);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), TacticalData.LastKnownTargetLocation);
        BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), TacticalData.bIsPackLeader);
        BlackboardComponent->SetValueAsFloat(TEXT("EngagementRange"), TacticalData.EngagementRange);
    }
}

FVector ACombat_TacticalAIController::CalculateFlankingPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the right
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (FlankDirection * TacticalData.FlankingDistance);
    
    return FlankPosition;
}

bool ACombat_TacticalAIController::ShouldRetreat() const
{
    // Check health, pack status, and threat level
    if (GetPawn())
    {
        // This would integrate with health/damage system
        // For now, simple distance check
        if (TacticalData.PrimaryTarget)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TacticalData.PrimaryTarget->GetActorLocation());
            return Distance < 300.0f && TacticalData.PackMembers.Num() < 2;
        }
    }
    return false;
}

void ACombat_TacticalAIController::BroadcastTacticalCommand(const FString& Command)
{
    UE_LOG(LogTemp, Log, TEXT("Tactical Command: %s"), *Command);
    
    // This would broadcast to pack members via a communication system
    for (AActor* Member : TacticalData.PackMembers)
    {
        if (Member)
        {
            // Send command to pack member
        }
    }
}