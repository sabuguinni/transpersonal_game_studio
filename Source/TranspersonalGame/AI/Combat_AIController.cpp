#include "Combat_AIController.h"
#include "Combat_BehaviorTreeManager.h"
#include "Combat_DinosaurPawn.h"
#include "TranspersonalCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Perception - Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure AI Perception - Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Combat State
    TacticalState.CurrentBehaviorState = ECombat_AIBehaviorState::Idle;
    TacticalState.FormationRole = ECombat_FormationRole::Solo;
    TacticalState.AggressionLevel = 0.5f;
    TacticalState.FearLevel = 0.0f;
    TacticalState.bInCombat = false;
    TacticalState.CombatTimer = 0.0f;

    // Initialize Threat Assessment
    CurrentThreat.ThreatLevel = 0.0f;
    CurrentThreat.Distance = 0.0f;
    CurrentThreat.LastKnownLocation = FVector::ZeroVector;
    CurrentThreat.TimeSinceLastSeen = 0.0f;
    CurrentThreat.bIsPlayerTarget = false;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Initialize Behavior Tree Manager
    BehaviorTreeManager = NewObject<UCombat_BehaviorTreeManager>(this);
    if (BehaviorTreeManager)
    {
        BehaviorTreeManager->Initialize(this);
    }

    // Bind Perception Events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }

    // Start Behavior Tree if available
    if (BehaviorTreeManager && BehaviorTreeManager->GetDefaultBehaviorTree())
    {
        RunBehaviorTree(BehaviorTreeManager->GetDefaultBehaviorTree());
    }
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatState(DeltaTime);
}

void ACombat_AIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent && BehaviorTreeManager)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTreeManager->GetBlackboardAsset());
    }
}

void ACombat_AIController::SetBehaviorState(ECombat_AIBehaviorState NewState)
{
    if (TacticalState.CurrentBehaviorState != NewState)
    {
        TacticalState.CurrentBehaviorState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }

        UE_LOG(LogTemp, Log, TEXT("Combat AI: Behavior state changed to %d"), static_cast<int32>(NewState));
    }
}

void ACombat_AIController::AssignFormationRole(ECombat_FormationRole NewRole)
{
    TacticalState.FormationRole = NewRole;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("FormationRole"), static_cast<uint8>(NewRole));
    }
}

void ACombat_AIController::UpdateThreatAssessment(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }

    CurrentThreat.ThreatLevel = ThreatLevel;
    CurrentThreat.Distance = FVector::Dist(GetPawn()->GetActorLocation(), ThreatActor->GetActorLocation());
    CurrentThreat.LastKnownLocation = ThreatActor->GetActorLocation();
    CurrentThreat.TimeSinceLastSeen = 0.0f;
    CurrentThreat.bIsPlayerTarget = ThreatActor->IsA<ATranspersonalCharacter>();

    PrimaryTarget = ThreatActor;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), ThreatActor);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), ThreatActor->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), ThreatLevel);
    }
}

void ACombat_AIController::ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver)
{
    if (!GetPawn() || !PrimaryTarget)
    {
        return;
    }

    FVector TargetLocation = FVector::ZeroVector;

    switch (Maneuver)
    {
        case ECombat_TacticalManeuver::DirectAssault:
            TargetLocation = PrimaryTarget->GetActorLocation();
            SetBehaviorState(ECombat_AIBehaviorState::Attacking);
            break;

        case ECombat_TacticalManeuver::FlankLeft:
        case ECombat_TacticalManeuver::FlankRight:
            TargetLocation = CalculateFlankingPosition(PrimaryTarget);
            SetBehaviorState(ECombat_AIBehaviorState::Flanking);
            break;

        case ECombat_TacticalManeuver::Retreat:
            TargetLocation = FindOptimalCoverPosition();
            SetBehaviorState(ECombat_AIBehaviorState::Retreating);
            break;

        case ECombat_TacticalManeuver::Ambush:
            SetBehaviorState(ECombat_AIBehaviorState::Ambushing);
            break;

        case ECombat_TacticalManeuver::Distraction:
            SetBehaviorState(ECombat_AIBehaviorState::Distracting);
            break;
    }

    if (BlackboardComponent && !TargetLocation.IsZero())
    {
        BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), TargetLocation);
    }
}

