#include "Combat_CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACombat_CombatAIController::ACombat_CombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Setup Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    
    // Setup Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize Behavior Tree Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    
    // Initialize Combat State
    CurrentCombatState = ECombat_CombatState::Idle;
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    LastAttackTime = 0.0f;
    bIsInCombat = false;
    PatrolRadius = 1000.0f;
    StateChangeTime = 0.0f;
    AttackComboCount = 0;
    
    // Initialize Combat Stats with default values
    CombatStats = FCombat_CombatStats();
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
    
    // Set home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }
    
    // Initialize combat stats from pawn tags
    InitializeCombatStats();
    
    // Start combat update timer
    GetWorld()->GetTimerManager().SetTimer(CombatUpdateTimer, this, &ACombat_CombatAIController::UpdateCombatBehavior, 0.1f, true);
    
    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombat_CombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update blackboard values
    UpdateBlackboardValues();
}

void ACombat_CombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        InitializeCombatStats();
    }
}

void ACombat_CombatAIController::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI State Changed to: %d"), static_cast<int32>(NewState));
    }
}

void ACombat_CombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        bIsInCombat = true;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), true);
        }
    }
    else
    {
        bIsInCombat = false;
        
        // Clear blackboard target
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
            BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), false);
        }
    }
}

bool ACombat_CombatAIController::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= CombatStats.AttackCooldown && IsTargetInAttackRange();
}

void ACombat_CombatAIController::PerformAttack()
{
    if (!CanAttack() || !CurrentTarget)
    {
        return;
    }
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
    AttackComboCount++;
    
    // Apply damage to target if it has health component
    if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
    {
        // Apply damage through gameplay statics
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            CombatStats.AttackDamage,
            this,
            GetPawn(),
            UDamageType::StaticClass()
        );
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI performed attack for %f damage"), CombatStats.AttackDamage);
    }
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("LastAttackTime"), LastAttackTime);
        BlackboardComponent->SetValueAsInt(TEXT("AttackCombo"), AttackComboCount);
    }
}

float ACombat_CombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ACombat_CombatAIController::IsTargetInAttackRange() const
{
    return GetDistanceToTarget() <= CombatStats.AttackRange;
}

bool ACombat_CombatAIController::IsTargetInDetectionRange() const
{
    return GetDistanceToTarget() <= CombatStats.DetectionRange;
}

void ACombat_CombatAIController::UpdateCombatBehavior()
{
    if (!GetPawn())
    {
        return;
    }
    
    // Handle current state
    switch (CurrentCombatState)
    {
        case ECombat_CombatState::Idle:
            HandleIdleState();
            break;
        case ECombat_CombatState::Patrolling:
            HandlePatrollingState();
            break;
        case ECombat_CombatState::Investigating:
            HandleInvestigatingState();
            break;
        case ECombat_CombatState::Stalking:
            HandleStalkingState();
            break;
        case ECombat_CombatState::Attacking:
            HandleAttackingState();
            break;
        case ECombat_CombatState::Fleeing:
            HandleFleeingState();
            break;
        case ECombat_CombatState::Defending:
            HandleDefendingState();
            break;
    }
}

