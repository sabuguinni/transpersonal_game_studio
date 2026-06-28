// TRexBehavior.cpp
// Agent #11 — NPC Behavior Agent
// Cycle: PROD_CYCLE_AUTO_20260628_009
//
// T-Rex AI Controller — patrol, chase, and attack behavior.
// Patrols a 5000-unit radius. Chases player within 3000 units.
// Attacks when within 300 units.

#include "TRexBehavior.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
//  ATRexAIController
// ─────────────────────────────────────────────────────────────────────────────

ATRexAIController::ATRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                = 3000.0f;
    SightConfig->LoseSightRadius            = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(8.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ATRexAIController::OnPerceptionUpdated);

    // Behavior tree component
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp   = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

    // State defaults
    TRexState         = ENPC_TRexState::Patrolling;
    PatrolRadius      = 5000.0f;
    ChaseRadius       = 3000.0f;
    AttackRadius      = 300.0f;
    PatrolSpeed       = 300.0f;
    ChaseSpeed        = 700.0f;
    AttackCooldown    = 2.5f;
    bCanAttack        = true;
    bShowDebugTraces  = false;
    TargetActor       = nullptr;
}

void ATRexAIController::BeginPlay()
{
    Super::BeginPlay();

    // Store home location for patrol radius
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        HomeLocation = ControlledPawn->GetActorLocation();
    }

    // Start patrol immediately
    SetTRexState(ENPC_TRexState::Patrolling);
    PickNewPatrolPoint();

    // Periodic state evaluation
    GetWorld()->GetTimerManager().SetTimer(
        StateEvalTimerHandle,
        this,
        &ATRexAIController::EvaluateState,
        0.5f,
        true
    );
}

void ATRexAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShowDebugTraces)
    {
        APawn* Pawn = GetPawn();
        if (Pawn)
        {
            DrawDebugSphere(GetWorld(), HomeLocation, PatrolRadius, 24, FColor::Green, false, -1.0f, 0, 3.0f);
            DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), ChaseRadius, 20, FColor::Orange, false, -1.0f, 0, 2.0f);
            DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), AttackRadius, 12, FColor::Red, false, -1.0f, 0, 2.0f);

            if (TargetActor)
            {
                DrawDebugLine(GetWorld(), Pawn->GetActorLocation(), TargetActor->GetActorLocation(), FColor::Red, false, -1.0f, 0, 4.0f);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  State Machine
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::SetTRexState(ENPC_TRexState NewState)
{
    if (NewState == TRexState) return;

    TRexState = NewState;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* TRexChar = Cast<ACharacter>(ControlledPawn);
    if (!TRexChar) return;

    UCharacterMovementComponent* MoveComp = TRexChar->GetCharacterMovement();
    if (!MoveComp) return;

    switch (TRexState)
    {
        case ENPC_TRexState::Patrolling:
            MoveComp->MaxWalkSpeed = PatrolSpeed;
            StopMovement();
            PickNewPatrolPoint();
            break;

        case ENPC_TRexState::Investigating:
            MoveComp->MaxWalkSpeed = PatrolSpeed * 1.4f;
            break;

        case ENPC_TRexState::Chasing:
            MoveComp->MaxWalkSpeed = ChaseSpeed;
            break;

        case ENPC_TRexState::Attacking:
            MoveComp->MaxWalkSpeed = ChaseSpeed * 1.1f;
            break;

        case ENPC_TRexState::Roaring:
            StopMovement();
            // Roar lasts 2 seconds then transitions to Chasing
            GetWorld()->GetTimerManager().SetTimer(
                RoarTimerHandle,
                [this]() { SetTRexState(ENPC_TRexState::Chasing); },
                2.0f,
                false
            );
            break;

        case ENPC_TRexState::Resting:
            MoveComp->MaxWalkSpeed = 0.0f;
            StopMovement();
            break;
    }

    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(FName("TRexState"), static_cast<uint8>(TRexState));
    }

    UE_LOG(LogTemp, Log, TEXT("TRexAIController: State -> %d"), static_cast<int32>(TRexState));
}

void ATRexAIController::EvaluateState()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector TRexLocation = ControlledPawn->GetActorLocation();

    // Find player
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return;

    float DistToPlayer = FVector::Dist(TRexLocation, PlayerChar->GetActorLocation());

    switch (TRexState)
    {
        case ENPC_TRexState::Patrolling:
        case ENPC_TRexState::Resting:
        case ENPC_TRexState::Investigating:
            if (DistToPlayer <= AttackRadius)
            {
                TargetActor = PlayerChar;
                SetTRexState(ENPC_TRexState::Attacking);
            }
            else if (DistToPlayer <= ChaseRadius)
            {
                TargetActor = PlayerChar;
                SetTRexState(ENPC_TRexState::Roaring);
            }
            break;

        case ENPC_TRexState::Chasing:
            if (DistToPlayer <= AttackRadius)
            {
                SetTRexState(ENPC_TRexState::Attacking);
            }
            else if (DistToPlayer > ChaseRadius * 1.5f)
            {
                // Lost the player
                TargetActor = nullptr;
                SetTRexState(ENPC_TRexState::Patrolling);
            }
            else
            {
                // Continue chasing
                MoveToActor(PlayerChar, AttackRadius * 0.8f);
            }
            break;

        case ENPC_TRexState::Attacking:
            if (DistToPlayer > AttackRadius * 1.5f)
            {
                SetTRexState(ENPC_TRexState::Chasing);
            }
            else if (bCanAttack)
            {
                PerformAttack();
            }
            break;

        default:
            break;
    }

    // Patrol movement — pick new point if we've arrived
    if (TRexState == ENPC_TRexState::Patrolling)
    {
        EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
        if (MoveStatus == EPathFollowingStatus::Idle)
        {
            PickNewPatrolPoint();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Patrol
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::PickNewPatrolPoint()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    FNavLocation RandomPoint;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomPoint);

    if (bFound)
    {
        CurrentPatrolTarget = RandomPoint.Location;
        MoveToLocation(CurrentPatrolTarget, 100.0f);

        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsVector(FName("PatrolTarget"), CurrentPatrolTarget);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Attack
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::PerformAttack()
{
    if (!bCanAttack || !TargetActor) return;

    bCanAttack = false;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        TargetActor,
        50.0f,                  // T-Rex bite damage
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("TRexAIController: ATTACK on %s — 50 damage"), *TargetActor->GetName());

    // Attack cooldown
    GetWorld()->GetTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Perception Callback
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to the player
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Actor != PlayerChar) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        TargetActor = Actor;

        if (TRexState == ENPC_TRexState::Patrolling || TRexState == ENPC_TRexState::Resting)
        {
            SetTRexState(ENPC_TRexState::Roaring);
        }

        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsObject(FName("TargetActor"), Actor);
            BlackboardComp->SetValueAsVector(FName("LastKnownThreatLocation"), Actor->GetActorLocation());
        }
    }
    else
    {
        // Lost sight — investigate last known location
        if (TRexState == ENPC_TRexState::Chasing)
        {
            SetTRexState(ENPC_TRexState::Investigating);
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(FName("LastKnownThreatLocation"), Stimulus.StimulusLocation);
            }
            MoveToLocation(Stimulus.StimulusLocation, 200.0f);
        }
    }
}
