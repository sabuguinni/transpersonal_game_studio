#include "Combat_CombatAIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_CombatAIController::ACombat_CombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(5.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Add configurations to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize combat parameters
    CurrentCombatState = ECombat_CombatState::Idle;
    CombatRange = 1500.0f;
    AttackRange = 300.0f;
    RetreatHealthThreshold = 0.3f;
    ThreatForgetTime = 30.0f;
    AttackCooldown = 3.0f;
    LastAttackTime = 0.0f;
    bCanFlank = true;
    bCanAmbush = false;
    CurrentTarget = nullptr;

    // Initialize Blackboard and Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ACombat_CombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_CombatAIController::OnPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombat_CombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateThreatAssessment();
    UpdateCombatBehavior(DeltaTime);
    CleanupOldThreats();
}

void ACombat_CombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent && BehaviorTree)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
    }
}

void ACombat_CombatAIController::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_CombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Combat AI State Changed: %d -> %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ACombat_CombatAIController::AddThreat(AActor* ThreatActor, ECombat_ThreatLevel Level)
{
    if (!ThreatActor)
        return;

    // Check if threat already exists
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor == ThreatActor)
        {
            // Update existing threat
            Threat.ThreatLevel = Level;
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            Threat.LastKnownLocation = ThreatActor->GetActorLocation();
            Threat.Distance = GetThreatDistance(ThreatActor);
            return;
        }
    }

    // Add new threat
    FCombat_ThreatData NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = Level;
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewThreat.LastKnownLocation = ThreatActor->GetActorLocation();
    NewThreat.Distance = GetThreatDistance(ThreatActor);

    KnownThreats.Add(NewThreat);

    // Update combat state if this is a significant threat
    if (Level >= ECombat_ThreatLevel::Medium && CurrentCombatState == ECombat_CombatState::Idle)
    {
        SetCombatState(ECombat_CombatState::Investigating);
    }
}

void ACombat_CombatAIController::RemoveThreat(AActor* ThreatActor)
{
    KnownThreats.RemoveAll([ThreatActor](const FCombat_ThreatData& Threat)
    {
        return Threat.ThreatActor == ThreatActor;
    });

    if (CurrentTarget == ThreatActor)
    {
        CurrentTarget = GetHighestThreat();
        if (!CurrentTarget && CurrentCombatState != ECombat_CombatState::Idle)
        {
            SetCombatState(ECombat_CombatState::Idle);
        }
    }
}

AActor* ACombat_CombatAIController::GetHighestThreat()
{
    AActor* HighestThreat = nullptr;
    ECombat_ThreatLevel HighestLevel = ECombat_ThreatLevel::None;
    float ClosestDistance = FLT_MAX;

    for (const FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && IsValid(Threat.ThreatActor))
        {
            // Prioritize by threat level, then by distance
            if (Threat.ThreatLevel > HighestLevel || 
                (Threat.ThreatLevel == HighestLevel && Threat.Distance < ClosestDistance))
            {
                HighestThreat = Threat.ThreatActor;
                HighestLevel = Threat.ThreatLevel;
                ClosestDistance = Threat.Distance;
            }
        }
    }

    return HighestThreat;
}

float ACombat_CombatAIController::GetThreatDistance(AActor* ThreatActor)
{
    if (!ThreatActor || !GetPawn())
        return FLT_MAX;

    return FVector::Dist(GetPawn()->GetActorLocation(), ThreatActor->GetActorLocation());
}

bool ACombat_CombatAIController::CanAttack()
{
    if (!CurrentTarget || !GetPawn())
        return false;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
        return false;

    float DistanceToTarget = GetThreatDistance(CurrentTarget);
    return DistanceToTarget <= AttackRange;
}

void ACombat_CombatAIController::ExecuteAttack()
{
    if (!CanAttack())
        return;

    LastAttackTime = GetWorld()->GetTimeSeconds();
    SetCombatState(ECombat_CombatState::Attacking);

    // Update blackboard with attack target
    if (BlackboardComponent && CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("AttackTarget"), CurrentTarget);
    }

    UE_LOG(LogTemp, Log, TEXT("Combat AI executing attack on: %s"), 
           CurrentTarget ? *CurrentTarget->GetName() : TEXT("Unknown"));
}

