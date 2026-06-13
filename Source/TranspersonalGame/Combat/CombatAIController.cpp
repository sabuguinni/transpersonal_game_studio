#include "CombatAIController.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/AIController.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Set default values
    CurrentState = ECombat_AIState::Idle;
    SightRadius = 1500.0f;
    HearingRadius = 800.0f;
    AttackRange = 200.0f;
    FleeHealthThreshold = 0.3f;
    AggressionLevel = 0.7f;
    PatrolRadius = 1000.0f;
    StateChangeTime = 0.0f;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    InitializeAIPerception();
    InitializeBehaviorTree();

    // Set home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAIState(DeltaTime);
    ProcessThreats(DeltaTime);
    UpdateThreatAssessment();
    SetBlackboardValues();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
    }

    InitializeBehaviorTree();
}

void ACombatAIController::InitializeAIPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = SightRadius + 200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(10.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRadius;
        HearingConfig->SetMaxAge(5.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ACombatAIController::InitializeBehaviorTree()
{
    if (BehaviorTree && BlackboardComponent)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && IsActorHostile(Actor))
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            AddThreat(Actor, ThreatLevel);
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !IsActorHostile(Actor))
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        float ThreatLevel = CalculateThreatLevel(Actor);
        AddThreat(Actor, ThreatLevel);

        // If this is a high threat and we're not in combat, start combat
        if (ThreatLevel > 0.7f && CurrentState != ECombat_AIState::Attacking && CurrentState != ECombat_AIState::Chasing)
        {
            StartCombat(Actor);
        }
    }
    else
    {
        // Lost sight of target
        for (FCombat_ThreatInfo& Threat : KnownThreats)
        {
            if (Threat.ThreatActor == Actor)
            {
                Threat.bIsVisible = false;
                Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
                break;
            }
        }
    }
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
        }
    }
}

void ACombatAIController::SetCurrentTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
    }
}

void ACombatAIController::AddThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
        return;

    // Check if threat already exists
    for (FCombat_ThreatInfo& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            Threat.ThreatLevel = ThreatLevel;
            Threat.LastKnownLocation = ThreatActor->GetActorLocation();
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            Threat.bIsVisible = true;
            return;
        }
    }

    // Add new threat
    FCombat_ThreatInfo NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = ThreatLevel;
    NewThreat.LastKnownLocation = ThreatActor->GetActorLocation();
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewThreat.bIsVisible = true;

    KnownThreats.Add(NewThreat);
}

void ACombatAIController::RemoveThreat(AActor* ThreatActor)
{
    KnownThreats.RemoveAll([ThreatActor](const FCombat_ThreatInfo& Threat)
    {
        return Threat.ThreatActor == ThreatActor;
    });
}

FCombat_ThreatInfo ACombatAIController::GetHighestThreat() const
{
    FCombat_ThreatInfo HighestThreat;
    float MaxThreat = 0.0f;

    for (const FCombat_ThreatInfo& Threat : KnownThreats)
    {
        if (Threat.ThreatLevel > MaxThreat && Threat.ThreatActor)
        {
            MaxThreat = Threat.ThreatLevel;
            HighestThreat = Threat;
        }
    }

    return HighestThreat;
}

bool ACombatAIController::CanAttackTarget() const
{
    if (!CurrentTarget || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Distance <= AttackRange;
}

bool ACombatAIController::ShouldFlee() const
{
    if (!GetPawn())
        return false;

    // Check health
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Assume health component exists
        float HealthPercent = 1.0f; // Placeholder - would get from health component
        if (HealthPercent < FleeHealthThreshold)
            return true;
    }

    // Check if overwhelmed by threats
    int32 HighThreats = 0;
    for (const FCombat_ThreatInfo& Threat : KnownThreats)
    {
        if (Threat.ThreatLevel > 0.6f && Threat.bIsVisible)
            HighThreats++;
    }

    return HighThreats > 2;
}

void ACombatAIController::StartCombat(AActor* Enemy)
{
    SetCurrentTarget(Enemy);
    SetAIState(ECombat_AIState::Chasing);
}

void ACombatAIController::EndCombat()
{
    SetCurrentTarget(nullptr);
    SetAIState(ECombat_AIState::Patrolling);
}

FVector ACombatAIController::GetBestAttackPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector Direction = (TargetLocation - PawnLocation).GetSafeNormal();

    // Position slightly closer than attack range
    return TargetLocation - (Direction * (AttackRange * 0.8f));
}

void ACombatAIController::UpdateThreatAssessment()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Remove old threats
    KnownThreats.RemoveAll([CurrentTime](const FCombat_ThreatInfo& Threat)
    {
        return (CurrentTime - Threat.LastSeenTime) > 30.0f || !IsValid(Threat.ThreatActor);
    });

    // Update threat visibility
    for (FCombat_ThreatInfo& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && AIPerceptionComponent)
        {
            FActorPerceptionBlueprintInfo PerceptionInfo;
            AIPerceptionComponent->GetActorsPerception(Threat.ThreatActor, PerceptionInfo);
            Threat.bIsVisible = PerceptionInfo.bIsHostile && PerceptionInfo.LastSensedStimuli.Num() > 0;
        }
    }
}

void ACombatAIController::UpdateAIState(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_AIState::Idle:
            if (KnownThreats.Num() > 0)
            {
                SetAIState(ECombat_AIState::Investigating);
            }
            break;

        case ECombat_AIState::Investigating:
            if (CurrentTarget)
            {
                SetAIState(ECombat_AIState::Chasing);
            }
            break;

        case ECombat_AIState::Chasing:
            if (CanAttackTarget())
            {
                SetAIState(ECombat_AIState::Attacking);
            }
            else if (ShouldFlee())
            {
                SetAIState(ECombat_AIState::Fleeing);
            }
            break;

        case ECombat_AIState::Attacking:
            if (!CanAttackTarget())
            {
                SetAIState(ECombat_AIState::Chasing);
            }
            else if (ShouldFlee())
            {
                SetAIState(ECombat_AIState::Fleeing);
            }
            break;

        case ECombat_AIState::Fleeing:
            if (KnownThreats.Num() == 0)
            {
                SetAIState(ECombat_AIState::Patrolling);
            }
            break;
    }
}

void ACombatAIController::ProcessThreats(float DeltaTime)
{
    if (KnownThreats.Num() == 0)
        return;

    FCombat_ThreatInfo HighestThreat = GetHighestThreat();
    if (HighestThreat.ThreatActor && HighestThreat.ThreatActor != CurrentTarget)
    {
        SetCurrentTarget(HighestThreat.ThreatActor);
    }
}

float ACombatAIController::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return 0.0f;

    float ThreatLevel = 0.5f; // Base threat

    // Distance factor
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / SightRadius), 0.0f, 1.0f);
    ThreatLevel += DistanceFactor * 0.3f;

    // Size/health factor (placeholder)
    ThreatLevel += 0.2f;

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool ACombatAIController::IsActorHostile(AActor* Actor) const
{
    if (!Actor)
        return false;

    // Check if it's a player character
    if (Actor->IsA<ACharacter>())
    {
        return true; // For now, all characters are considered hostile
    }

    return false;
}

void ACombatAIController::SetBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    BlackboardComponent->SetValueAsBool(TEXT("ShouldFlee"), ShouldFlee());

    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), 
            FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()));
    }

    FCombat_ThreatInfo HighestThreat = GetHighestThreat();
    BlackboardComponent->SetValueAsFloat(TEXT("HighestThreatLevel"), HighestThreat.ThreatLevel);
}