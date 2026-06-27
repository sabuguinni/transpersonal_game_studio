#include "TRexBehavior.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set up AI Perception (sight)
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRange;
    SightConfig->LoseSightRadius = SightRange * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = SightAngleDegrees * 0.5f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &ATRexBehaviorController::OnTargetPerceptionUpdated);
    }

    // Generate default patrol waypoints if none assigned
    if (PatrolWaypoints.Num() == 0)
    {
        FVector Origin = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        const float R = PatrolRadius * 0.3f;
        const TArray<FVector> Offsets = {
            FVector(R, 0, 0), FVector(0, R, 0),
            FVector(-R, 0, 0), FVector(0, -R, 0)
        };
        for (const FVector& Offset : Offsets)
        {
            FNPC_TRexPatrolPoint WP;
            WP.Location = Origin + Offset;
            WP.WaitTime = FMath::RandRange(1.5f, 3.5f);
            WP.bRoarOnArrival = (FMath::RandRange(0, 3) == 0);
            PatrolWaypoints.Add(WP);
        }
    }

    SetState(ENPC_TRexState::Patrol);
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Set movement speed for patrol
    if (ACharacter* Char = Cast<ACharacter>(InPawn))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->MaxWalkSpeed = PatrolMoveSpeed;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::OnUnPossess()
{
    Super::OnUnPossess();
    CurrentTarget = nullptr;
    CurrentState = ENPC_TRexState::Patrol;
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update attack cooldown
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // State machine tick
    switch (CurrentState)
    {
    case ENPC_TRexState::Patrol:
        TryDetectPlayer();
        UpdatePatrol(DeltaTime);
        break;

    case ENPC_TRexState::Alert:
        TryDetectPlayer();
        break;

    case ENPC_TRexState::Chase:
        UpdateChase(DeltaTime);
        break;

    case ENPC_TRexState::Attack:
        UpdateAttack(DeltaTime);
        break;

    case ENPC_TRexState::Roar:
        // Roar handled by animation — transition back after 2s via anim notify
        break;

    case ENPC_TRexState::Rest:
        // Idle — check for nearby player
        TryDetectPlayer();
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    // Adjust movement speed per state
    if (ACharacter* Char = Cast<ACharacter>(MyPawn))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            switch (NewState)
            {
            case ENPC_TRexState::Patrol:
            case ENPC_TRexState::Alert:
            case ENPC_TRexState::Rest:
                Move->MaxWalkSpeed = PatrolMoveSpeed;
                break;
            case ENPC_TRexState::Chase:
                Move->MaxWalkSpeed = ChaseMoveSpeed;
                break;
            case ENPC_TRexState::Attack:
            case ENPC_TRexState::Roar:
                Move->MaxWalkSpeed = 0.0f;
                StopMovement();
                break;
            }
        }
    }

    // Update blackboard
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsEnum(BB_AIState, static_cast<uint8>(NewState));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::TryDetectPlayer()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return;

    const float DistSq = FVector::DistSquared(MyPawn->GetActorLocation(), PlayerChar->GetActorLocation());
    const float SightRangeSq = SightRange * SightRange;
    const float AttackRangeSq = AttackRange * AttackRange;

    if (DistSq <= AttackRangeSq)
    {
        CurrentTarget = PlayerChar;
        SetState(ENPC_TRexState::Attack);
    }
    else if (DistSq <= SightRangeSq && IsPlayerInSightCone(PlayerChar))
    {
        if (CurrentTarget == nullptr)
        {
            // First detection — roar
            CurrentTarget = PlayerChar;
            SetState(ENPC_TRexState::Roar);
        }
        else
        {
            CurrentTarget = PlayerChar;
            SetState(ENPC_TRexState::Chase);
        }
    }
    else if (CurrentState == ENPC_TRexState::Chase || CurrentState == ENPC_TRexState::Alert)
    {
        // Lost sight — return to patrol
        if (DistSq > SightRangeSq * 1.44f) // 1.2x sight range squared
        {
            CurrentTarget = nullptr;
            SetState(ENPC_TRexState::Patrol);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
bool ATRexBehaviorController::IsPlayerInSightCone(AActor* Player) const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn || !Player) return false;

    const FVector ToPlayer = (Player->GetActorLocation() - MyPawn->GetActorLocation()).GetSafeNormal();
    const FVector Forward = MyPawn->GetActorForwardVector();
    const float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    const float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(SightAngleDegrees * 0.5f));

    return DotProduct >= HalfAngleCos;
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::UpdatePatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    if (bWaitingAtWaypoint)
    {
        WaypointWaitTimer -= DeltaTime;
        if (WaypointWaitTimer <= 0.0f)
        {
            bWaitingAtWaypoint = false;
            AdvancePatrolWaypoint();
        }
        return;
    }

    const FNPC_TRexPatrolPoint& WP = PatrolWaypoints[CurrentWaypointIndex];
    const float DistToWP = FVector::Dist(MyPawn->GetActorLocation(), WP.Location);

    if (DistToWP < 200.0f)
    {
        // Arrived at waypoint
        bWaitingAtWaypoint = true;
        WaypointWaitTimer = WP.WaitTime;

        if (WP.bRoarOnArrival)
        {
            SetState(ENPC_TRexState::Roar);
        }
    }
    else
    {
        // Move toward waypoint
        MoveToLocation(WP.Location, 150.0f, true, true, false, true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::AdvancePatrolWaypoint()
{
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsVector(BB_PatrolLocation, PatrolWaypoints[CurrentWaypointIndex].Location);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::UpdateChase(float DeltaTime)
{
    if (!CurrentTarget) 
    {
        SetState(ENPC_TRexState::Patrol);
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    const float Dist = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist <= AttackRange)
    {
        SetState(ENPC_TRexState::Attack);
        return;
    }

    if (Dist > SightRange * 1.2f)
    {
        CurrentTarget = nullptr;
        SetState(ENPC_TRexState::Patrol);
        return;
    }

    MoveToActor(CurrentTarget, AttackRange * 0.8f, true, true, false);

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(BB_TargetActor, CurrentTarget);
        BB->SetValueAsFloat(BB_DistanceToTarget, Dist);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::UpdateAttack(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetState(ENPC_TRexState::Patrol);
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    const float Dist = FVector::Dist(MyPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Dist > AttackRange * 1.5f)
    {
        SetState(ENPC_TRexState::Chase);
        return;
    }

    // Execute attack on cooldown
    if (AttackCooldownTimer <= 0.0f)
    {
        AttackCooldownTimer = AttackCooldown;

        // Apply damage via UE5 damage system
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            AttackDamage,
            this,
            MyPawn,
            nullptr
        );

        UE_LOG(LogTemp, Log, TEXT("TRex ATTACK: dealt %.0f damage to %s"),
            AttackDamage, *CurrentTarget->GetName());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ATRexBehaviorController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to player character
    if (!Cast<ACharacter>(Actor)) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;
        if (CurrentState == ENPC_TRexState::Patrol || CurrentState == ENPC_TRexState::Rest)
        {
            SetState(ENPC_TRexState::Alert);
        }
    }
    else
    {
        // Lost perception — don't immediately drop target, let distance check handle it
        UE_LOG(LogTemp, Log, TEXT("TRex lost sight of %s"), *Actor->GetName());
    }
}
