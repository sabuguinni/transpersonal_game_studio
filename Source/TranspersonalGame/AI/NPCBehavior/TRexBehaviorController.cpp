// TRexBehaviorController.cpp
// NPC Behavior Agent #11 — T-Rex AI Controller implementation
// PROD_CYCLE_AUTO_20260622_008

#include "TRexBehaviorController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = SightAngle;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initial state
    CurrentState = ENPC_TRexState::Idle;
    CurrentWaypointIndex = 0;
    StateTimer = 0.0f;
    AttackCooldownTimer = 0.0f;
    bAttackOnCooldown = false;
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ATRexBehaviorController::OnPerceptionUpdated);
    }

    // Start patrolling after short idle
    FTimerHandle StartTimer;
    GetWorld()->GetTimerManager().SetTimer(StartTimer, [this]()
    {
        SetTRexState(ENPC_TRexState::Patrolling);
    }, 2.0f, false);
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    StateTimer += DeltaTime;

    // Update attack cooldown
    if (bAttackOnCooldown)
    {
        AttackCooldownTimer -= DeltaTime;
        if (AttackCooldownTimer <= 0.0f)
        {
            bAttackOnCooldown = false;
        }
    }

    // Update perception data age
    if (PerceptionData.LastKnownTarget)
    {
        PerceptionData.TimeSinceLastSeen += DeltaTime;
    }

    // State machine update
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            UpdateIdleState(DeltaTime);
            break;
        case ENPC_TRexState::Patrolling:
            UpdatePatrolState(DeltaTime);
            break;
        case ENPC_TRexState::Investigating:
            UpdateInvestigateState(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            UpdateChaseState(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            UpdateAttackState(DeltaTime);
            break;
        default:
            break;
    }
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Auto-discover waypoints in level tagged "TRex_Waypoint"
    TArray<AActor*> FoundWaypoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TRexWaypoint"), FoundWaypoints);
    if (FoundWaypoints.Num() > 0)
    {
        SetPatrolWaypoints(FoundWaypoints);
    }
}

void ATRexBehaviorController::OnUnPossess()
{
    Super::OnUnPossess();
    PatrolWaypoints.Empty();
    PerceptionData = FNPC_TRexPerceptionData();
}

void ATRexBehaviorController::SetTRexState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    switch (NewState)
    {
        case ENPC_TRexState::Patrolling:
            SetMovementSpeed(PatrolSpeed);
            MoveToNextWaypoint();
            break;
        case ENPC_TRexState::Chasing:
            SetMovementSpeed(ChaseSpeed);
            break;
        case ENPC_TRexState::Idle:
            StopMovement();
            break;
        case ENPC_TRexState::Roaring:
            StopMovement();
            // Return to patrol after roar animation
            {
                FTimerHandle RoarTimer;
                GetWorld()->GetTimerManager().SetTimer(RoarTimer, [this]()
                {
                    SetTRexState(ENPC_TRexState::Patrolling);
                }, 3.0f, false);
            }
            break;
        default:
            break;
    }
}

void ATRexBehaviorController::SetPatrolWaypoints(const TArray<AActor*>& Waypoints)
{
    PatrolWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
}

void ATRexBehaviorController::MoveToNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0) return;

    AActor* Waypoint = PatrolWaypoints[CurrentWaypointIndex % PatrolWaypoints.Num()];
    if (Waypoint)
    {
        MoveToActor(Waypoint, WaypointAcceptanceRadius);
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    }
}

void ATRexBehaviorController::StartChasing(AActor* Target)
{
    if (!Target) return;

    PerceptionData.LastKnownTarget = Target;
    PerceptionData.LastKnownLocation = Target->GetActorLocation();
    PerceptionData.TimeSinceLastSeen = 0.0f;
    PerceptionData.bTargetInSight = true;

    SetTRexState(ENPC_TRexState::Chasing);
}

void ATRexBehaviorController::ExecuteAttack()
{
    if (bAttackOnCooldown) return;
    if (!PerceptionData.LastKnownTarget) return;

    float Dist = GetDistanceToTarget();
    if (Dist <= AttackRange)
    {
        SetTRexState(ENPC_TRexState::Attacking);

        // Apply damage via UE5 damage system
        UGameplayStatics::ApplyDamage(
            PerceptionData.LastKnownTarget,
            AttackDamage,
            this,
            GetPawn(),
            nullptr
        );

        bAttackOnCooldown = true;
        AttackCooldownTimer = AttackCooldown;

        // Return to chase after attack
        FTimerHandle AttackTimer;
        GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this]()
        {
            if (PerceptionData.LastKnownTarget && PerceptionData.bTargetInSight)
            {
                SetTRexState(ENPC_TRexState::Chasing);
            }
            else
            {
                SetTRexState(ENPC_TRexState::Patrolling);
            }
        }, 1.5f, false);
    }
}

