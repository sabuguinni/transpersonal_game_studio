#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Blackboard Keys
const FName ACombatAIController::TargetActorKey = TEXT("TargetActor");
const FName ACombatAIController::TargetLocationKey = TEXT("TargetLocation");
const FName ACombatAIController::CombatStateKey = TEXT("CombatState");
const FName ACombatAIController::ThreatLevelKey = TEXT("ThreatLevel");
const FName ACombatAIController::LastKnownLocationKey = TEXT("LastKnownLocation");
const FName ACombatAIController::CanAttackKey = TEXT("CanAttack");
const FName ACombatAIController::ShouldRetreatKey = TEXT("ShouldRetreat");
const FName ACombatAIController::PreferredDistanceKey = TEXT("PreferredDistance");

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);

    // Set Sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Combat Parameters
    SightRange = 2000.0f;
    HearingRange = 1500.0f;
    PeripheralVisionAngle = 120.0f;
    AttackRange = 300.0f;
    AttackCooldown = 2.0f;
    PreferredCombatDistance = 500.0f;
    CirclingRadius = 800.0f;
    FleeDistance = 1500.0f;

    // Initialize Combat State
    CurrentCombatState = ECombatState::Idle;
    CombatRole = ECombatRole::SoloPredator;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind Perception Events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }

    // Start Behavior Tree
    if (CombatBehaviorTree && Blackboard)
    {
        UseBlackboard(CombatBlackboard);
        RunBehaviorTree(CombatBehaviorTree);

        // Initialize Blackboard Values
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));
            BlackboardComp->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(EThreatLevel::None));
            BlackboardComp->SetValueAsFloat(PreferredDistanceKey, PreferredCombatDistance);
            BlackboardComp->SetValueAsBool(CanAttackKey, false);
            BlackboardComp->SetValueAsBool(ShouldRetreatKey, false);
        }
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatState();
    ProcessCombatMemory();
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;

        // Update Blackboard
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));
        }

        // Execute state-specific logic
        switch (NewState)
        {
        case ECombatState::Hunting:
            // Start active search for target
            break;
        case ECombatState::Stalking:
            // Begin stealth approach
            break;
        case ECombatState::Attacking:
            // Initiate attack sequence
            break;
        case ECombatState::Retreating:
            ExecuteRetreat();
            break;
        case ECombatState::Circling:
            ExecuteCircling();
            break;
        case ECombatState::Ambushing:
            ExecuteAmbush();
            break;
        case ECombatState::Fleeing:
            // Emergency escape
            break;
        }
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject(TargetActorKey, NewTarget);

        if (NewTarget)
        {
            FVector TargetLocation = NewTarget->GetActorLocation();
            BlackboardComp->SetValueAsVector(TargetLocationKey, TargetLocation);
            UpdateTargetMemory(NewTarget, TargetLocation);
        }
    }
}

AActor* ACombatAIController::GetCurrentTarget() const
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        return Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey));
    }
    return nullptr;
}

void ACombatAIController::UpdateTargetMemory(AActor* Target, const FVector& Location)
{
    if (Target)
    {
        CombatMemory.LastKnownTarget = Target;
        CombatMemory.LastKnownLocation = Location;
        CombatMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        CombatMemory.ThreatLevel = AssessThreatLevel(Target);

        // Update Blackboard
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(LastKnownLocationKey, Location);
            BlackboardComp->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(CombatMemory.ThreatLevel));
        }
    }
}

bool ACombatAIController::ShouldAttack() const
{
    AActor* Target = GetCurrentTarget();
    if (!Target || !GetPawn())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    bool bInRange = DistanceToTarget <= AttackRange;
    bool bCooldownReady = (GetWorld()->GetTimeSeconds() - CombatMemory.LastAttackTime) >= AttackCooldown;
    bool bNotRetreating = CurrentCombatState != ECombatState::Retreating && CurrentCombatState != ECombatState::Fleeing;

    return bInRange && bCooldownReady && bNotRetreating;
}

bool ACombatAIController::ShouldRetreat() const
{
    // Retreat if heavily damaged or outnumbered
    bool bHeavyDamage = CombatMemory.DamageReceived > 50.0f;
    bool bMultipleFailedAttacks = CombatMemory.AttackAttempts > 3;
    bool bHighThreat = CombatMemory.ThreatLevel == EThreatLevel::Critical;

    return bHeavyDamage || bMultipleFailedAttacks || bHighThreat;
}

bool ACombatAIController::ShouldCircle() const
{
    AActor* Target = GetCurrentTarget();
    if (!Target || !GetPawn())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    bool bInCirclingRange = DistanceToTarget > AttackRange && DistanceToTarget < CirclingRadius;
    bool bCanSeeTarget = AIPerceptionComponent && AIPerceptionComponent->GetActorsPerception(Target).Num() > 0;

    return bInCirclingRange && bCanSeeTarget && !ShouldRetreat();
}

