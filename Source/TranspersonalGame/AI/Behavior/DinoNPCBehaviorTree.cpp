#include "DinoNPCBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ADinoNPCBehaviorTree::ADinoNPCBehaviorTree()
{
    PrimaryActorTick.bCanEverTick = true;

    // Behavior Tree component
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp   = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

    // Perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                = PerceptionData.SightRadius;
    SightConfig->LoseSightRadius            = PerceptionData.LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PerceptionData.PeripheralVisionAngle;
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = PerceptionData.HearingRange;
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initial state
    CurrentAlertState   = ENPC_DinoAlertState::Calm;
    CurrentBehaviorMode = ENPC_DinoBehaviorMode::Patrol;
    CurrentAlertLevel   = 0.f;
    CurrentTarget       = nullptr;
    PatrolOrigin        = FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// OnPossess
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Record patrol origin from pawn spawn location
    if (InPawn)
    {
        PatrolOrigin = InPawn->GetActorLocation();
    }

    // Bind perception delegate
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &ADinoNPCBehaviorTree::OnTargetPerceptionUpdated);

    // Initialize blackboard and start behavior tree
    if (BehaviorTreeAsset)
    {
        if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp))
        {
            BlackboardComp->SetValueAsVector(BB_PatrolOrigin, PatrolOrigin);
            BlackboardComp->SetValueAsFloat(BB_AlertLevel, 0.f);
            BlackboardComp->SetValueAsBool(BB_bCanSeeTarget, false);
            BlackboardComp->SetValueAsBool(BB_bIsAttacking, false);
            BlackboardComp->SetValueAsBool(BB_bIsDead, false);
            RunBehaviorTree(BehaviorTreeAsset);
        }
    }
    else
    {
        // No BT asset assigned — still update blackboard manually each tick
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsVector(BB_PatrolOrigin, PatrolOrigin);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnUnPossess
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::OnUnPossess()
{
    Super::OnUnPossess();
    PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
        this, &ADinoNPCBehaviorTree::OnTargetPerceptionUpdated);
    BehaviorTreeComp->StopTree();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentAlertState == ENPC_DinoAlertState::Dead)
    {
        return;
    }

    // Decay alert when no active target
    if (!CurrentTarget)
    {
        DecayAlertLevel(DeltaTime);
    }
    else
    {
        // Verify target is still in chase range; clear if not
        if (!IsTargetInChaseRange())
        {
            ClearTarget();
        }
    }

    UpdateAlertState();
    UpdateBlackboard();
}

