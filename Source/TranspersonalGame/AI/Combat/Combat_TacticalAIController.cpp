#include "Combat_TacticalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

ACombat_TacticalAIController::ACombat_TacticalAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Set default tactical state
    CurrentTacticalState = ECombat_TacticalState::Idle;
    
    // Initialize tactical data with balanced values
    TacticalData.AggressionLevel = 0.6f;
    TacticalData.CautiousDistance = 1200.0f;
    TacticalData.AttackDistance = 400.0f;
    TacticalData.FlankingRadius = 900.0f;
    TacticalData.bCanCallForHelp = true;
    TacticalData.PackSize = 1;
}

void ACombat_TacticalAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
    
    // Find nearby allies
    FindNearbyAllies();
}

void ACombat_TacticalAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTacticalBehavior(DeltaTime);
    UpdateBlackboard();
    
    StateChangeTimer += DeltaTime;
}

void ACombat_TacticalAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAIController::OnTargetPerceptionUpdated);
}

void ACombat_TacticalAIController::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        CurrentTacticalState = NewState;
        StateChangeTimer = 0.0f;
        
        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombat_TacticalAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
        }
    }
}

bool ACombat_TacticalAIController::CanSeeTarget() const
{
    if (!CurrentTarget || !AIPerceptionComponent)
        return false;
    
    FActorPerceptionBlueprintInfo PerceptionInfo;
    return AIPerceptionComponent->GetActorsPerception(CurrentTarget, PerceptionInfo) && 
           PerceptionInfo.HasKnownStimulusOfSense(UAISense_Sight::StaticClass());
}

float ACombat_TacticalAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
        return -1.0f;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

FVector ACombat_TacticalAIController::GetFlankingPosition() const
{
    if (!CurrentTarget || !GetPawn())
        return FVector::ZeroVector;
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the side
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (FlankDirection * TacticalData.FlankingRadius);
    
    return FlankPosition;
}

void ACombat_TacticalAIController::CallForHelp(float Radius)
{
    if (!TacticalData.bCanCallForHelp || !GetPawn())
        return;
    
    FVector PawnLocation = GetPawn()->GetActorLocation();
    
    // Find nearby allied AIs and alert them
    for (AActor* Ally : AlliedAIs)
    {
        if (Ally && FVector::Dist(PawnLocation, Ally->GetActorLocation()) <= Radius)
        {
            if (ACombat_TacticalAIController* AllyController = Cast<ACombat_TacticalAIController>(Ally->GetInstigatorController()))
            {
                AllyController->SetTarget(CurrentTarget);
                AllyController->SetTacticalState(ECombat_TacticalState::Investigating);
            }
        }
    }
}

bool ACombat_TacticalAIController::ShouldRetreat() const
{
    if (!GetPawn())
        return false;
    
    // Simple health-based retreat logic
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Retreat if health is low (would need actual health component)
        // For now, retreat based on aggression level and distance
        float DistanceToTarget = GetDistanceToTarget();
        return (DistanceToTarget < TacticalData.CautiousDistance * 0.5f) && (TacticalData.AggressionLevel < 0.3f);
    }
    
    return false;
}

void ACombat_TacticalAIController::UpdateTacticalBehavior(float DeltaTime)
{
    ProcessTacticalDecision();
    
    // Update search timer for lost targets
    if (!CanSeeTarget() && CurrentTarget)
    {
        SearchTimer += DeltaTime;
        
        // Give up searching after 10 seconds
        if (SearchTimer > 10.0f)
        {
            SetTarget(nullptr);
            SetTacticalState(ECombat_TacticalState::Patrolling);
            SearchTimer = 0.0f;
        }
    }
    else
    {
        SearchTimer = 0.0f;
    }
}

