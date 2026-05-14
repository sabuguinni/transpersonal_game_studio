#include "NPC_DinosaurAIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ANPC_DinosaurAIController::ANPC_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Default AI Settings
    SightRadius = 1500.0f;
    LoseSightRadius = 1600.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 1200.0f;
    PatrolRadius = 1000.0f;
    ChaseDistance = 3000.0f;
    AttackDistance = 200.0f;

    // Initialize state
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    PatrolDestination = FVector::ZeroVector;
}

void ANPC_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    // Set home location to spawn location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        PatrolDestination = HomeLocation;
    }
}

void ANPC_DinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    InitializeBehaviorTree();
    
    // Start behavior tree if available
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }
}

void ANPC_DinosaurAIController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ANPC_DinosaurAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnTargetPerceptionUpdated);
}

void ANPC_DinosaurAIController::InitializeBehaviorTree()
{
    if (BlackboardComponent && BehaviorTree)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
        UpdateBlackboardValues();
    }
}

void ANPC_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            ACharacter* Character = Cast<ACharacter>(Actor);
            if (Character && Character->IsPlayerControlled())
            {
                HandlePlayerDetection(Actor);
                break;
            }
        }
    }
}

void ANPC_DinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->IsA<ACharacter>())
        return;

    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character || !Character->IsPlayerControlled())
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        HandlePlayerDetection(Actor);
    }
    else
    {
        HandlePlayerLost();
    }
}

void ANPC_DinosaurAIController::HandlePlayerDetection(AActor* Player)
{
    if (!Player)
        return;

    SetTarget(Player);
    
    float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
    
    if (DistanceToPlayer <= AttackDistance)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Attacking);
    }
    else if (DistanceToPlayer <= ChaseDistance)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Chasing);
    }
    else
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Alert);
    }
}

void ANPC_DinosaurAIController::HandlePlayerLost()
{
    if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Chasing || 
        CurrentBehaviorState == ENPC_DinosaurBehaviorState::Attacking)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Searching);
        
        // Clear target after a delay
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { 
                if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Searching)
                {
                    ClearTarget();
                    SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
                }
            },
            5.0f,
            false
        );
    }
}

void ANPC_DinosaurAIController::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
        return;

    CurrentBehaviorState = NewState;
    UpdateBlackboardValues();

    // Handle state-specific logic
    switch (NewState)
    {
        case ENPC_DinosaurBehaviorState::Patrolling:
            SetPatrolDestination(GetRandomPatrolPoint());
            break;
            
        case ENPC_DinosaurBehaviorState::Idle:
            StopMovement();
            break;
            
        case ENPC_DinosaurBehaviorState::Returning:
            MoveToLocation(HomeLocation);
            break;
            
        default:
            break;
    }
}

void ANPC_DinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::ClearTarget()
{
    CurrentTarget = nullptr;
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::SetPatrolDestination(FVector NewDestination)
{
    PatrolDestination = NewDestination;
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::ReturnToHome()
{
    SetBehaviorState(ENPC_DinosaurBehaviorState::Returning);
}

float ANPC_DinosaurAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
        return -1.0f;
        
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

float ANPC_DinosaurAIController::GetDistanceToHome() const
{
    if (!GetPawn())
        return -1.0f;
        
    return FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation);
}

bool ANPC_DinosaurAIController::IsPlayerInSight() const
{
    return CurrentTarget != nullptr && CanSeeActor(CurrentTarget);
}

bool ANPC_DinosaurAIController::IsTargetInAttackRange() const
{
    if (!CurrentTarget)
        return false;
        
    float Distance = GetDistanceToTarget();
    return Distance > 0 && Distance <= AttackDistance;
}

FVector ANPC_DinosaurAIController::GetRandomPatrolPoint() const
{
    if (!GetPawn())
        return HomeLocation;

    // Generate random point within patrol radius
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
    FVector RandomPoint = HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find a valid navigation point
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(RandomPoint, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return RandomPoint;
}

bool ANPC_DinosaurAIController::CanSeeActor(AActor* Actor) const
{
    if (!AIPerceptionComponent || !Actor)
        return false;

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(Actor, Info);
    
    for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }
    
    return false;
}

void ANPC_DinosaurAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComponent->SetValueAsVector(TEXT("PatrolDestination"), PatrolDestination);
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    }
    
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToHome"), GetDistanceToHome());
    BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), IsPlayerInSight());
    BlackboardComponent->SetValueAsBool(TEXT("InAttackRange"), IsTargetInAttackRange());
}