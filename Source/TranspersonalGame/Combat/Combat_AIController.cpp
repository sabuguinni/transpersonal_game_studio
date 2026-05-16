#include "Combat_AIController.h"
#include "Combat_ThreatAssessment.h"
#include "Combat_TacticalAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    // Initialize Combat Systems
    ThreatAssessment = CreateDefaultSubobject<UCombat_ThreatAssessment>(TEXT("ThreatAssessment"));
    TacticalAI = CreateDefaultSubobject<UCombat_TacticalAI>(TEXT("TacticalAI"));

    // Configure AI Perception - Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        PerceptionComp->ConfigureSense(*SightConfig);
    }

    // Configure AI Perception - Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 2000.0f;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        PerceptionComp->ConfigureSense(*HearingConfig);
    }

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Default Values
    DinosaurSpecies = ECombat_DinosaurSpecies::Velociraptor;
    CurrentAIState = ECombat_AIState::Patrol;
    AggressionLevel = 0.5f;
    TerritorialRadius = 1500.0f;
    bIsPackHunter = true;
    bIsApexPredator = false;
    bIsPackLeader = false;
    PackLeader = nullptr;

    // Initialize Timers
    LastAttackTime = 0.0f;
    LastThreatAssessmentTime = 0.0f;
    CombatCooldownTime = 3.0f;

    // Initialize Blackboard Keys
    TargetActorKey = FName("TargetActor");
    AIStateKey = FName("AIState");
    ThreatLevelKey = FName("ThreatLevel");
    PackPositionKey = FName("PackPosition");
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind Perception Events
    if (PerceptionComp)
    {
        PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }

    // Start Behavior Tree if available
    if (BehaviorTreeAsset && BlackboardComp)
    {
        RunBehaviorTree(BehaviorTreeAsset);
        
        // Initialize Blackboard values
        BlackboardComp->SetValueAsEnum(AIStateKey, static_cast<uint8>(CurrentAIState));
        BlackboardComp->SetValueAsFloat(ThreatLevelKey, 0.0f);
    }

    // Configure species-specific behavior
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TyrannosaurusRex:
            AggressionLevel = 0.9f;
            TerritorialRadius = 3000.0f;
            bIsPackHunter = false;
            bIsApexPredator = true;
            break;
            
        case ECombat_DinosaurSpecies::Velociraptor:
            AggressionLevel = 0.8f;
            TerritorialRadius = 1200.0f;
            bIsPackHunter = true;
            bIsApexPredator = false;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            AggressionLevel = 0.3f;
            TerritorialRadius = 800.0f;
            bIsPackHunter = false;
            bIsApexPredator = false;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            AggressionLevel = 0.1f;
            TerritorialRadius = 500.0f;
            bIsPackHunter = false;
            bIsApexPredator = false;
            break;
    }
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAIBehavior(DeltaTime);
}

void ACombat_AIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentAIState != NewState)
    {
        CurrentAIState = NewState;
        
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(AIStateKey, static_cast<uint8>(NewState));
        }

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("AI Controller %s changed state to %d"), 
               *GetName(), static_cast<int32>(NewState));
    }
}

void ACombat_AIController::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || !ThreatAssessment)
    {
        return;
    }

    float ThreatLevel = ThreatAssessment->CalculateThreatLevel(PotentialThreat, GetPawn());
    
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsFloat(ThreatLevelKey, ThreatLevel);
        
        if (ThreatLevel > 0.7f)
        {
            BlackboardComp->SetValueAsObject(TargetActorKey, PotentialThreat);
            SetAIState(ECombat_AIState::Combat);
        }
        else if (ThreatLevel > 0.3f)
        {
            SetAIState(ECombat_AIState::Alert);
        }
    }
}

float ACombat_AIController::GetThreatLevel(AActor* Target)
{
    if (!Target || !ThreatAssessment)
    {
        return 0.0f;
    }

    return ThreatAssessment->CalculateThreatLevel(Target, GetPawn());
}

void ACombat_AIController::JoinPack(ACombat_AIController* PackLeaderController)
{
    if (!PackLeaderController || PackLeaderController == this)
    {
        return;
    }

    // Leave current pack if in one
    LeavePack();

    // Join new pack
    PackLeader = PackLeaderController;
    PackLeaderController->PackMembers.AddUnique(this);
    
    UE_LOG(LogTemp, Log, TEXT("AI Controller %s joined pack led by %s"), 
           *GetName(), *PackLeaderController->GetName());
}