bool ACombat_AIController::CanExecuteAttack() const
{
    if (!GetPawn() || !PrimaryTarget)
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation());
    return DistanceToTarget <= AttackRange && TacticalState.bInCombat;
}

FVector ACombat_AIController::CalculateFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomize left or right flanking
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }

    return TargetLocation + (FlankDirection * FlankingDistance);
}

void ACombat_AIController::RegisterAlliedUnit(AActor* AllyActor)
{
    if (AllyActor && !AlliedUnits.Contains(AllyActor))
    {
        AlliedUnits.Add(AllyActor);
    }
}

void ACombat_AIController::UnregisterAlliedUnit(AActor* AllyActor)
{
    AlliedUnits.Remove(AllyActor);
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ATranspersonalCharacter>())
        {
            float ThreatLevel = 1.0f; // Player is always high threat
            UpdateThreatAssessment(Actor, ThreatLevel);
            break;
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
        if (Actor->IsA<ATranspersonalCharacter>())
        {
            UpdateThreatAssessment(Actor, 1.0f);
            SetBehaviorState(ECombat_AIBehaviorState::Alert);
        }
    }
    else
    {
        // Lost sight of target
        CurrentThreat.TimeSinceLastSeen = 0.0f;
        if (TacticalState.CurrentBehaviorState == ECombat_AIBehaviorState::Alert)
        {
            SetBehaviorState(ECombat_AIBehaviorState::Investigating);
        }
    }
}

void ACombat_AIController::UpdateCombatState(float DeltaTime)
{
    LastThreatUpdateTime += DeltaTime;
    LastCommunicationTime += DeltaTime;
    TacticalState.CombatTimer += DeltaTime;
    CurrentThreat.TimeSinceLastSeen += DeltaTime;

    if (LastThreatUpdateTime >= CombatStateUpdateInterval)
    {
        ProcessThreatAssessment();
        ExecuteCombatBehavior();
        LastThreatUpdateTime = 0.0f;
    }

    if (LastCommunicationTime >= CommunicationInterval)
    {
        HandleCombatCommunication();
        LastCommunicationTime = 0.0f;
    }
}

void ACombat_AIController::ProcessThreatAssessment()
{
    if (!PrimaryTarget)
    {
        TacticalState.bInCombat = false;
        if (TacticalState.CurrentBehaviorState != ECombat_AIBehaviorState::Idle)
        {
            SetBehaviorState(ECombat_AIBehaviorState::Idle);
        }
        return;
    }

    // Update threat distance
    CurrentThreat.Distance = FVector::Dist(GetPawn()->GetActorLocation(), PrimaryTarget->GetActorLocation());

    // Determine if in combat range
    bool bWasInCombat = TacticalState.bInCombat;
    TacticalState.bInCombat = CurrentThreat.Distance <= (AttackRange * 2.0f);

    // Update aggression and fear based on threat assessment
    if (TacticalState.bInCombat)
    {
        TacticalState.AggressionLevel = FMath::Clamp(TacticalState.AggressionLevel + 0.1f, 0.0f, 1.0f);
        TacticalState.FearLevel = FMath::Clamp(CurrentThreat.ThreatLevel - 0.5f, 0.0f, 1.0f);
    }
    else
    {
        TacticalState.AggressionLevel = FMath::Clamp(TacticalState.AggressionLevel - 0.05f, 0.0f, 1.0f);
        TacticalState.FearLevel = FMath::Clamp(TacticalState.FearLevel - 0.1f, 0.0f, 1.0f);
    }

    // State transitions based on combat status
    if (!bWasInCombat && TacticalState.bInCombat)
    {
        SetBehaviorState(ECombat_AIBehaviorState::Engaging);
    }
    else if (bWasInCombat && !TacticalState.bInCombat && CurrentThreat.TimeSinceLastSeen > 5.0f)
    {
        SetBehaviorState(ECombat_AIBehaviorState::Idle);
    }
}