void ACombat_TacticalAIController::ProcessTacticalDecision()
{
    if (!GetPawn())
        return;
    
    float DistanceToTarget = GetDistanceToTarget();
    bool bCanSeeTarget = CanSeeTarget();
    
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Idle:
            if (CurrentTarget)
            {
                SetTacticalState(ECombat_TacticalState::Investigating);
            }
            else if (StateChangeTimer > 5.0f)
            {
                SetTacticalState(ECombat_TacticalState::Patrolling);
            }
            break;
            
        case ECombat_TacticalState::Patrolling:
            if (CurrentTarget)
            {
                SetTacticalState(ECombat_TacticalState::Investigating);
            }
            break;
            
        case ECombat_TacticalState::Investigating:
            if (bCanSeeTarget && DistanceToTarget <= TacticalData.CautiousDistance)
            {
                SetTacticalState(ECombat_TacticalState::Stalking);
            }
            else if (!CurrentTarget)
            {
                SetTacticalState(ECombat_TacticalState::Patrolling);
            }
            break;
            
        case ECombat_TacticalState::Stalking:
            if (DistanceToTarget <= TacticalData.AttackDistance)
            {
                SetTacticalState(ECombat_TacticalState::Attacking);
            }
            else if (ShouldRetreat())
            {
                SetTacticalState(ECombat_TacticalState::Retreating);
            }
            else if (TacticalData.PackSize > 1 && StateChangeTimer > 3.0f)
            {
                SetTacticalState(ECombat_TacticalState::Flanking);
            }
            break;
            
        case ECombat_TacticalState::Attacking:
            if (DistanceToTarget > TacticalData.AttackDistance * 1.5f)
            {
                SetTacticalState(ECombat_TacticalState::Stalking);
            }
            else if (ShouldRetreat())
            {
                SetTacticalState(ECombat_TacticalState::Retreating);
            }
            break;
            
        case ECombat_TacticalState::Flanking:
            if (DistanceToTarget <= TacticalData.AttackDistance)
            {
                SetTacticalState(ECombat_TacticalState::Attacking);
            }
            else if (StateChangeTimer > 8.0f)
            {
                SetTacticalState(ECombat_TacticalState::Stalking);
            }
            break;
            
        case ECombat_TacticalState::Retreating:
            if (DistanceToTarget > TacticalData.CautiousDistance * 2.0f)
            {
                SetTacticalState(ECombat_TacticalState::Regrouping);
            }
            break;
            
        case ECombat_TacticalState::Regrouping:
            if (StateChangeTimer > 5.0f)
            {
                if (CurrentTarget && TacticalData.AggressionLevel > 0.5f)
                {
                    SetTacticalState(ECombat_TacticalState::Stalking);
                }
                else
                {
                    SetTacticalState(ECombat_TacticalState::Patrolling);
                }
            }
            break;
    }
}

void ACombat_TacticalAIController::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;
    
    BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), CanSeeTarget());
    BlackboardComponent->SetValueAsVector(TEXT("FlankingPosition"), GetFlankingPosition());
    BlackboardComponent->SetValueAsBool(TEXT("ShouldRetreat"), ShouldRetreat());
}

void ACombat_TacticalAIController::FindNearbyAllies()
{
    if (!GetPawn())
        return;
    
    AlliedAIs.Empty();
    FVector PawnLocation = GetPawn()->GetActorLocation();
    
    // Find other tactical AI controllers within a reasonable distance
    for (TActorIterator<APawn> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        APawn* OtherPawn = *ActorIterator;
        if (OtherPawn && OtherPawn != GetPawn())
        {
            if (ACombat_TacticalAIController* OtherController = Cast<ACombat_TacticalAIController>(OtherPawn->GetController()))
            {
                float Distance = FVector::Dist(PawnLocation, OtherPawn->GetActorLocation());
                if (Distance <= 3000.0f) // Allies within 30 meters
                {
                    AlliedAIs.Add(OtherPawn);
                }
            }
        }
    }
}

void ACombat_TacticalAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is a player character or enemy
            if (!CurrentTarget)
            {
                SetTarget(Actor);
                SetTacticalState(ECombat_TacticalState::Investigating);
                
                // Call for help if configured
                if (TacticalData.bCanCallForHelp)
                {
                    CallForHelp();
                }
            }
        }
    }
}

void ACombat_TacticalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == CurrentTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            LastKnownTargetLocation = Stimulus.StimulusLocation;
            LastTargetSightTime = GetWorld()->GetTimeSeconds();
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            }
        }
    }
}