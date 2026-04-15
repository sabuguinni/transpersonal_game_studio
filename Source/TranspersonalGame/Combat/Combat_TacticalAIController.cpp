#include "Combat_TacticalAIController.h"
#include "Combat_TacticalBehaviorComponent.h"
#include "Combat_PackCoordinationComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Blackboard Keys
const FName ACombat_TacticalAIController::BB_TargetActor = TEXT("TargetActor");
const FName ACombat_TacticalAIController::BB_TargetLocation = TEXT("TargetLocation");
const FName ACombat_TacticalAIController::BB_AIState = TEXT("AIState");
const FName ACombat_TacticalAIController::BB_ThreatLevel = TEXT("ThreatLevel");
const FName ACombat_TacticalAIController::BB_ShouldRetreat = TEXT("ShouldRetreat");
const FName ACombat_TacticalAIController::BB_FlankingPosition = TEXT("FlankingPosition");
const FName ACombat_TacticalAIController::BB_PackLeader = TEXT("PackLeader");
const FName ACombat_TacticalAIController::BB_PackMembers = TEXT("PackMembers");

ACombat_TacticalAIController::ACombat_TacticalAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Create Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfView;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Create Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Create Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Create Blackboard Component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Create Tactical Components
    TacticalBehaviorComponent = CreateDefaultSubobject<UCombat_TacticalBehaviorComponent>(TEXT("TacticalBehaviorComponent"));
    PackCoordinationComponent = CreateDefaultSubobject<UCombat_PackCoordinationComponent>(TEXT("PackCoordinationComponent"));

    // Initialize State
    CurrentState = ECombat_AIState::Idle;
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

    // Start Behavior Tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }

    // Initialize blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(BB_AIState, static_cast<uint8>(CurrentState));
        BlackboardComponent->SetValueAsBool(BB_ShouldRetreat, false);
    }
}

void ACombat_TacticalAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateTacticalState(DeltaTime);
}

void ACombat_TacticalAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(BB_AIState, static_cast<uint8>(CurrentState));
        }

        // Notify tactical behavior component
        if (TacticalBehaviorComponent)
        {
            TacticalBehaviorComponent->OnAIStateChanged(CurrentState);
        }
    }
}

void ACombat_TacticalAIController::SetPrimaryTarget(AActor* NewTarget)
{
    if (NewTarget != PrimaryTarget.TargetActor)
    {
        PrimaryTarget.TargetActor = NewTarget;
        
        if (NewTarget)
        {
            PrimaryTarget.LastKnownPosition = NewTarget->GetActorLocation();
            PrimaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
            PrimaryTarget.bIsVisible = CanSeeTarget(NewTarget);
            PrimaryTarget.ThreatLevel = static_cast<float>(EvaluateThreatLevel(NewTarget));
        }
        else
        {
            PrimaryTarget = FCombat_TacticalTarget();
        }

        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(BB_TargetActor, NewTarget);
            BlackboardComponent->SetValueAsVector(BB_TargetLocation, PrimaryTarget.LastKnownPosition);
        }
    }
}

void ACombat_TacticalAIController::AddKnownTarget(AActor* Target, float ThreatLevel)
{
    if (!Target) return;

    // Check if target already exists
    for (FCombat_TacticalTarget& KnownTarget : KnownTargets)
    {
        if (KnownTarget.TargetActor == Target)
        {
            KnownTarget.LastKnownPosition = Target->GetActorLocation();
            KnownTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
            KnownTarget.ThreatLevel = ThreatLevel;
            KnownTarget.bIsVisible = CanSeeTarget(Target);
            return;
        }
    }

    // Add new target
    FCombat_TacticalTarget NewTarget;
    NewTarget.TargetActor = Target;
    NewTarget.LastKnownPosition = Target->GetActorLocation();
    NewTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewTarget.ThreatLevel = ThreatLevel;
    NewTarget.bIsVisible = CanSeeTarget(Target);
    
    KnownTargets.Add(NewTarget);
}

void ACombat_TacticalAIController::RemoveKnownTarget(AActor* Target)
{
    KnownTargets.RemoveAll([Target](const FCombat_TacticalTarget& KnownTarget)
    {
        return KnownTarget.TargetActor == Target;
    });
}

bool ACombat_TacticalAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent) return false;

    FActorPerceptionBlueprintInfo PerceptionInfo;
    AIPerceptionComponent->GetActorsPerception(Target, PerceptionInfo);
    
    for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }
    
    return false;
}

float ACombat_TacticalAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return -1.0f;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

ECombat_ThreatLevel ACombat_TacticalAIController::EvaluateThreatLevel(AActor* Target) const
{
    if (!Target) return ECombat_ThreatLevel::None;

    float Distance = GetDistanceToTarget(Target);
    bool bCanSee = CanSeeTarget(Target);
    
    // Evaluate based on distance and visibility
    if (Distance < 300.0f && bCanSee)
        return ECombat_ThreatLevel::Critical;
    else if (Distance < 600.0f && bCanSee)
        return ECombat_ThreatLevel::High;
    else if (Distance < 1000.0f && bCanSee)
        return ECombat_ThreatLevel::Medium;
    else if (Distance < 1500.0f)
        return ECombat_ThreatLevel::Low;
    
    return ECombat_ThreatLevel::None;
}

FVector ACombat_TacticalAIController::GetTacticalPosition(AActor* Target, float Distance) const
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector Direction = (PawnLocation - TargetLocation).GetSafeNormal();
    
    // Add some randomness for flanking
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    float FlankingOffset = FMath::RandRange(-1.0f, 1.0f) * 500.0f;
    
    return TargetLocation + (Direction * Distance) + (RightVector * FlankingOffset);
}