void ACombat_AIController::ExecuteCombatBehavior()
{
    if (!TacticalState.bInCombat || !PrimaryTarget)
    {
        return;
    }

    ECombat_TacticalManeuver OptimalManeuver = SelectOptimalManeuver();
    ExecuteTacticalManeuver(OptimalManeuver);
}

void ACombat_AIController::UpdateFormationPosition()
{
    if (TacticalState.FormationRole == ECombat_FormationRole::Solo)
    {
        return;
    }

    FVector FormationPosition = CalculateFormationPosition();
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("FormationPosition"), FormationPosition);
    }
}

void ACombat_AIController::HandleCombatCommunication()
{
    // Placeholder for combat communication system
    // This would integrate with audio system for tactical callouts
}

ECombat_TacticalManeuver ACombat_AIController::SelectOptimalManeuver() const
{
    if (ShouldRetreat())
    {
        return ECombat_TacticalManeuver::Retreat;
    }

    if (CurrentThreat.Distance <= AttackRange)
    {
        return ECombat_TacticalManeuver::DirectAssault;
    }

    // Formation-based maneuver selection
    switch (TacticalState.FormationRole)
    {
        case ECombat_FormationRole::Alpha:
            return ECombat_TacticalManeuver::DirectAssault;
        case ECombat_FormationRole::Flanker:
            return FMath::RandBool() ? ECombat_TacticalManeuver::FlankLeft : ECombat_TacticalManeuver::FlankRight;
        case ECombat_FormationRole::Distractor:
            return ECombat_TacticalManeuver::Distraction;
        case ECombat_FormationRole::Ambusher:
            return ECombat_TacticalManeuver::Ambush;
        default:
            return ECombat_TacticalManeuver::DirectAssault;
    }
}

bool ACombat_AIController::ShouldEngageTarget() const
{
    return TacticalState.AggressionLevel > 0.6f && TacticalState.FearLevel < 0.4f;
}

bool ACombat_AIController::ShouldRetreat() const
{
    return TacticalState.FearLevel > RetreatThreshold || 
           (CurrentThreat.ThreatLevel > 0.8f && TacticalState.AggressionLevel < 0.3f);
}

FVector ACombat_AIController::FindOptimalCoverPosition() const
{
    if (!GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector RetreatDirection = -GetPawn()->GetActorForwardVector();
    
    return MyLocation + (RetreatDirection * 1000.0f);
}

void ACombat_AIController::UpdateFormationRole()
{
    // Dynamic role assignment based on combat situation
    if (AlliedUnits.Num() <= 1)
    {
        AssignFormationRole(ECombat_FormationRole::Solo);
    }
    else if (TacticalState.FormationRole == ECombat_FormationRole::Solo)
    {
        // Assign role based on position in pack
        AssignFormationRole(ECombat_FormationRole::Flanker);
    }
}

FVector ACombat_AIController::CalculateFormationPosition() const
{
    if (!PrimaryTarget || AlliedUnits.Num() == 0)
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    
    switch (TacticalState.FormationRole)
    {
        case ECombat_FormationRole::Alpha:
            return TargetLocation + (GetPawn()->GetActorForwardVector() * -AttackRange);
        case ECombat_FormationRole::Flanker:
            return CalculateFlankingPosition(PrimaryTarget);
        default:
            return GetPawn()->GetActorLocation();
    }
}

void ACombat_AIController::CoordinateWithAllies()
{
    // Placeholder for ally coordination system
    for (AActor* Ally : AlliedUnits)
    {
        if (ACombat_AIController* AllyController = Cast<ACombat_AIController>(Cast<APawn>(Ally)->GetController()))
        {
            // Share threat information
            if (PrimaryTarget)
            {
                AllyController->UpdateThreatAssessment(PrimaryTarget, CurrentThreat.ThreatLevel);
            }
        }
    }
}