void ACombat_AIController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }

    if (bIsPackLeader)
    {
        // Disband pack
        for (ACombat_AIController* Member : PackMembers)
        {
            if (Member)
            {
                Member->PackLeader = nullptr;
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }
}

bool ACombat_AIController::IsInPack() const
{
    return PackLeader != nullptr || bIsPackLeader;
}

void ACombat_AIController::InitiateAttack(AActor* Target)
{
    if (!Target || !GetPawn())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < CombatCooldownTime)
    {
        return; // Still in cooldown
    }

    LastAttackTime = CurrentTime;
    SetAIState(ECombat_AIState::Combat);

    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(TargetActorKey, Target);
    }

    // Coordinate pack attack if pack leader
    if (bIsPackLeader && bIsPackHunter)
    {
        for (ACombat_AIController* Member : PackMembers)
        {
            if (Member && Member->GetPawn())
            {
                Member->InitiateAttack(Target);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AI Controller %s initiating attack on %s"), 
           *GetName(), *Target->GetName());
}

void ACombat_AIController::FlankTarget(AActor* Target, FVector FlankPosition)
{
    if (!Target || !GetPawn())
    {
        return;
    }

    // Move to flank position
    MoveToLocation(FlankPosition, 100.0f);
    
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(PackPositionKey, FlankPosition);
    }

    UE_LOG(LogTemp, Log, TEXT("AI Controller %s flanking target at position %s"), 
           *GetName(), *FlankPosition.ToString());
}

void ACombat_AIController::RetreatFromCombat()
{
    SetAIState(ECombat_AIState::Flee);
    
    if (BlackboardComp)
    {
        BlackboardComp->ClearValue(TargetActorKey);
        BlackboardComp->SetValueAsFloat(ThreatLevelKey, 0.0f);
    }

    // Signal pack to retreat if leader
    if (bIsPackLeader)
    {
        for (ACombat_AIController* Member : PackMembers)
        {
            if (Member)
            {
                Member->RetreatFromCombat();
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("AI Controller %s retreating from combat"), *GetName());
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            AssessThreat(Actor);
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
        AssessThreat(Actor);
    }
    else
    {
        // Target lost
        if (BlackboardComp && BlackboardComp->GetValueAsObject(TargetActorKey) == Actor)
        {
            BlackboardComp->ClearValue(TargetActorKey);
            SetAIState(ECombat_AIState::Patrol);
        }
    }
}

void ACombat_AIController::UpdateAIBehavior(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Periodic threat assessment
    if (CurrentTime - LastThreatAssessmentTime > 2.0f)
    {
        ProcessThreatAssessment();
        LastThreatAssessmentTime = CurrentTime;
    }

    // Execute tactical decisions
    if (TacticalAI)
    {
        ExecuteTacticalDecision();
    }
}

void ACombat_AIController::ProcessThreatAssessment()
{
    if (!PerceptionComp)
    {
        return;
    }

    TArray<AActor*> PerceivedActors;
    PerceptionComp->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    float HighestThreat = 0.0f;
    AActor* HighestThreatActor = nullptr;

    for (AActor* Actor : PerceivedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            float ThreatLevel = GetThreatLevel(Actor);
            if (ThreatLevel > HighestThreat)
            {
                HighestThreat = ThreatLevel;
                HighestThreatActor = Actor;
            }
        }
    }

    if (HighestThreatActor && HighestThreat > 0.5f)
    {
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsObject(TargetActorKey, HighestThreatActor);
            BlackboardComp->SetValueAsFloat(ThreatLevelKey, HighestThreat);
        }
    }
}

void ACombat_AIController::ExecuteTacticalDecision()
{
    if (!TacticalAI || !BlackboardComp)
    {
        return;
    }

    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey));
    if (!CurrentTarget)
    {
        return;
    }

    // Get tactical decision from AI system
    ECombat_TacticalDecision Decision = TacticalAI->MakeTacticalDecision(
        GetPawn(), 
        CurrentTarget, 
        CurrentAIState,
        IsInPack()
    );

    // Execute the decision
    switch (Decision)
    {
        case ECombat_TacticalDecision::DirectAttack:
            InitiateAttack(CurrentTarget);
            break;
            
        case ECombat_TacticalDecision::FlankLeft:
        case ECombat_TacticalDecision::FlankRight:
            {
                FVector FlankPos = TacticalAI->CalculateFlankPosition(GetPawn(), CurrentTarget, Decision);
                FlankTarget(CurrentTarget, FlankPos);
            }
            break;
            
        case ECombat_TacticalDecision::Retreat:
            RetreatFromCombat();
            break;
            
        case ECombat_TacticalDecision::Wait:
        default:
            // Hold position and observe
            break;
    }
}