void ACombat_CombatAIController::InitializeCombatStats()
{
    if (!GetPawn())
    {
        return;
    }
    
    // Parse combat stats from pawn tags
    TArray<FName> Tags = GetPawn()->Tags;
    
    for (const FName& Tag : Tags)
    {
        FString TagString = Tag.ToString();
        
        if (TagString.StartsWith(TEXT("aggression_")))
        {
            FString ValueString = TagString.RightChop(11); // Remove "aggression_"
            CombatStats.Aggression = FCString::Atof(*ValueString);
        }
        else if (TagString.StartsWith(TEXT("attack_range_")))
        {
            FString ValueString = TagString.RightChop(13); // Remove "attack_range_"
            CombatStats.AttackRange = FCString::Atof(*ValueString);
        }
        else if (TagString.StartsWith(TEXT("detection_range_")))
        {
            FString ValueString = TagString.RightChop(16); // Remove "detection_range_"
            CombatStats.DetectionRange = FCString::Atof(*ValueString);
        }
        else if (TagString.StartsWith(TEXT("combat_style_")))
        {
            FString StyleString = TagString.RightChop(13); // Remove "combat_style_"
            
            if (StyleString == TEXT("ambush_predator"))
            {
                CombatStats.CombatStyle = ECombat_CombatStyle::AmbushPredator;
            }
            else if (StyleString == TEXT("pack_hunter"))
            {
                CombatStats.CombatStyle = ECombat_CombatStyle::PackHunter;
            }
            else if (StyleString == TEXT("defensive"))
            {
                CombatStats.CombatStyle = ECombat_CombatStyle::Defensive;
            }
            else if (StyleString == TEXT("flee"))
            {
                CombatStats.CombatStyle = ECombat_CombatStyle::Flee;
            }
            else if (StyleString == TEXT("tank"))
            {
                CombatStats.CombatStyle = ECombat_CombatStyle::Tank;
            }
        }
    }
    
    // Adjust stats based on combat style
    switch (CombatStats.CombatStyle)
    {
        case ECombat_CombatStyle::AmbushPredator:
            CombatStats.AttackDamage *= 1.5f;
            CombatStats.MovementSpeed *= 1.2f;
            break;
        case ECombat_CombatStyle::PackHunter:
            CombatStats.DetectionRange *= 1.3f;
            CombatStats.MovementSpeed *= 1.4f;
            break;
        case ECombat_CombatStyle::Tank:
            CombatStats.AttackDamage *= 0.8f;
            CombatStats.MovementSpeed *= 0.7f;
            break;
        case ECombat_CombatStyle::Flee:
            CombatStats.MovementSpeed *= 1.6f;
            CombatStats.Aggression *= 0.3f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat AI initialized: Aggression=%f, AttackRange=%f, Style=%d"), 
           CombatStats.Aggression, CombatStats.AttackRange, static_cast<int32>(CombatStats.CombatStyle));
}

void ACombat_CombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ShouldEngageTarget(Actor))
        {
            SetTarget(Actor);
            SetCombatState(ECombat_CombatState::Stalking);
            break;
        }
    }
}

void ACombat_CombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor && Stimulus.WasSuccessfullySensed())
    {
        if (ShouldEngageTarget(Actor))
        {
            SetTarget(Actor);
            LastKnownTargetLocation = Actor->GetActorLocation();
            
            if (CurrentCombatState == ECombat_CombatState::Idle || CurrentCombatState == ECombat_CombatState::Patrolling)
            {
                SetCombatState(ECombat_CombatState::Investigating);
            }
        }
    }
}

void ACombat_CombatAIController::HandleIdleState()
{
    float StateTime = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    if (StateTime > 3.0f) // Idle for 3 seconds
    {
        SetCombatState(ECombat_CombatState::Patrolling);
    }
}

void ACombat_CombatAIController::HandlePatrollingState()
{
    // Move to random patrol point
    FVector PatrolPoint = GetRandomPatrolPoint();
    MoveToLocation(PatrolPoint, 100.0f);
    
    float StateTime = GetWorld()->GetTimeSeconds() - StateChangeTime;
    if (StateTime > 10.0f) // Patrol for 10 seconds
    {
        SetCombatState(ECombat_CombatState::Idle);
    }
}

void ACombat_CombatAIController::HandleInvestigatingState()
{
    if (CurrentTarget)
    {
        MoveToActor(CurrentTarget, CombatStats.AttackRange * 0.8f);
        
        if (IsTargetInAttackRange())
        {
            SetCombatState(ECombat_CombatState::Attacking);
        }
        else if (!IsTargetInDetectionRange())
        {
            SetTarget(nullptr);
            SetCombatState(ECombat_CombatState::Patrolling);
        }
    }
    else
    {
        SetCombatState(ECombat_CombatState::Patrolling);
    }
}

