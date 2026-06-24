#include "NPCBehaviorTree.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

ANPCBehaviorController::ANPCBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ANPCBehaviorController::OnPerceptionUpdated);
}

// ============================================================
// Lifecycle
// ============================================================

void ANPCBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    PlayerPawn = FindPlayerPawn();
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void ANPCBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update attack cooldown
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // Update perception data
    UpdatePerception(DeltaTime);

    // State machine dispatch
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        HandleIdleState(DeltaTime);
        break;
    case ENPC_BehaviorState::Patrol:
        HandlePatrolState(DeltaTime);
        break;
    case ENPC_BehaviorState::Alert:
        HandleAlertState(DeltaTime);
        break;
    case ENPC_BehaviorState::Chase:
        HandleChaseState(DeltaTime);
        break;
    case ENPC_BehaviorState::Attack:
        HandleAttackState(DeltaTime);
        break;
    case ENPC_BehaviorState::Flee:
        HandleFleeState(DeltaTime);
        break;
    default:
        break;
    }
}

void ANPCBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PlayerPawn = FindPlayerPawn();

    // Apply movement speed from config
    if (InPawn)
    {
        UCharacterMovementComponent* MovComp = InPawn->FindComponentByClass<UCharacterMovementComponent>();
        if (MovComp)
        {
            MovComp->MaxWalkSpeed = BehaviorConfig.MoveSpeed_Patrol;
        }
    }
}

void ANPCBehaviorController::OnUnPossess()
{
    Super::OnUnPossess();
}

// ============================================================
// State Machine
// ============================================================

void ANPCBehaviorController::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    UCharacterMovementComponent* MovComp = ControlledPawn->FindComponentByClass<UCharacterMovementComponent>();
    if (!MovComp) return;

    switch (NewState)
    {
    case ENPC_BehaviorState::Patrol:
        MovComp->MaxWalkSpeed = BehaviorConfig.MoveSpeed_Patrol;
        break;
    case ENPC_BehaviorState::Chase:
    case ENPC_BehaviorState::Attack:
        MovComp->MaxWalkSpeed = BehaviorConfig.MoveSpeed_Chase;
        break;
    case ENPC_BehaviorState::Flee:
        MovComp->MaxWalkSpeed = BehaviorConfig.MoveSpeed_Chase * 1.2f;
        break;
    default:
        MovComp->MaxWalkSpeed = BehaviorConfig.MoveSpeed_Patrol;
        break;
    }
}

// ============================================================
// Patrol
// ============================================================

void ANPCBehaviorController::SetPatrolWaypoints(const TArray<FNPC_PatrolWaypoint>& Waypoints)
{
    PatrolWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
    bWaitingAtWaypoint = false;
}

void ANPCBehaviorController::AdvanceToNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0) return;
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    bWaitingAtWaypoint = false;
}

// ============================================================
// Perception
// ============================================================

void ANPCBehaviorController::UpdatePerception(float DeltaTime)
{
    if (!PlayerPawn)
    {
        PlayerPawn = FindPlayerPawn();
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector MyLocation = ControlledPawn->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    PerceptionData.DistanceToPlayer = FVector::Dist(MyLocation, PlayerLocation);

    if (PerceptionData.bCanSeePlayer)
    {
        PerceptionData.LastKnownPlayerLocation = PlayerLocation;
        PerceptionData.TimeSinceLastSeen = 0.0f;
    }
    else
    {
        PerceptionData.TimeSinceLastSeen += DeltaTime;
    }
}

void ANPCBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FAIStimulus Stimulus;
        if (PerceptionComp->HasAnyCurrentStimulus(*Actor))
        {
            PerceptionData.bCanSeePlayer = true;
            PerceptionData.LastKnownPlayerLocation = Actor->GetActorLocation();
            PerceptionData.TimeSinceLastSeen = 0.0f;

            // Transition to chase if not already attacking
            if (CurrentState == ENPC_BehaviorState::Patrol ||
                CurrentState == ENPC_BehaviorState::Idle ||
                CurrentState == ENPC_BehaviorState::Alert)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
        }
        else
        {
            PerceptionData.bCanSeePlayer = false;
        }
    }
}