void ATRexBehaviorController::RoarAlert()
{
    SetTRexState(ENPC_TRexState::Roaring);
}

void ATRexBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if it's the player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (Actor == PlayerPawn)
        {
            FActorPerceptionBlueprintInfo Info;
            PerceptionComponent->GetActorsPerception(Actor, Info);

            bool bCurrentlySeen = false;
            for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
            {
                if (Stimulus.IsActive())
                {
                    bCurrentlySeen = true;
                    break;
                }
            }

            if (bCurrentlySeen)
            {
                PerceptionData.bTargetInSight = true;
                PerceptionData.TimeSinceLastSeen = 0.0f;
                PerceptionData.LastKnownLocation = Actor->GetActorLocation();

                if (CurrentState == ENPC_TRexState::Patrolling || CurrentState == ENPC_TRexState::Idle)
                {
                    RoarAlert();
                    FTimerHandle ChaseDelay;
                    GetWorld()->GetTimerManager().SetTimer(ChaseDelay, [this, Actor]()
                    {
                        StartChasing(Actor);
                    }, 1.5f, false);
                }
                else if (CurrentState == ENPC_TRexState::Chasing)
                {
                    PerceptionData.LastKnownTarget = Actor;
                }
            }
            else
            {
                PerceptionData.bTargetInSight = false;
                // Lost sight — investigate last known location
                if (CurrentState == ENPC_TRexState::Chasing)
                {
                    SetTRexState(ENPC_TRexState::Investigating);
                }
            }
        }
    }
}

// --- Private state update methods ---

void ATRexBehaviorController::UpdateIdleState(float DeltaTime)
{
    // After 5 seconds idle, start patrolling
    if (StateTimer >= 5.0f)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
    }
}

void ATRexBehaviorController::UpdatePatrolState(float DeltaTime)
{
    // Check if we reached the waypoint
    EPathFollowingStatus::Type Status = GetMoveStatus();
    if (Status == EPathFollowingStatus::Idle)
    {
        MoveToNextWaypoint();
    }
}

void ATRexBehaviorController::UpdateInvestigateState(float DeltaTime)
{
    // Move to last known location
    if (StateTimer < 0.5f)
    {
        MoveToLocation(PerceptionData.LastKnownLocation, 150.0f);
    }

    // Give up investigating after timeout
    if (StateTimer >= InvestigateTimeout)
    {
        PerceptionData.LastKnownTarget = nullptr;
        PerceptionData.bTargetInSight = false;
        SetTRexState(ENPC_TRexState::Patrolling);
    }
}

void ATRexBehaviorController::UpdateChaseState(float DeltaTime)
{
    if (!PerceptionData.LastKnownTarget)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
        return;
    }

    // Update move target every 0.3s for responsive chasing
    if (FMath::Fmod(StateTimer, 0.3f) < DeltaTime)
    {
        PerceptionData.LastKnownLocation = PerceptionData.LastKnownTarget->GetActorLocation();
        MoveToActor(PerceptionData.LastKnownTarget, AttackRange * 0.8f);
    }

    // Check attack range
    float Dist = GetDistanceToTarget();
    if (Dist <= AttackRange && !bAttackOnCooldown)
    {
        ExecuteAttack();
    }

    // Lost target for too long — investigate
    if (!PerceptionData.bTargetInSight && PerceptionData.TimeSinceLastSeen > 4.0f)
    {
        SetTRexState(ENPC_TRexState::Investigating);
    }
}

void ATRexBehaviorController::UpdateAttackState(float DeltaTime)
{
    // Attack state is handled by timer in ExecuteAttack()
    // Just ensure we don't get stuck here
    if (StateTimer > 3.0f)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
    }
}

float ATRexBehaviorController::GetDistanceToTarget() const
{
    if (!PerceptionData.LastKnownTarget || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), PerceptionData.LastKnownTarget->GetActorLocation());
}

bool ATRexBehaviorController::IsTargetVisible() const
{
    return PerceptionData.bTargetInSight && PerceptionData.TimeSinceLastSeen < 1.0f;
}

void ATRexBehaviorController::SetMovementSpeed(float Speed)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* Character = Cast<ACharacter>(ControlledPawn);
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}