void ACombat_CombatAIController::HandleStalkingState()
{
    if (CurrentTarget)
    {
        float Distance = GetDistanceToTarget();
        
        if (Distance <= CombatStats.AttackRange)
        {
            SetCombatState(ECombat_CombatState::Attacking);
        }
        else if (Distance > CombatStats.DetectionRange)
        {
            SetTarget(nullptr);
            SetCombatState(ECombat_CombatState::Patrolling);
        }
        else
        {
            // Move closer but maintain stealth distance
            MoveToActor(CurrentTarget, CombatStats.AttackRange * 1.2f);
        }
    }
    else
    {
        SetCombatState(ECombat_CombatState::Patrolling);
    }
}

void ACombat_CombatAIController::HandleAttackingState()
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_CombatState::Patrolling);
        return;
    }
    
    if (ShouldFleeFromTarget(CurrentTarget))
    {
        SetCombatState(ECombat_CombatState::Fleeing);
        return;
    }
    
    if (IsTargetInAttackRange())
    {
        if (CanAttack())
        {
            PerformAttack();
        }
        else
        {
            // Circle around target
            FVector TargetLocation = CurrentTarget->GetActorLocation();
            FVector MyLocation = GetPawn()->GetActorLocation();
            FVector Direction = (MyLocation - TargetLocation).GetSafeNormal();
            FVector CirclePoint = TargetLocation + Direction.RotateAngleAxis(90.0f, FVector::UpVector) * CombatStats.AttackRange;
            MoveToLocation(CirclePoint, 50.0f);
        }
    }
    else
    {
        MoveToActor(CurrentTarget, CombatStats.AttackRange * 0.8f);
    }
}

void ACombat_CombatAIController::HandleFleeingState()
{
    if (CurrentTarget)
    {
        FVector FleeDirection = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleePoint = GetPawn()->GetActorLocation() + FleeDirection * 1500.0f;
        MoveToLocation(FleePoint, 100.0f);
        
        if (GetDistanceToTarget() > CombatStats.DetectionRange * 2.0f)
        {
            SetTarget(nullptr);
            SetCombatState(ECombat_CombatState::Patrolling);
        }
    }
    else
    {
        SetCombatState(ECombat_CombatState::Patrolling);
    }
}

void ACombat_CombatAIController::HandleDefendingState()
{
    // Stay in defensive position
    MoveToLocation(HomeLocation, 200.0f);
    
    if (CurrentTarget && GetDistanceToTarget() > CombatStats.DetectionRange)
    {
        SetTarget(nullptr);
        SetCombatState(ECombat_CombatState::Idle);
    }
}

bool ACombat_CombatAIController::ShouldEngageTarget(AActor* PotentialTarget) const
{
    if (!PotentialTarget || PotentialTarget == GetPawn())
    {
        return false;
    }
    
    // Check if target is player character
    if (PotentialTarget->IsA<ACharacter>())
    {
        return CombatStats.Aggression > 0.3f;
    }
    
    return false;
}

bool ACombat_CombatAIController::ShouldFleeFromTarget(AActor* PotentialTarget) const
{
    if (!PotentialTarget)
    {
        return false;
    }
    
    // Flee if aggression is very low or if heavily damaged
    return CombatStats.Aggression < 0.2f || CombatStats.CombatStyle == ECombat_CombatStyle::Flee;
}

FVector ACombat_CombatAIController::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    return HomeLocation + RandomDirection * RandomDistance;
}

void ACombat_CombatAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }
    
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
    BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), CombatStats.Aggression);
    BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), CombatStats.AttackRange);
    BlackboardComponent->SetValueAsFloat(TEXT("DetectionRange"), CombatStats.DetectionRange);
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    }
}