bool ACombat_TacticalAIController::ShouldRetreat() const
{
    if (!GetPawn()) return false;

    // Check health
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Simple retreat logic - retreat if heavily damaged or outnumbered
        float HealthRatio = 1.0f; // TODO: Get actual health ratio
        int32 NearbyEnemies = KnownTargets.Num();
        
        return (HealthRatio < 0.3f) || (NearbyEnemies > 2 && HealthRatio < 0.6f);
    }
    
    return false;
}

void ACombat_TacticalAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Evaluate if this is a threat
            ECombat_ThreatLevel ThreatLevel = EvaluateThreatLevel(Actor);
            
            if (ThreatLevel != ECombat_ThreatLevel::None)
            {
                AddKnownTarget(Actor, static_cast<float>(ThreatLevel));
                
                // Set as primary target if it's the highest threat
                if (!PrimaryTarget.TargetActor || ThreatLevel > static_cast<ECombat_ThreatLevel>(PrimaryTarget.ThreatLevel))
                {
                    SetPrimaryTarget(Actor);
                }
            }
        }
    }
}

void ACombat_TacticalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        ECombat_ThreatLevel ThreatLevel = EvaluateThreatLevel(Actor);
        AddKnownTarget(Actor, static_cast<float>(ThreatLevel));
        
        // Switch to hunting state if we see a threat
        if (ThreatLevel >= ECombat_ThreatLevel::Medium && CurrentState == ECombat_AIState::Idle)
        {
            SetAIState(ECombat_AIState::Hunting);
        }
    }
    else
    {
        // Target lost
        for (FCombat_TacticalTarget& KnownTarget : KnownTargets)
        {
            if (KnownTarget.TargetActor == Actor)
            {
                KnownTarget.bIsVisible = false;
                KnownTarget.LastKnownPosition = Stimulus.StimulusLocation;
                break;
            }
        }
    }
}

void ACombat_TacticalAIController::UpdateTacticalState(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastTacticalUpdate >= TacticalUpdateInterval)
    {
        EvaluateThreats();
        UpdateTargetInformation();
        MakeTacticalDecision();
        
        LastTacticalUpdate = CurrentTime;
    }
}

void ACombat_TacticalAIController::EvaluateThreats()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old targets
    KnownTargets.RemoveAll([CurrentTime, this](const FCombat_TacticalTarget& Target)
    {
        return (CurrentTime - Target.LastSeenTime) > (1.0f / MemoryDecayRate);
    });
    
    // Update threat levels
    for (FCombat_TacticalTarget& Target : KnownTargets)
    {
        if (Target.TargetActor)
        {
            Target.ThreatLevel = static_cast<float>(EvaluateThreatLevel(Target.TargetActor));
            Target.bIsVisible = CanSeeTarget(Target.TargetActor);
            
            if (Target.bIsVisible)
            {
                Target.LastKnownPosition = Target.TargetActor->GetActorLocation();
                Target.LastSeenTime = CurrentTime;
            }
        }
    }
}

void ACombat_TacticalAIController::UpdateTargetInformation()
{
    // Update primary target information
    if (PrimaryTarget.TargetActor)
    {
        PrimaryTarget.bIsVisible = CanSeeTarget(PrimaryTarget.TargetActor);
        PrimaryTarget.ThreatLevel = static_cast<float>(EvaluateThreatLevel(PrimaryTarget.TargetActor));
        
        if (PrimaryTarget.bIsVisible)
        {
            PrimaryTarget.LastKnownPosition = PrimaryTarget.TargetActor->GetActorLocation();
            PrimaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
        }
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(BB_TargetLocation, PrimaryTarget.LastKnownPosition);
            BlackboardComponent->SetValueAsFloat(BB_ThreatLevel, PrimaryTarget.ThreatLevel);
        }
    }
}

void ACombat_TacticalAIController::MakeTacticalDecision()
{
    if (!GetPawn()) return;

    bool bShouldRetreat = ShouldRetreat();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(BB_ShouldRetreat, bShouldRetreat);
    }

    // State transitions based on tactical situation
    if (bShouldRetreat && CurrentState != ECombat_AIState::Retreating)
    {
        SetAIState(ECombat_AIState::Retreating);
    }
    else if (PrimaryTarget.TargetActor && PrimaryTarget.bIsVisible)
    {
        float Distance = GetDistanceToTarget(PrimaryTarget.TargetActor);
        
        if (Distance < 400.0f && CurrentState != ECombat_AIState::Attacking)
        {
            SetAIState(ECombat_AIState::Attacking);
        }
        else if (Distance < 800.0f && CurrentState != ECombat_AIState::Flanking)
        {
            SetAIState(ECombat_AIState::Flanking);
            
            // Calculate flanking position
            FVector FlankingPos = GetTacticalPosition(PrimaryTarget.TargetActor, 600.0f);
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(BB_FlankingPosition, FlankingPos);
            }
        }
        else if (CurrentState != ECombat_AIState::Hunting)
        {
            SetAIState(ECombat_AIState::Hunting);
        }
    }
    else if (KnownTargets.Num() > 0 && CurrentState != ECombat_AIState::Investigating)
    {
        SetAIState(ECombat_AIState::Investigating);
    }
    else if (KnownTargets.Num() == 0 && CurrentState != ECombat_AIState::Patrolling)
    {
        SetAIState(ECombat_AIState::Patrolling);
    }
}