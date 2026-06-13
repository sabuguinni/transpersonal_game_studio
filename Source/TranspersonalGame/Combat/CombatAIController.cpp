#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize combat settings
    AttackRange = 300.0f;
    DetectionRadius = 2000.0f;
    FleeHealthThreshold = 0.3f;
    CurrentAIState = ECombat_AIState::Idle;

    // Initialize internal state
    CurrentTarget = nullptr;
    LastCombatTime = 0.0f;
    bInCombat = false;
    ThreatLevel = 0.0f;
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

    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("SelfActor"), InPawn);
        BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
    }
}

void ACombatAIController::SetCombatTarget(AActor* Target)
{
    CurrentTarget = Target;
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
    }

    if (Target)
    {
        StartCombat();
    }
    else
    {
        EndCombat();
    }
}

AActor* ACombatAIController::GetCombatTarget() const
{
    return CurrentTarget;
}

void ACombatAIController::StartCombat()
{
    bInCombat = true;
    LastCombatTime = GetWorld()->GetTimeSeconds();
    SetAIState(ECombat_AIState::Combat);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), true);
    }
}

void ACombatAIController::EndCombat()
{
    bInCombat = false;
    CurrentTarget = nullptr;
    SetAIState(ECombat_AIState::Idle);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), false);
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
    }
}

bool ACombatAIController::IsInCombat() const
{
    return bInCombat;
}

void ACombatAIController::FleeFromThreat()
{
    SetAIState(ECombat_AIState::Fleeing);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("ShouldFlee"), true);
    }
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    CurrentAIState = NewState;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(NewState));
    }
}

ECombat_AIState ACombatAIController::GetAIState() const
{
    return CurrentAIState;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a player or hostile entity
            if (Actor->ActorHasTag(TEXT("Player")) || Actor->ActorHasTag(TEXT("Hostile")))
            {
                SetCombatTarget(Actor);
                break;
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (Actor->ActorHasTag(TEXT("Player")))
        {
            SetCombatTarget(Actor);
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            SetAIState(ECombat_AIState::Investigating);
            
            // Set last known location
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("LastKnownLocation"), Actor->GetActorLocation());
            }
        }
    }
}

void ACombatAIController::UpdateCombatBehavior()
{
    if (!GetPawn()) return;

    EvaluateThreatLevel();

    if (ShouldFlee())
    {
        FleeFromThreat();
    }
    else if (ShouldAttack())
    {
        SetAIState(ECombat_AIState::Combat);
    }
    else if (ShouldInvestigate())
    {
        SetAIState(ECombat_AIState::Investigating);
    }
}

void ACombatAIController::EvaluateThreatLevel()
{
    ThreatLevel = 0.0f;

    if (CurrentTarget)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        // Closer targets are more threatening
        if (Distance < AttackRange)
        {
            ThreatLevel += 0.8f;
        }
        else if (Distance < DetectionRadius)
        {
            ThreatLevel += 0.4f;
        }

        // Player is always high threat
        if (CurrentTarget->ActorHasTag(TEXT("Player")))
        {
            ThreatLevel += 0.6f;
        }
    }

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), ThreatLevel);
    }
}

bool ACombatAIController::ShouldFlee() const
{
    // Check health threshold (would need health component)
    // For now, flee if threat level is very high
    return ThreatLevel > 1.2f;
}

bool ACombatAIController::ShouldAttack() const
{
    if (!CurrentTarget) return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Distance <= AttackRange && ThreatLevel > 0.5f;
}

bool ACombatAIController::ShouldInvestigate() const
{
    return CurrentTarget == nullptr && ThreatLevel > 0.2f;
}