void ACombat_CombatAIController::ExecuteRetreat()
{
    SetCombatState(ECombat_CombatState::Retreating);

    if (GetPawn() && CurrentTarget)
    {
        // Calculate retreat direction (away from threat)
        FVector RetreatDirection = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector RetreatLocation = GetPawn()->GetActorLocation() + (RetreatDirection * 1000.0f);

        // Update blackboard with retreat location
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("RetreatLocation"), RetreatLocation);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Combat AI executing retreat"));
}

FVector ACombat_CombatAIController::GetFlankingPosition(AActor* Target)
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Calculate perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }

    // Position at flanking distance
    FVector FlankPosition = TargetLocation + (FlankDirection * AttackRange * 0.8f);
    
    return FlankPosition;
}

void ACombat_CombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && IsValid(Actor))
        {
            ECombat_ThreatLevel ThreatLevel = CalculateThreatLevel(Actor);
            if (ThreatLevel > ECombat_ThreatLevel::None)
            {
                AddThreat(Actor, ThreatLevel);
            }
        }
    }
}

void ACombat_CombatAIController::UpdateThreatAssessment()
{
    // Update distances and reassess threats
    for (FCombat_ThreatData& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && IsValid(Threat.ThreatActor))
        {
            Threat.Distance = GetThreatDistance(Threat.ThreatActor);
            Threat.LastKnownLocation = Threat.ThreatActor->GetActorLocation();
        }
    }

    // Update current target
    AActor* NewTarget = GetHighestThreat();
    if (NewTarget != CurrentTarget)
    {
        CurrentTarget = NewTarget;
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
        }
    }
}

void ACombat_CombatAIController::UpdateCombatBehavior(float DeltaTime)
{
    if (!GetPawn())
        return;

    switch (CurrentCombatState)
    {
        case ECombat_CombatState::Idle:
            if (KnownThreats.Num() > 0)
            {
                SetCombatState(ECombat_CombatState::Investigating);
            }
            break;

        case ECombat_CombatState::Investigating:
            if (CurrentTarget)
            {
                float DistanceToTarget = GetThreatDistance(CurrentTarget);
                if (DistanceToTarget <= CombatRange)
                {
                    SetCombatState(ECombat_CombatState::Engaging);
                }
            }
            else
            {
                SetCombatState(ECombat_CombatState::Idle);
            }
            break;

        case ECombat_CombatState::Engaging:
            if (CurrentTarget)
            {
                float DistanceToTarget = GetThreatDistance(CurrentTarget);
                if (DistanceToTarget <= AttackRange && CanAttack())
                {
                    ExecuteAttack();
                }
                else if (DistanceToTarget > CombatRange)
                {
                    SetCombatState(ECombat_CombatState::Investigating);
                }
            }
            break;

        case ECombat_CombatState::Attacking:
            // Return to engaging after attack animation completes
            if (GetWorld()->GetTimeSeconds() - LastAttackTime > 2.0f)
            {
                SetCombatState(ECombat_CombatState::Engaging);
            }
            break;

        case ECombat_CombatState::Retreating:
            // Check if we should stop retreating
            if (!CurrentTarget || GetThreatDistance(CurrentTarget) > CombatRange * 1.5f)
            {
                SetCombatState(ECombat_CombatState::Idle);
            }
            break;
    }
}

ECombat_ThreatLevel ACombat_CombatAIController::CalculateThreatLevel(AActor* Actor)
{
    if (!Actor || !GetPawn())
        return ECombat_ThreatLevel::None;

    // Check if it's a player character
    if (Actor->IsA<ACharacter>())
    {
        float Distance = GetThreatDistance(Actor);
        
        if (Distance < AttackRange)
            return ECombat_ThreatLevel::Critical;
        else if (Distance < CombatRange * 0.5f)
            return ECombat_ThreatLevel::High;
        else if (Distance < CombatRange)
            return ECombat_ThreatLevel::Medium;
        else
            return ECombat_ThreatLevel::Low;
    }

    return ECombat_ThreatLevel::None;
}

void ACombat_CombatAIController::CleanupOldThreats()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    KnownThreats.RemoveAll([CurrentTime, this](const FCombat_ThreatData& Threat)
    {
        return !IsValid(Threat.ThreatActor) || 
               (CurrentTime - Threat.LastSeenTime > ThreatForgetTime);
    });
}