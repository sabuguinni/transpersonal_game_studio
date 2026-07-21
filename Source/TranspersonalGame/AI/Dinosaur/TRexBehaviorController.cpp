#include "TRexBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Blackboard and Behavior Tree Components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 4000.0f;
        SightConfig->LoseSightRadius = 4500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        
        // Detect players
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Initialize state
    CurrentBehaviorState = ENPCBehaviorState::Idle;
    TargetPlayer = nullptr;
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    // Store home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ATRexBehaviorController::OnPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        InitializeBlackboard();
        RunBehaviorTree(BehaviorTree);
        StartPatrolling();
    }
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateBehaviorState();
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
    }
}

void ATRexBehaviorController::StartPatrolling()
{
    CurrentBehaviorState = ENPCBehaviorState::Patrolling;
    SetBlackboardBehaviorState(ENPCBehaviorState::Patrolling);
    
    // Set patrol location
    FVector PatrolLocation = GetRandomPatrolLocation();
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(PatrolLocationKey, PatrolLocation);
    }

    // Move to patrol location
    MoveToLocation(PatrolLocation);
}

void ATRexBehaviorController::StartChasing(APawn* Target)
{
    if (!Target) return;

    CurrentBehaviorState = ENPCBehaviorState::Chasing;
    TargetPlayer = Target;
    SetBlackboardBehaviorState(ENPCBehaviorState::Chasing);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(PlayerActorKey, Target);
    }

    // Move to target
    MoveToActor(Target);
}

void ATRexBehaviorController::StartAttacking(APawn* Target)
{
    if (!Target) return;

    CurrentBehaviorState = ENPCBehaviorState::Attacking;
    TargetPlayer = Target;
    SetBlackboardBehaviorState(ENPCBehaviorState::Attacking);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(PlayerActorKey, Target);
    }

    // Stop movement for attack
    StopMovement();
}

void ATRexBehaviorController::ReturnToPatrol()
{
    TargetPlayer = nullptr;
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(PlayerActorKey);
    }
    StartPatrolling();
}

FVector ATRexBehaviorController::GetRandomPatrolLocation()
{
    FVector RandomLocation = HomeLocation;
    
    // Get random point within patrol radius
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLocation))
        {
            RandomLocation = NavLocation.Location;
        }
    }
    else
    {
        // Fallback: random point in circle
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
        
        RandomLocation.X += FMath::Cos(RandomAngle) * RandomDistance;
        RandomLocation.Y += FMath::Sin(RandomAngle) * RandomDistance;
    }
    
    return RandomLocation;
}

float ATRexBehaviorController::GetDistanceToPlayer() const
{
    if (!TargetPlayer || !GetPawn()) return -1.0f;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
}

bool ATRexBehaviorController::IsPlayerInRange(float Range) const
{
    float Distance = GetDistanceToPlayer();
    return Distance >= 0.0f && Distance <= Range;
}

void ATRexBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Check if this is a player character
            if (Character->IsPlayerControlled())
            {
                FAIStimulus Stimulus;
                if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
                {
                    if (Stimulus.WasSuccessfullySensed())
                    {
                        // Player detected
                        TargetPlayer = Character;
                        
                        float DistanceToPlayer = GetDistanceToPlayer();
                        
                        if (DistanceToPlayer <= AttackRange)
                        {
                            StartAttacking(Character);
                        }
                        else if (DistanceToPlayer <= ChaseRange)
                        {
                            StartChasing(Character);
                        }
                    }
                    else
                    {
                        // Player lost
                        if (TargetPlayer == Character)
                        {
                            ReturnToPatrol();
                        }
                    }
                }
            }
        }
    }
}

void ATRexBehaviorController::UpdateBehaviorState()
{
    if (!GetPawn()) return;

    // Check distance to player if we have one
    if (TargetPlayer)
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        
        switch (CurrentBehaviorState)
        {
            case ENPCBehaviorState::Chasing:
                if (DistanceToPlayer <= AttackRange)
                {
                    StartAttacking(TargetPlayer);
                }
                else if (DistanceToPlayer > ChaseRange)
                {
                    ReturnToPatrol();
                }
                break;
                
            case ENPCBehaviorState::Attacking:
                if (DistanceToPlayer > AttackRange)
                {
                    if (DistanceToPlayer <= ChaseRange)
                    {
                        StartChasing(TargetPlayer);
                    }
                    else
                    {
                        ReturnToPatrol();
                    }
                }
                break;
                
            default:
                break;
        }
    }

    // Check if we've reached patrol destination
    if (CurrentBehaviorState == ENPCBehaviorState::Patrolling)
    {
        if (GetPathFollowingComponent() && GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
        {
            // Start new patrol
            FVector PatrolLocation = GetRandomPatrolLocation();
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(PatrolLocationKey, PatrolLocation);
            }
            MoveToLocation(PatrolLocation);
        }
    }
}

void ATRexBehaviorController::SetBlackboardBehaviorState(ENPCBehaviorState NewState)
{
    CurrentBehaviorState = NewState;
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(BehaviorStateKey, static_cast<uint8>(NewState));
    }
}

void ATRexBehaviorController::InitializeBlackboard()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(HomeLocationKey, HomeLocation);
        BlackboardComponent->SetValueAsEnum(BehaviorStateKey, static_cast<uint8>(ENPCBehaviorState::Idle));
    }
}