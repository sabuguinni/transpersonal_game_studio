// TRexBehavior.cpp
// Agent #11 — NPC Behavior Agent
// T-Rex AI: patrol 5000u radius, chase at 3000u, attack at 300u

#include "TRexBehavior.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

// ============================================================
// ATRexAIController
// ============================================================

ATRexAIController::ATRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 60.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ATRexAIController::OnTargetPerceptionUpdated);

    // Blackboard + BT
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // State
    CurrentState = ENPC_TRexState::Idle;
    PatrolRadius = 5000.0f;
    ChaseRange = 3000.0f;
    AttackRange = 300.0f;
    PatrolSpeed = 300.0f;
    ChaseSpeed = 600.0f;
    PatrolWaitTime = 3.0f;
    bIsWaiting = false;
    WaitTimer = 0.0f;
    TargetActor = nullptr;
}

void ATRexAIController::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;

    if (TRexBehaviorTree)
    {
        RunBehaviorTree(TRexBehaviorTree);
    }

    // Start patrol immediately
    SetState(ENPC_TRexState::Patrolling);
    PickNewPatrolPoint();
}

void ATRexAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn()) return;

    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_TRexState::Patrolling:
            TickPatrol(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            TickChase(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            TickAttack(DeltaTime);
            break;
        case ENPC_TRexState::Returning:
            TickReturn(DeltaTime);
            break;
    }
}

void ATRexAIController::TickIdle(float DeltaTime)
{
    WaitTimer += DeltaTime;
    if (WaitTimer >= PatrolWaitTime)
    {
        WaitTimer = 0.0f;
        SetState(ENPC_TRexState::Patrolling);
        PickNewPatrolPoint();
    }
}

void ATRexAIController::TickPatrol(float DeltaTime)
{
    if (!GetPawn()) return;

    // Check if player is within chase range
    if (TargetActor)
    {
        float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
        if (DistToTarget <= ChaseRange)
        {
            SetState(ENPC_TRexState::Chasing);
            return;
        }
    }

    // Check if we reached patrol point
    float DistToGoal = FVector::Dist(GetPawn()->GetActorLocation(), CurrentPatrolTarget);
    if (DistToGoal < 200.0f)
    {
        SetState(ENPC_TRexState::Idle);
    }
}

void ATRexAIController::TickChase(float DeltaTime)
{
    if (!GetPawn() || !TargetActor) 
    {
        SetState(ENPC_TRexState::Returning);
        return;
    }

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
    float DistFromHome = FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation);

    // Attack range reached
    if (DistToTarget <= AttackRange)
    {
        SetState(ENPC_TRexState::Attacking);
        return;
    }

    // Lost target or too far from home
    if (DistToTarget > ChaseRange * 1.5f || DistFromHome > PatrolRadius * 1.5f)
    {
        TargetActor = nullptr;
        SetState(ENPC_TRexState::Returning);
        return;
    }

    MoveToActor(TargetActor, AttackRange * 0.8f);
}

void ATRexAIController::TickAttack(float DeltaTime)
{
    if (!GetPawn() || !TargetActor)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());

    // Target escaped attack range
    if (DistToTarget > AttackRange * 1.5f)
    {
        SetState(ENPC_TRexState::Chasing);
        return;
    }

    // Face the target
    FVector Dir = (TargetActor->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FRotator LookAt = Dir.Rotation();
    GetPawn()->SetActorRotation(FMath::RInterpTo(GetPawn()->GetActorRotation(), LookAt, DeltaTime, 5.0f));
}

void ATRexAIController::TickReturn(float DeltaTime)
{
    if (!GetPawn()) return;

    float DistFromHome = FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation);
    if (DistFromHome < 300.0f)
    {
        SetState(ENPC_TRexState::Idle);
        return;
    }

    MoveToLocation(HomeLocation, 200.0f);
}

void ATRexAIController::SetState(ENPC_TRexState NewState)
{
    CurrentState = NewState;

    APawn* Pawn = GetPawn();
    if (!Pawn) return;

    UCharacterMovementComponent* Movement = Pawn->FindComponentByClass<UCharacterMovementComponent>();
    if (Movement)
    {
        switch (NewState)
        {
            case ENPC_TRexState::Patrolling:
            case ENPC_TRexState::Returning:
                Movement->MaxWalkSpeed = PatrolSpeed;
                break;
            case ENPC_TRexState::Chasing:
            case ENPC_TRexState::Attacking:
                Movement->MaxWalkSpeed = ChaseSpeed;
                break;
            default:
                Movement->MaxWalkSpeed = 0.0f;
                break;
        }
    }

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("TRexState"), (uint8)NewState);
    }
}

void ATRexAIController::PickNewPatrolPoint()
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc);
    if (bFound)
    {
        CurrentPatrolTarget = NavLoc.Location;
        MoveToLocation(CurrentPatrolTarget, 150.0f);
    }
}

void ATRexAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to player pawns
    if (!Actor->ActorHasTag(TEXT("Player"))) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        TargetActor = Actor;
        if (CurrentState == ENPC_TRexState::Idle || CurrentState == ENPC_TRexState::Patrolling)
        {
            SetState(ENPC_TRexState::Chasing);
        }
    }
    else
    {
        // Lost perception — keep chasing for a bit then give up
        if (CurrentState == ENPC_TRexState::Chasing)
        {
            // TargetActor remains set — TickChase will handle distance falloff
        }
    }
}
