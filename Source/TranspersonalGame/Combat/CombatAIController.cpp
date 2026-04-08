#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Perception - Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception - Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception - Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize state
    CurrentCombatState = ECombatState::Idle;
    StateChangeTimer = 0.0f;
    LastPlayerSightingTime = 0.0f;
    TacticalEvaluationTimer = 0.0f;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);

    // Cache player reference
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    // Start behavior tree if assigned
    if (BehaviorTreeAsset && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatMemory(DeltaTime);
    
    TacticalEvaluationTimer += DeltaTime;
    if (TacticalEvaluationTimer >= 1.0f) // Evaluate tactics every second
    {
        EvaluateTacticalOptions();
        TacticalEvaluationTimer = 0.0f;
    }

    UpdateBlackboardValues();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        StateChangeTimer = 0.0f;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Combat AI State changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ACombatAIController::UpdateThreatLevel(EThreatLevel NewThreatLevel)
{
    CombatMemory.CurrentThreatLevel = NewThreatLevel;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(NewThreatLevel));
    }
}

void ACombatAIController::RecordPlayerSighting(const FVector& PlayerLocation)
{
    CombatMemory.LastKnownPlayerLocation = PlayerLocation;
    CombatMemory.TimeSinceLastSighting = 0.0f;
    LastPlayerSightingTime = GetWorld()->GetTimeSeconds();

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), PlayerLocation);
        BlackboardComponent->SetValueAsBool(TEXT("HasRecentPlayerSighting"), true);
    }

    // Escalate threat level based on proximity
    float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerLocation);
    if (DistanceToPlayer < AttackRange)
    {
        UpdateThreatLevel(EThreatLevel::Critical);
    }
    else if (DistanceToPlayer < PreferredCombatDistance)
    {
        UpdateThreatLevel(EThreatLevel::High);
    }
    else
    {
        UpdateThreatLevel(EThreatLevel::Medium);
    }
}

void ACombatAIController::RecordFailedAttack()
{
    CombatMemory.FailedAttackCount++;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsInt(TEXT("FailedAttackCount"), CombatMemory.FailedAttackCount);
    }
}

bool ACombatAIController::ShouldRetreat() const
{
    return CombatMemory.FailedAttackCount >= MaxFailedAttacksBeforeRetreat ||
           CombatMemory.CurrentThreatLevel == EThreatLevel::Critical;
}

bool ACombatAIController::CanSeePlayer() const
{
    if (!PlayerPawn || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(PlayerPawn, Info);
    
    return Info.LastSensedStimuli.Num() > 0 && 
           Info.LastSensedStimuli[0].WasSuccessfullySensed();
}

FVector ACombatAIController::CalculateOptimalAttackPosition()
{
    if (!PlayerPawn)
    {
        return GetPawn()->GetActorLocation();
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate position that maintains optimal attack distance
    FVector DirectionToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    FVector OptimalPosition = PlayerLocation - (DirectionToPlayer * PreferredCombatDistance);

    // Add some randomization to avoid predictable positioning
    FVector RandomOffset = FVector(
        FMath::RandRange(-200.0f, 200.0f),
        FMath::RandRange(-200.0f, 200.0f),
        0.0f
    );
    
    return OptimalPosition + RandomOffset;
}

FVector ACombatAIController::CalculateCirclingPosition()
{
    if (!PlayerPawn)
    {
        return GetPawn()->GetActorLocation();
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate perpendicular direction for circling
    FVector ToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToPlayer, FVector::UpVector);
    
    // Alternate circling direction based on time
    float CircleDirection = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f) > 0 ? 1.0f : -1.0f;
    
    FVector CirclePosition = PlayerLocation + (RightVector * CirclingRadius * CircleDirection);
    
    return CirclePosition;
}

FVector ACombatAIController::CalculateAmbushPosition()
{
    if (!PlayerPawn)
    {
        return GetPawn()->GetActorLocation();
    }

    // Predict where player will be
    FVector PredictedPlayerPos = PredictPlayerMovement();
    
    // Find a position that's hidden but close to predicted path
    FVector AmbushPos = PredictedPlayerPos + FVector(
        FMath::RandRange(-400.0f, 400.0f),
        FMath::RandRange(-400.0f, 400.0f),
        0.0f
    );

    return AmbushPos;
}

bool ACombatAIController::ShouldUseAmbushTactic() const
{
    return CombatMemory.CurrentThreatLevel <= EThreatLevel::Medium &&
           CombatMemory.TimeSinceLastSighting > 2.0f &&
           CombatMemory.TimeSinceLastSighting < 10.0f;
}

bool ACombatAIController::ShouldCirclePlayer() const
{
    return CombatMemory.CurrentThreatLevel == EThreatLevel::High &&
           CalculateDistanceToPlayer() > AttackRange &&
           CalculateDistanceToPlayer() < CirclingRadius * 1.5f;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor == PlayerPawn)
        {
            RecordPlayerSighting(Actor->GetActorLocation());
            break;
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == PlayerPawn && Stimulus.WasSuccessfullySensed())
    {
        RecordPlayerSighting(Stimulus.StimulusLocation);
    }
}