// ─────────────────────────────────────────────────────────────────────────────
// Perception callback
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || CurrentAlertState == ENPC_DinoAlertState::Dead)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Sight stimulus raises alert significantly
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            SetAlertLevel(CurrentAlertLevel + 1.5f);
            SetTarget(Actor);
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsBool(BB_bCanSeeTarget, true);
                BlackboardComp->SetValueAsVector(BB_LastKnownPos, Actor->GetActorLocation());
            }
        }
        // Hearing stimulus raises alert moderately
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            SetAlertLevel(CurrentAlertLevel + 0.8f);
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(BB_LastKnownPos, Stimulus.StimulusLocation);
            }
            // Only set target from hearing if not already chasing
            if (!CurrentTarget && CurrentAlertLevel >= PerceptionData.AlertThresholdChase)
            {
                SetTarget(Actor);
            }
        }
    }
    else
    {
        // Lost sight — keep alert but clear direct sight flag
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsBool(BB_bCanSeeTarget, false);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetAlertLevel
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::SetAlertLevel(float NewLevel)
{
    CurrentAlertLevel = FMath::Clamp(NewLevel, 0.f, 3.f);
    UpdateAlertState();
    UpdateBlackboard();
}

// ─────────────────────────────────────────────────────────────────────────────
// SetTarget
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (BlackboardComp && NewTarget)
    {
        BlackboardComp->SetValueAsObject(BB_TargetActor, NewTarget);
        BlackboardComp->SetValueAsVector(BB_LastKnownPos, NewTarget->GetActorLocation());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ClearTarget
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::ClearTarget()
{
    CurrentTarget = nullptr;
    if (BlackboardComp)
    {
        BlackboardComp->ClearValue(BB_TargetActor);
        BlackboardComp->SetValueAsBool(BB_bCanSeeTarget, false);
        BlackboardComp->SetValueAsBool(BB_bIsAttacking, false);
    }
    // Transition to investigate mode — move to last known position
    CurrentBehaviorMode = ENPC_DinoBehaviorMode::Investigate;
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerDeath
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::TriggerDeath()
{
    CurrentAlertState   = ENPC_DinoAlertState::Dead;
    CurrentBehaviorMode = ENPC_DinoBehaviorMode::Idle;
    CurrentTarget       = nullptr;

    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(BB_bIsDead, true);
        BlackboardComp->SetValueAsBool(BB_bIsAttacking, false);
        BlackboardComp->ClearValue(BB_TargetActor);
    }

    BehaviorTreeComp->StopTree(EBTStopMode::Safe);
    StopMovement();
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerFlee
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::TriggerFlee(FVector FleeDirection)
{
    CurrentBehaviorMode = ENPC_DinoBehaviorMode::Flee;
    ClearTarget();
    SetAlertLevel(1.0f);

    if (GetPawn())
    {
        FVector FleeTarget = GetPawn()->GetActorLocation() + FleeDirection.GetSafeNormal() * 5000.f;
        MoveToLocation(FleeTarget, 50.f, false, true, false, true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// IsTargetInAttackRange
// ─────────────────────────────────────────────────────────────────────────────
bool ADinoNPCBehaviorTree::IsTargetInAttackRange() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    float Dist = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= CombatData.AttackRange;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsTargetInChaseRange
// ─────────────────────────────────────────────────────────────────────────────
bool ADinoNPCBehaviorTree::IsTargetInChaseRange() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    float Dist = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= CombatData.MaxChaseDistance;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAlertState — maps float alert level to enum state
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::UpdateAlertState()
{
    ENPC_DinoAlertState NewState = AlertLevelToState(CurrentAlertLevel);
    if (NewState == CurrentAlertState)
    {
        return;
    }

    CurrentAlertState = NewState;

    // Update behavior mode based on alert state
    switch (CurrentAlertState)
    {
        case ENPC_DinoAlertState::Calm:
            CurrentBehaviorMode = ENPC_DinoBehaviorMode::Patrol;
            break;
        case ENPC_DinoAlertState::Curious:
            CurrentBehaviorMode = ENPC_DinoBehaviorMode::Investigate;
            break;
        case ENPC_DinoAlertState::Alert:
            CurrentBehaviorMode = (CurrentTarget != nullptr)
                ? ENPC_DinoBehaviorMode::Chase
                : ENPC_DinoBehaviorMode::Investigate;
            break;
        case ENPC_DinoAlertState::Aggressive:
            CurrentBehaviorMode = IsTargetInAttackRange()
                ? ENPC_DinoBehaviorMode::Attack
                : ENPC_DinoBehaviorMode::Chase;
            break;
        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateBlackboard — push runtime state to blackboard each tick
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::UpdateBlackboard()
{
    if (!BlackboardComp)
    {
        return;
    }

    BlackboardComp->SetValueAsFloat(BB_AlertLevel, CurrentAlertLevel);

    bool bAttacking = (CurrentBehaviorMode == ENPC_DinoBehaviorMode::Attack);
    BlackboardComp->SetValueAsBool(BB_bIsAttacking, bAttacking);

    if (CurrentTarget)
    {
        BlackboardComp->SetValueAsVector(BB_LastKnownPos, CurrentTarget->GetActorLocation());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DecayAlertLevel — calm down over time when no stimulus
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::DecayAlertLevel(float DeltaTime)
{
    if (CurrentAlertLevel > 0.f)
    {
        CurrentAlertLevel = FMath::Max(0.f,
            CurrentAlertLevel - PerceptionData.AlertDecayRate * DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AlertLevelToState
// ─────────────────────────────────────────────────────────────────────────────
ENPC_DinoAlertState ADinoNPCBehaviorTree::AlertLevelToState(float Level) const
{
    if (Level <= 0.f)   return ENPC_DinoAlertState::Calm;
    if (Level < 1.0f)   return ENPC_DinoAlertState::Curious;
    if (Level < 2.0f)   return ENPC_DinoAlertState::Alert;
    return ENPC_DinoAlertState::Aggressive;
}

// ─────────────────────────────────────────────────────────────────────────────
// SetupPerception — called if perception needs runtime reconfiguration
// ─────────────────────────────────────────────────────────────────────────────
void ADinoNPCBehaviorTree::SetupPerception()
{
    if (SightConfig)
    {
        SightConfig->SightRadius                  = PerceptionData.SightRadius;
        SightConfig->LoseSightRadius              = PerceptionData.LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PerceptionData.PeripheralVisionAngle;
    }
    if (HearingConfig)
    {
        HearingConfig->HearingRange = PerceptionData.HearingRange;
    }
    if (PerceptionComp)
    {
        PerceptionComp->RequestStimuliListenerUpdate();
    }
}
