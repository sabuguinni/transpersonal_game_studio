#include "Combat_CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombat_CombatAIController::ACombat_CombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRange;
        SightConfig->LoseSightRadius = DetectionRange + 500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Initialize Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = DetectionRange * 0.8f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Initialize Combat State
    CurrentCombatState = ECombat_AIState::Idle;
    CombatTarget = nullptr;
    LastCombatTime = 0.0f;
    LastPerceptionUpdate = 0.0f;
    TerritoryCenter = FVector::ZeroVector;
}

void ACombat_CombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_CombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_CombatAIController::OnTargetPerceptionUpdated);
    }

    // Set territory center to current location
    if (GetPawn())
    {
        TerritoryCenter = GetPawn()->GetActorLocation();
    }

    // Initialize blackboard
    InitializeBlackboard();
}

void ACombat_CombatAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (CombatBehaviorTree && BehaviorTreeComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombat_CombatAIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UpdateBlackboardValues();

        // Handle state transitions
        switch (NewState)
        {
        case ECombat_AIState::Combat:
            LastCombatTime = GetWorld()->GetTimeSeconds();
            break;
        case ECombat_AIState::Idle:
            CombatTarget = nullptr;
            break;
        case ECombat_AIState::Alert:
            // Increase perception range temporarily
            if (SightConfig)
            {
                SightConfig->SightRadius = DetectionRange * 1.5f;
            }
            break;
        }
    }
}

void ACombat_CombatAIController::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;
    UpdateBlackboardValues();

    if (NewTarget)
    {
        SetCombatState(ECombat_AIState::Combat);
    }
    else if (CurrentCombatState == ECombat_AIState::Combat)
    {
        SetCombatState(ECombat_AIState::Alert);
    }
}

bool ACombat_CombatAIController::IsInCombat() const
{
    return CurrentCombatState == ECombat_AIState::Combat && CombatTarget != nullptr;
}

bool ACombat_CombatAIController::CanAttackTarget() const
{
    if (!CombatTarget || !GetPawn())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatTarget->GetActorLocation());
    return DistanceToTarget <= AttackRange;
}

bool ACombat_CombatAIController::IsTargetInRange(float Range) const
{
    if (!CombatTarget || !GetPawn())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatTarget->GetActorLocation());
    return DistanceToTarget <= Range;
}

bool ACombat_CombatAIController::IsInTerritory() const
{
    if (!GetPawn())
    {
        return true;
    }

    float DistanceFromCenter = FVector::Dist(GetPawn()->GetActorLocation(), TerritoryCenter);
    return DistanceFromCenter <= TerritoryRadius;
}

void ACombat_CombatAIController::StartCombat(AActor* Target)
{
    SetCombatTarget(Target);
    SetCombatState(ECombat_AIState::Combat);

    // Notify pack members if pack hunter
    if (bIsPackHunter)
    {
        CoordinateWithPack();
    }
}

void ACombat_CombatAIController::EndCombat()
{
    SetCombatTarget(nullptr);
    SetCombatState(ECombat_AIState::Idle);

    // Reset perception range
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRange;
    }
}

void ACombat_CombatAIController::ExecuteAttack()
{
    if (!CanAttackTarget())
    {
        return;
    }

    // Basic attack implementation
    // This would trigger animations and damage in a full implementation
    UE_LOG(LogTemp, Warning, TEXT("Combat AI executing attack on target"));
}

void ACombat_CombatAIController::ExecuteDefense()
{
    // Basic defense implementation
    SetCombatState(ECombat_AIState::Defensive);
    UE_LOG(LogTemp, Warning, TEXT("Combat AI executing defensive maneuver"));
}

void ACombat_CombatAIController::ExecuteRetreat()
{
    // Basic retreat implementation
    SetCombatState(ECombat_AIState::Fleeing);
    
    if (GetPawn() && bIsTerritorial)
    {
        // Move towards territory center
        FVector RetreatDirection = (TerritoryCenter - GetPawn()->GetActorLocation()).GetSafeNormal();
        FVector RetreatLocation = GetPawn()->GetActorLocation() + (RetreatDirection * 1000.0f);
        MoveToLocation(RetreatLocation);
    }
}

void ACombat_CombatAIController::CoordinateWithPack()
{
    if (!bIsPackHunter)
    {
        return;
    }

    // Find nearby pack members and coordinate attack
    for (AActor* PackMember : PackMembers)
    {
        if (PackMember && IsValid(PackMember))
        {
            ACombat_CombatAIController* PackController = Cast<ACombat_CombatAIController>(PackMember->GetInstigatorController());
            if (PackController && PackController->GetCombatState() != ECombat_AIState::Combat)
            {
                PackController->StartCombat(CombatTarget);
            }
        }
    }
}

