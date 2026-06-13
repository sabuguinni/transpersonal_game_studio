#include "Combat_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = SightAngle;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Blackboard and Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Initialize tactical data
    TacticalData = FCombat_TacticalData();
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
        GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
    }
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatBehavior(DeltaTime);
}

void ACombat_AIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
        GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
    }
}

void ACombat_AIController::SetCombatState(ECombat_AIState NewState)
{
    if (TacticalData.CurrentState != NewState)
    {
        TacticalData.CurrentState = NewState;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
        
        ProcessTacticalDecision();
    }
}

void ACombat_AIController::SetPrimaryTarget(AActor* Target)
{
    TacticalData.PrimaryTarget = Target;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Target);
        
        if (Target)
        {
            TacticalData.LastKnownTargetLocation = Target->GetActorLocation();
            GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), TacticalData.LastKnownTargetLocation);
        }
    }
    
    UpdateThreatLevel();
}

void ACombat_AIController::UpdateThreatLevel()
{
    if (!TacticalData.PrimaryTarget)
    {
        TacticalData.ThreatLevel = ECombat_ThreatLevel::None;
        return;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TacticalData.PrimaryTarget->GetActorLocation());
    
    if (DistanceToTarget <= AttackRange)
    {
        TacticalData.ThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else if (DistanceToTarget <= TacticalData.EngagementRange * 0.5f)
    {
        TacticalData.ThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (DistanceToTarget <= TacticalData.EngagementRange)
    {
        TacticalData.ThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else
    {
        TacticalData.ThreatLevel = ECombat_ThreatLevel::Low;
    }

    // Update blackboard
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(TacticalData.ThreatLevel));
    }
}

bool ACombat_AIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo Info;
    return AIPerceptionComponent->GetActorsPerception(Target, Info) && Info.bIsHostile;
}

bool ACombat_AIController::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

FVector ACombat_AIController::GetFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the right or left
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f; // Random left or right
    }
    
    return TargetLocation + (FlankDirection * TacticalData.FlankingDistance);
}

void ACombat_AIController::CoordinatePackAttack()
{
    if (!TacticalData.bIsPackLeader || TacticalData.PackMembers.Num() == 0)
    {
        return;
    }

    // Assign roles to pack members
    for (int32 i = 0; i < TacticalData.PackMembers.Num(); i++)
    {
        if (AActor* Member = TacticalData.PackMembers[i])
        {
            if (ACombat_AIController* MemberController = Cast<ACombat_AIController>(Member))
            {
                if (i % 2 == 0)
                {
                    MemberController->SetCombatState(ECombat_AIState::Flank);
                }
                else
                {
                    MemberController->SetCombatState(ECombat_AIState::Attack);
                }
            }
        }
    }
}

void ACombat_AIController::RegisterPackMember(AActor* Member)
{
    if (Member && !TacticalData.PackMembers.Contains(Member))
    {
        TacticalData.PackMembers.Add(Member);
    }
}

void ACombat_AIController::RemovePackMember(AActor* Member)
{
    TacticalData.PackMembers.Remove(Member);
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a potential target
            if (!TacticalData.PrimaryTarget || 
                FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation()) < 
                FVector::Dist(GetPawn()->GetActorLocation(), TacticalData.PrimaryTarget->GetActorLocation()))
            {
                SetPrimaryTarget(Actor);
                SetCombatState(ECombat_AIState::Hunt);
            }
        }
    }
}

void ACombat_AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == TacticalData.PrimaryTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            TacticalData.LastKnownTargetLocation = Actor->GetActorLocation();
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), TacticalData.LastKnownTargetLocation);
            }
        }
    }
}

void ACombat_AIController::UpdateCombatBehavior(float DeltaTime)
{
    LastTacticalUpdate += DeltaTime;
    LastCoordinationUpdate += DeltaTime;

    if (LastTacticalUpdate >= TacticalUpdateInterval)
    {
        UpdateThreatLevel();
        ProcessTacticalDecision();
        LastTacticalUpdate = 0.0f;
    }

    if (LastCoordinationUpdate >= CoordinationUpdateInterval && TacticalData.bIsPackLeader)
    {
        UpdatePackCoordination();
        LastCoordinationUpdate = 0.0f;
    }
}

void ACombat_AIController::ProcessTacticalDecision()
{
    if (!TacticalData.PrimaryTarget)
    {
        SetCombatState(ECombat_AIState::Patrol);
        return;
    }

    switch (TacticalData.ThreatLevel)
    {
        case ECombat_ThreatLevel::Critical:
            if (IsInAttackRange(TacticalData.PrimaryTarget))
            {
                SetCombatState(ECombat_AIState::Attack);
            }
            else
            {
                SetCombatState(ECombat_AIState::Hunt);
            }
            break;
            
        case ECombat_ThreatLevel::High:
            SetCombatState(ECombat_AIState::Hunt);
            break;
            
        case ECombat_ThreatLevel::Medium:
            if (TacticalData.bIsPackLeader)
            {
                SetCombatState(ECombat_AIState::Coordinate);
                CoordinatePackAttack();
            }
            else
            {
                SetCombatState(ECombat_AIState::Flank);
            }
            break;
            
        case ECombat_ThreatLevel::Low:
            SetCombatState(ECombat_AIState::Patrol);
            break;
            
        default:
            SetCombatState(ECombat_AIState::Idle);
            break;
    }
}

void ACombat_AIController::ExecuteFlankingManeuver()
{
    if (TacticalData.PrimaryTarget)
    {
        FVector FlankPosition = GetFlankingPosition(TacticalData.PrimaryTarget);
        MoveToLocation(FlankPosition);
    }
}

void ACombat_AIController::HandleRetreat()
{
    if (GetPawn())
    {
        FVector RetreatDirection = (GetPawn()->GetActorLocation() - TacticalData.LastKnownTargetLocation).GetSafeNormal();
        FVector RetreatPosition = GetPawn()->GetActorLocation() + (RetreatDirection * 1000.0f);
        MoveToLocation(RetreatPosition);
        SetCombatState(ECombat_AIState::Retreat);
    }
}

void ACombat_AIController::UpdatePackCoordination()
{
    if (TacticalData.bIsPackLeader && TacticalData.PrimaryTarget)
    {
        // Remove invalid pack members
        TacticalData.PackMembers.RemoveAll([](AActor* Member) {
            return !IsValid(Member);
        });
        
        // Coordinate attack if we have a target
        CoordinatePackAttack();
    }
}