bool ACombatAIController::ShouldAmbush() const
{
    // Ambush if we lost sight of target but know their general location
    bool bLostSight = CombatMemory.LastKnownTarget && !GetCurrentTarget();
    bool bRecentMemory = (GetWorld()->GetTimeSeconds() - CombatMemory.LastSeenTime) < 5.0f;
    bool bAmbushRole = CombatRole == ECombatRole::Ambusher;

    return (bLostSight && bRecentMemory) || bAmbushRole;
}

void ACombatAIController::OnDamageReceived(float DamageAmount, AActor* DamageSource)
{
    CombatMemory.bHasBeenDamaged = true;
    CombatMemory.DamageReceived += DamageAmount;

    // Set damage source as target if we don't have one
    if (!GetCurrentTarget() && DamageSource)
    {
        SetTarget(DamageSource);
        SetCombatState(ECombatState::Hunting);
    }

    // Consider retreating if damage is significant
    if (DamageAmount > 30.0f)
    {
        UpdateThreatLevel();
    }
}

EThreatLevel ACombatAIController::AssessThreatLevel(AActor* Target) const
{
    if (!Target)
    {
        return EThreatLevel::None;
    }

    // Basic threat assessment - can be expanded based on target type, equipment, etc.
    if (Target->IsA<ACharacter>())
    {
        // Player character is always high threat
        if (Cast<APlayerController>(Cast<ACharacter>(Target)->GetController()))
        {
            return EThreatLevel::High;
        }
        return EThreatLevel::Medium;
    }

    return EThreatLevel::Low;
}

void ACombatAIController::UpdateThreatLevel()
{
    AActor* Target = GetCurrentTarget();
    if (Target)
    {
        CombatMemory.ThreatLevel = AssessThreatLevel(Target);

        // Update Blackboard
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsEnum(ThreatLevelKey, static_cast<uint8>(CombatMemory.ThreatLevel));
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is a player character
            if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                if (Cast<APlayerController>(Character->GetController()))
                {
                    SetTarget(Actor);
                    SetCombatState(ECombatState::Hunting);
                    break;
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor && Stimulus.WasSuccessfullySensed())
    {
        UpdateTargetMemory(Actor, Stimulus.StimulusLocation);

        // React based on stimulus type
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            SetCombatState(ECombatState::Stalking);
        }
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            SetCombatState(ECombatState::Hunting);
        }
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
        {
            SetCombatState(ECombatState::Attacking);
        }
    }
    else if (Actor == GetCurrentTarget())
    {
        // Lost sight of current target
        SetCombatState(ECombatState::Hunting);
    }
}

void ACombatAIController::UpdateCombatState()
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        // Update combat decision flags
        BlackboardComp->SetValueAsBool(CanAttackKey, ShouldAttack());
        BlackboardComp->SetValueAsBool(ShouldRetreatKey, ShouldRetreat());
    }
}

void ACombatAIController::ProcessCombatMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Decay threat level over time if no recent contact
    if (CurrentTime - CombatMemory.LastSeenTime > 10.0f)
    {
        if (CombatMemory.ThreatLevel > EThreatLevel::None)
        {
            CombatMemory.ThreatLevel = static_cast<EThreatLevel>(static_cast<uint8>(CombatMemory.ThreatLevel) - 1);
        }
    }

    // Reset damage over time (natural healing)
    if (CombatMemory.DamageReceived > 0.0f)
    {
        CombatMemory.DamageReceived = FMath::Max(0.0f, CombatMemory.DamageReceived - (10.0f * GetWorld()->GetDeltaSeconds()));
    }
}

FVector ACombatAIController::CalculateCirclingPosition() const
{
    AActor* Target = GetCurrentTarget();
    if (!Target || !GetPawn())
    {
        return GetPawn()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();

    // Calculate perpendicular vector for circling
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Alternate circling direction based on time
    float Time = GetWorld()->GetTimeSeconds();
    float Direction = FMath::Sin(Time * 0.5f) > 0 ? 1.0f : -1.0f;

    FVector CirclingDirection = (Right * Direction + ToTarget * 0.3f).GetSafeNormal();
    return TargetLocation + CirclingDirection * CirclingRadius;
}

FVector ACombatAIController::CalculateAmbushPosition() const
{
    if (CombatMemory.LastKnownLocation != FVector::ZeroVector)
    {
        // Position between last known location and predicted path
        FVector PredictedLocation = CombatMemory.LastKnownLocation;
        FVector AmbushDirection = (PredictedLocation - GetPawn()->GetActorLocation()).GetSafeNormal();
        return PredictedLocation - AmbushDirection * 200.0f;
    }

    return GetPawn()->GetActorLocation();
}

FVector ACombatAIController::CalculateRetreatPosition() const
{
    AActor* Target = GetCurrentTarget();
    if (!Target || !GetPawn())
    {
        return GetPawn()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector AwayFromTarget = (CurrentLocation - TargetLocation).GetSafeNormal();

    return CurrentLocation + AwayFromTarget * FleeDistance;
}