// ============================================================
// Combat
// ============================================================

void ANPCBehaviorController::ExecuteAttack()
{
    if (!CanAttack()) return;
    AttackCooldownTimer = BehaviorConfig.AttackCooldown;

    // Damage is applied by the pawn's attack animation notify
    // This controller just resets the cooldown and logs
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorController: Attack executed by %s"),
        GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

bool ANPCBehaviorController::CanAttack() const
{
    return AttackCooldownTimer <= 0.0f;
}

// ============================================================
// State Handlers
// ============================================================

void ANPCBehaviorController::HandleIdleState(float DeltaTime)
{
    // After 3 seconds idle, begin patrol
    WaypointWaitTimer += DeltaTime;
    if (WaypointWaitTimer >= 3.0f)
    {
        WaypointWaitTimer = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void ANPCBehaviorController::HandlePatrolState(float DeltaTime)
{
    // Check if player is close enough to detect
    if (PerceptionData.DistanceToPlayer <= BehaviorConfig.DetectRadius &&
        PerceptionData.bCanSeePlayer)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    if (PatrolWaypoints.Num() == 0) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    if (bWaitingAtWaypoint)
    {
        WaypointWaitTimer += DeltaTime;
        if (WaypointWaitTimer >= PatrolWaypoints[CurrentWaypointIndex].WaitTime)
        {
            AdvanceToNextWaypoint();
        }
        return;
    }

    // Move toward current waypoint
    FVector TargetLocation = PatrolWaypoints[CurrentWaypointIndex].Location;
    float DistToWaypoint = FVector::Dist(ControlledPawn->GetActorLocation(), TargetLocation);

    if (DistToWaypoint < 200.0f)
    {
        bWaitingAtWaypoint = true;
        WaypointWaitTimer = 0.0f;
    }
    else
    {
        MoveToLocation(TargetLocation, 150.0f);
    }
}

void ANPCBehaviorController::HandleAlertState(float DeltaTime)
{
    // Brief alert pause before committing to chase
    WaypointWaitTimer += DeltaTime;
    if (WaypointWaitTimer >= 1.0f)
    {
        WaypointWaitTimer = 0.0f;
        if (PerceptionData.bCanSeePlayer)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
}

void ANPCBehaviorController::HandleChaseState(float DeltaTime)
{
    if (!PlayerPawn) return;

    // Attack range check
    if (PerceptionData.DistanceToPlayer <= BehaviorConfig.AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
        return;
    }

    // Lost sight — return to patrol after 5 seconds
    if (!PerceptionData.bCanSeePlayer && PerceptionData.TimeSinceLastSeen > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }

    // Move toward last known player location
    MoveToLocation(PerceptionData.LastKnownPlayerLocation, 100.0f);
}

void ANPCBehaviorController::HandleAttackState(float DeltaTime)
{
    if (!PlayerPawn) return;

    // Player escaped attack range — resume chase
    if (PerceptionData.DistanceToPlayer > BehaviorConfig.AttackRadius * 1.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
        return;
    }

    // Execute attack on cooldown
    if (CanAttack())
    {
        ExecuteAttack();
    }

    // Face the player
    FVector ToPlayer = (PlayerPawn->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FRotator LookAt = ToPlayer.Rotation();
    SetControlRotation(LookAt);
}

void ANPCBehaviorController::HandleFleeState(float DeltaTime)
{
    if (!PlayerPawn) return;

    // Flee away from player
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector FleeDirection = (MyLocation - PlayerLocation).GetSafeNormal();
    FVector FleeTarget = MyLocation + FleeDirection * 3000.0f;

    MoveToLocation(FleeTarget, 200.0f);

    // Stop fleeing when far enough
    if (PerceptionData.DistanceToPlayer > BehaviorConfig.ChaseRadius * 1.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ============================================================
// Helpers
// ============================================================

APawn* ANPCBehaviorController::FindPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    return PC->GetPawn();
}