void ACombatAIController::UpdateCombatMemory(float DeltaTime)
{
    CombatMemory.TimeSinceLastSighting += DeltaTime;
    StateChangeTimer += DeltaTime;

    // Decay threat level over time
    if (CombatMemory.TimeSinceLastSighting > ThreatEscalationTime)
    {
        if (CombatMemory.CurrentThreatLevel > EThreatLevel::None)
        {
            EThreatLevel NewLevel = static_cast<EThreatLevel>(
                static_cast<int32>(CombatMemory.CurrentThreatLevel) - 1
            );
            UpdateThreatLevel(NewLevel);
        }
    }

    // Update player hiding status
    CombatMemory.bPlayerIsHiding = !CanSeePlayer() && 
                                   CombatMemory.TimeSinceLastSighting > 3.0f;
}

void ACombatAIController::EvaluateTacticalOptions()
{
    if (!PlayerPawn)
    {
        return;
    }

    float DistanceToPlayer = CalculateDistanceToPlayer();
    
    // State machine logic
    switch (CurrentCombatState)
    {
        case ECombatState::Idle:
            if (CanSeePlayer())
            {
                SetCombatState(ECombatState::Stalking);
            }
            break;

        case ECombatState::Stalking:
            if (!CanSeePlayer() && ShouldUseAmbushTactic())
            {
                SetCombatState(ECombatState::Ambushing);
            }
            else if (DistanceToPlayer < AttackRange)
            {
                SetCombatState(ECombatState::Attacking);
            }
            else if (ShouldCirclePlayer())
            {
                SetCombatState(ECombatState::Circling);
            }
            break;

        case ECombatState::Attacking:
            if (ShouldRetreat())
            {
                SetCombatState(ECombatState::Retreating);
            }
            else if (DistanceToPlayer > AttackRange * 1.5f)
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;

        case ECombatState::Circling:
            if (DistanceToPlayer < AttackRange)
            {
                SetCombatState(ECombatState::Attacking);
            }
            else if (!CanSeePlayer())
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;

        case ECombatState::Retreating:
            if (DistanceToPlayer > RetreatDistance)
            {
                SetCombatState(ECombatState::Idle);
                CombatMemory.FailedAttackCount = 0; // Reset failed attacks
            }
            break;

        case ECombatState::Ambushing:
            if (CanSeePlayer() && DistanceToPlayer < AttackRange)
            {
                SetCombatState(ECombatState::Attacking);
            }
            else if (StateChangeTimer > AmbushWaitTime)
            {
                SetCombatState(ECombatState::Hunting);
            }
            break;

        case ECombatState::Hunting:
            if (CanSeePlayer())
            {
                SetCombatState(ECombatState::Stalking);
            }
            else if (CombatMemory.TimeSinceLastSighting > 30.0f)
            {
                SetCombatState(ECombatState::Idle);
            }
            break;
    }
}

void ACombatAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update all relevant blackboard keys
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToPlayer"), CalculateDistanceToPlayer());
    BlackboardComponent->SetValueAsBool(TEXT("CanSeePlayer"), CanSeePlayer());
    BlackboardComponent->SetValueAsBool(TEXT("ShouldRetreat"), ShouldRetreat());
    BlackboardComponent->SetValueAsVector(TEXT("OptimalAttackPosition"), CalculateOptimalAttackPosition());
    BlackboardComponent->SetValueAsVector(TEXT("CirclingPosition"), CalculateCirclingPosition());
    BlackboardComponent->SetValueAsVector(TEXT("AmbushPosition"), CalculateAmbushPosition());
    BlackboardComponent->SetValueAsBool(TEXT("ShouldUseAmbush"), ShouldUseAmbushTactic());
    BlackboardComponent->SetValueAsBool(TEXT("ShouldCircle"), ShouldCirclePlayer());
}

float ACombatAIController::CalculateDistanceToPlayer() const
{
    if (!PlayerPawn)
    {
        return FLT_MAX;
    }

    return FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
}

bool ACombatAIController::IsPlayerInLineOfSight() const
{
    if (!PlayerPawn)
    {
        return false;
    }

    FVector Start = GetPawn()->GetActorLocation();
    FVector End = PlayerPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());
    
    return !GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_Visibility, Params
    );
}

FVector ACombatAIController::PredictPlayerMovement() const
{
    if (!PlayerPawn)
    {
        return FVector::ZeroVector;
    }

    // Simple prediction based on current velocity
    FVector PlayerVelocity = PlayerPawn->GetVelocity();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Predict 2 seconds ahead
    return PlayerLocation + (PlayerVelocity * 2.0f);
}