void ACombat_CombatAIController::CallForBackup()
{
    // Implementation for calling nearby allies
    UE_LOG(LogTemp, Warning, TEXT("Combat AI calling for backup"));
}

void ACombat_CombatAIController::SetTerritory(FVector Center, float Radius)
{
    TerritoryCenter = Center;
    TerritoryRadius = Radius;
    UpdateBlackboardValues();
}

void ACombat_CombatAIController::DefendTerritory()
{
    if (!bIsTerritorial)
    {
        return;
    }

    // Check if there are intruders in territory
    for (AActor* Threat : NearbyThreats)
    {
        if (Threat && IsValid(Threat))
        {
            float DistanceFromCenter = FVector::Dist(Threat->GetActorLocation(), TerritoryCenter);
            if (DistanceFromCenter <= TerritoryRadius)
            {
                StartCombat(Threat);
                break;
            }
        }
    }
}

void ACombat_CombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    LastPerceptionUpdate = GetWorld()->GetTimeSeconds();
    
    // Update nearby threats list
    NearbyThreats.Empty();
    
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && IsValid(Actor) && Actor != GetPawn())
        {
            // Simple threat detection - could be expanded with faction system
            if (Actor->IsA<APawn>())
            {
                NearbyThreats.Add(Actor);
            }
        }
    }

    UpdateCombatDecision();
}

void ACombat_CombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (CurrentCombatState == ECombat_AIState::Idle)
        {
            SetCombatState(ECombat_AIState::Alert);
        }

        // If aggressive enough and no current target, engage
        if (!CombatTarget && AggressionLevel > 0.6f)
        {
            StartCombat(Actor);
        }
    }
    else
    {
        // Target lost
        if (CombatTarget == Actor)
        {
            SetCombatTarget(nullptr);
        }
    }
}

void ACombat_CombatAIController::UpdateCombatDecision()
{
    if (NearbyThreats.Num() == 0)
    {
        if (CurrentCombatState != ECombat_AIState::Idle)
        {
            SetCombatState(ECombat_AIState::Idle);
        }
        return;
    }

    EvaluateThreatLevel();
    SelectBestTarget();
}

void ACombat_CombatAIController::EvaluateThreatLevel()
{
    // Simple threat evaluation based on number of nearby threats
    float ThreatLevel = FMath::Clamp(NearbyThreats.Num() / 5.0f, 0.0f, 1.0f);
    
    if (ThreatLevel > 0.8f && AggressionLevel < 0.3f)
    {
        // High threat, low aggression - consider retreat
        if (CurrentCombatState == ECombat_AIState::Combat)
        {
            ExecuteRetreat();
        }
    }
}

void ACombat_CombatAIController::SelectBestTarget()
{
    if (NearbyThreats.Num() == 0 || !GetPawn())
    {
        return;
    }

    AActor* BestTarget = nullptr;
    float BestScore = 0.0f;

    for (AActor* Threat : NearbyThreats)
    {
        if (!Threat || !IsValid(Threat))
        {
            continue;
        }

        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation());
        float Score = 1.0f / (Distance + 1.0f); // Closer targets have higher score

        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Threat;
        }
    }

    if (BestTarget && BestTarget != CombatTarget)
    {
        StartCombat(BestTarget);
    }
}

void ACombat_CombatAIController::InitializeBlackboard()
{
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        Blackboard->SetValueAsObject(TEXT("CombatTarget"), CombatTarget);
        Blackboard->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        Blackboard->SetValueAsFloat(TEXT("TerritoryRadius"), TerritoryRadius);
        Blackboard->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
        Blackboard->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        Blackboard->SetValueAsBool(TEXT("IsPackHunter"), bIsPackHunter);
        Blackboard->SetValueAsBool(TEXT("IsTerritorial"), bIsTerritorial);
    }
}

void ACombat_CombatAIController::UpdateBlackboardValues()
{
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        Blackboard->SetValueAsObject(TEXT("CombatTarget"), CombatTarget);
        Blackboard->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        Blackboard->SetValueAsBool(TEXT("CanAttackTarget"), CanAttackTarget());
        Blackboard->SetValueAsBool(TEXT("IsInTerritory"), IsInTerritory());
        Blackboard->SetValueAsInt(TEXT("NearbyThreatsCount"), NearbyThreats.Num());
    }
}