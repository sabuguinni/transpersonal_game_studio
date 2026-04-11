#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Default values
    AIType = ECombatAIType::Predator;
    CurrentState = ECombatAIState::Idle;
    
    // Perception defaults
    SightRadius = 1500.0f;
    LoseSightRadius = 1600.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 1000.0f;
    
    // Combat defaults
    AttackRange = 200.0f;
    FlankingDistance = 500.0f;
    RetreatHealthThreshold = 0.3f;
    bCanCallForHelp = true;
    GroupCoordinationRadius = 800.0f;
    
    // State tracking
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    TimeSinceLastSawTarget = 0.0f;
    LastAllyUpdateTime = 0.0f;
    StateTransitionCooldown = 0.0f;
    bIsCoordinatingAttack = false;

    SetupPerception();
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastSawTarget += DeltaTime;
    StateTransitionCooldown -= DeltaTime;
    
    UpdateCombatState();
    HandleStateTransitions();
    
    // Update nearby allies periodically
    LastAllyUpdateTime += DeltaTime;
    if (LastAllyUpdateTime >= 2.0f)
    {
        UpdateNearbyAllies();
        LastAllyUpdateTime = 0.0f;
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (BlackboardComponent && BehaviorTree)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
    }
}

void ACombatAIController::SetupPerception()
{
    if (!PerceptionComponent)
        return;

    // Setup sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Setup hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetSeen);
}

void ACombatAIController::SetCombatState(ECombatAIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), (uint8)NewState);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI State changed to: %d"), (int32)NewState);
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            TimeSinceLastSawTarget = 0.0f;
        }
    }
}

bool ACombatAIController::CanSeeTarget() const
{
    if (!CurrentTarget || !PerceptionComponent)
        return false;
        
    FActorPerceptionBlueprintInfo Info;
    PerceptionComponent->GetActorsPerception(CurrentTarget, Info);
    
    for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }
    
    return false;
}

bool ACombatAIController::IsInAttackRange() const
{
    if (!CurrentTarget || !GetPawn())
        return false;
        
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Distance <= AttackRange;
}

FVector ACombatAIController::GetFlankingPosition() const
{
    if (!CurrentTarget || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    return TargetLocation + (FlankDirection * FlankingDistance);
}

void ACombatAIController::CallForHelp()
{
    if (!bCanCallForHelp)
        return;
    
    for (ACombatAIController* Ally : NearbyAllies)
    {
        if (Ally && Ally->CurrentTarget == nullptr)
        {
            Ally->SetTarget(CurrentTarget);
            Ally->SetCombatState(ECombatAIState::Chasing);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat AI called for help from %d allies"), NearbyAllies.Num());
}

void ACombatAIController::CoordinateGroupAttack()
{
    if (bIsCoordinatingAttack || NearbyAllies.Num() == 0)
        return;
    
    bIsCoordinatingAttack = true;
    
    // Assign different roles to allies
    for (int32 i = 0; i < NearbyAllies.Num(); ++i)
    {
        ACombatAIController* Ally = NearbyAllies[i];
        if (!Ally) continue;
        
        switch (i % 3)
        {
            case 0:
                Ally->SetCombatState(ECombatAIState::Attacking);
                break;
            case 1:
                Ally->SetCombatState(ECombatAIState::Flanking);
                break;
            case 2:
                Ally->SetCombatState(ECombatAIState::GroupCoordination);
                break;
        }
    }
    
    // Reset coordination flag after delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { bIsCoordinatingAttack = false; },
        5.0f,
        false
    );
}

bool ACombatAIController::ShouldRetreat() const
{
    // Check health threshold (would need health component reference)
    // For now, retreat based on overwhelming enemy numbers
    return NearbyAllies.Num() < 2 && TimeSinceLastSawTarget > 10.0f;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a potential target (player or enemy)
            if (!CurrentTarget)
            {
                SetTarget(Actor);
                SetCombatState(ECombatAIState::Investigating);
            }
        }
    }
}

void ACombatAIController::OnTargetSeen(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        SetTarget(Actor);
        LastKnownTargetLocation = Stimulus.StimulusLocation;
        TimeSinceLastSawTarget = 0.0f;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
        }
    }
}

void ACombatAIController::OnTargetLost(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == CurrentTarget)
    {
        LastKnownTargetLocation = Stimulus.StimulusLocation;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
        }
        
        SetCombatState(ECombatAIState::Investigating);
    }
}

void ACombatAIController::UpdateNearbyAllies()
{
    NearbyAllies.Empty();
    
    if (!GetPawn())
        return;
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find all combat AI controllers in range
    for (TActorIterator<ACombatAIController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ACombatAIController* OtherAI = *ActorItr;
        if (OtherAI && OtherAI != this && OtherAI->GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, OtherAI->GetPawn()->GetActorLocation());
            if (Distance <= GroupCoordinationRadius)
            {
                NearbyAllies.Add(OtherAI);
            }
        }
    }
}

void ACombatAIController::UpdateCombatState()
{
    if (!CurrentTarget)
    {
        if (CurrentState != ECombatAIState::Patrolling)
        {
            SetCombatState(ECombatAIState::Patrolling);
        }
        return;
    }
    
    bool bCanSeeTarget = CanSeeTarget();
    bool bInAttackRange = IsInAttackRange();
    
    // State logic based on AI type
    switch (AIType)
    {
        case ECombatAIType::Predator:
            if (bCanSeeTarget && bInAttackRange)
            {
                SetCombatState(ECombatAIState::Attacking);
            }
            else if (bCanSeeTarget)
            {
                SetCombatState(ECombatAIState::Chasing);
            }
            else if (TimeSinceLastSawTarget < 5.0f)
            {
                SetCombatState(ECombatAIState::Investigating);
            }
            break;
            
        case ECombatAIType::Pack:
            if (NearbyAllies.Num() >= 2)
            {
                CoordinateGroupAttack();
            }
            else if (bCanSeeTarget)
            {
                CallForHelp();
                SetCombatState(ECombatAIState::Chasing);
            }
            break;
            
        case ECombatAIType::Ambush:
            if (bCanSeeTarget && bInAttackRange)
            {
                SetCombatState(ECombatAIState::Attacking);
            }
            else
            {
                SetCombatState(ECombatAIState::Idle); // Wait for ambush opportunity
            }
            break;
            
        default:
            break;
    }
}

void ACombatAIController::HandleStateTransitions()
{
    if (StateTransitionCooldown > 0.0f)
        return;
    
    // Handle retreat logic
    if (ShouldRetreat() && CurrentState != ECombatAIState::Retreating)
    {
        SetCombatState(ECombatAIState::Retreating);
        StateTransitionCooldown = 2.0f;
    }
    
    // Handle flanking maneuvers
    if (CurrentState == ECombatAIState::Chasing && NearbyAllies.Num() > 0)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to flank
        {
            SetCombatState(ECombatAIState::Flanking);
            StateTransitionCooldown = 3.0f;
        }
    }
}