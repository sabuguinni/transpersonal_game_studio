// DinosaurAIController.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_006
// Full implementation of ADinosaurAIController FSM.

#include "DinosaurAIController.h"
#include "DinosaurBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — sufficient for AI FSM

    // Default detection parameters (tuned for T-Rex; child BPs override)
    DetectionRadius    = 3000.0f;  // 30 m
    AttackRadius       = 250.0f;   // 2.5 m
    VisionHalfAngleDeg = 45.0f;    // 90° total FOV
    PatrolRadius       = 2000.0f;  // 20 m patrol range
    PatrolWaitTime     = 3.0f;
    AttackCooldown     = 2.0f;
    AttackDamage       = 30.0f;

    CurrentState         = ECore_DinoAIState::Idle;
    IdleTimer            = 0.0f;
    AttackTimer          = 0.0f;
    ControlledDino       = nullptr;
    ChaseTarget          = nullptr;
    SpawnLocation        = FVector::ZeroVector;
    PatrolTarget         = FVector::ZeroVector;
    LastKnownPlayerLocation = FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// Possess / UnPossess
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledDino = Cast<ADinosaurBase>(InPawn);
    if (InPawn)
    {
        SpawnLocation = InPawn->GetActorLocation();
    }

    SetAIState(ECore_DinoAIState::Idle);
}

void ADinosaurAIController::OnUnPossess()
{
    Super::OnUnPossess();
    ControlledDino = nullptr;
    ChaseTarget    = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — dispatch to current state handler
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn() || CurrentState == ECore_DinoAIState::Dead)
    {
        return;
    }

    switch (CurrentState)
    {
        case ECore_DinoAIState::Idle:   HandleIdle(DeltaTime);   break;
        case ECore_DinoAIState::Patrol: HandlePatrol(DeltaTime); break;
        case ECore_DinoAIState::Chase:  HandleChase(DeltaTime);  break;
        case ECore_DinoAIState::Attack: HandleAttack(DeltaTime); break;
        case ECore_DinoAIState::Flee:   HandleFlee(DeltaTime);   break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::SetAIState(ECore_DinoAIState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    IdleTimer    = 0.0f;
    AttackTimer  = 0.0f;

    // On entering Patrol, pick a destination immediately
    if (NewState == ECore_DinoAIState::Patrol)
    {
        PatrolTarget = PickPatrolDestination();
        MoveToDestination(PatrolTarget);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Handlers
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::HandleIdle(float DeltaTime)
{
    IdleTimer += DeltaTime;

    // Check for player every tick
    APawn* Player = FindPlayerInRange();
    if (Player)
    {
        ChaseTarget = Player;
        SetAIState(ECore_DinoAIState::Chase);
        return;
    }

    // After waiting, begin patrol
    if (IdleTimer >= PatrolWaitTime)
    {
        SetAIState(ECore_DinoAIState::Patrol);
    }
}

void ADinosaurAIController::HandlePatrol(float DeltaTime)
{
    // Check for player
    APawn* Player = FindPlayerInRange();
    if (Player)
    {
        ChaseTarget = Player;
        StopMovement();
        SetAIState(ECore_DinoAIState::Chase);
        return;
    }

    // Check if we reached the patrol point
    EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        // Arrived — wait then pick new point
        IdleTimer += DeltaTime;
        if (IdleTimer >= PatrolWaitTime)
        {
            IdleTimer    = 0.0f;
            PatrolTarget = PickPatrolDestination();
            MoveToDestination(PatrolTarget);
        }
    }
}

void ADinosaurAIController::HandleChase(float DeltaTime)
{
    if (!ChaseTarget || !ChaseTarget->IsValidLowLevel())
    {
        SetAIState(ECore_DinoAIState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), ChaseTarget->GetActorLocation());

    // Within attack range → attack
    if (DistToTarget <= AttackRadius)
    {
        StopMovement();
        SetAIState(ECore_DinoAIState::Attack);
        return;
    }

    // Lost sight (too far) → return to patrol
    if (DistToTarget > DetectionRadius * 1.5f)
    {
        ChaseTarget = nullptr;
        SetAIState(ECore_DinoAIState::Patrol);
        return;
    }

    // Keep chasing
    LastKnownPlayerLocation = ChaseTarget->GetActorLocation();
    MoveToDestination(LastKnownPlayerLocation);
}

void ADinosaurAIController::HandleAttack(float DeltaTime)
{
    AttackTimer += DeltaTime;

    if (!ChaseTarget || !ChaseTarget->IsValidLowLevel())
    {
        SetAIState(ECore_DinoAIState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), ChaseTarget->GetActorLocation());

    // Target moved out of attack range — chase again
    if (DistToTarget > AttackRadius * 1.5f)
    {
        SetAIState(ECore_DinoAIState::Chase);
        return;
    }

    // Attack on cooldown
    if (AttackTimer >= AttackCooldown)
    {
        AttackTimer = 0.0f;
        PerformAttack(ChaseTarget);
    }
}

void ADinosaurAIController::HandleFlee(float DeltaTime)
{
    // Flee: move away from spawn toward a random distant point.
    // Transition back to Idle after 10 seconds.
    IdleTimer += DeltaTime;
    if (IdleTimer >= 10.0f)
    {
        SetAIState(ECore_DinoAIState::Idle);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Detection
// ─────────────────────────────────────────────────────────────────────────────

APawn* ADinosaurAIController::FindPlayerInRange() const
{
    UWorld* World = GetWorld();
    if (!World || !GetPawn())
    {
        return nullptr;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    FVector MyLocation     = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float   Distance       = FVector::Dist(MyLocation, PlayerLocation);

    if (Distance > DetectionRadius)
    {
        return nullptr;
    }

    // Vision cone check
    FVector ToPlayer    = (PlayerLocation - MyLocation).GetSafeNormal();
    FVector ForwardDir  = GetPawn()->GetActorForwardVector();
    float   DotProduct  = FVector::DotProduct(ForwardDir, ToPlayer);
    float   CosHalfFOV  = FMath::Cos(FMath::DegreesToRadians(VisionHalfAngleDeg));

    if (DotProduct < CosHalfFOV)
    {
        return nullptr; // Outside vision cone
    }

    // Line-of-sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());

    bool bBlocked = World->LineTraceSingleByChannel(
        HitResult,
        MyLocation,
        PlayerLocation,
        ECC_Visibility,
        Params
    );

    if (bBlocked && HitResult.GetActor() != PlayerPawn)
    {
        return nullptr; // Obstructed
    }

    return PlayerPawn;
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation helpers
// ─────────────────────────────────────────────────────────────────────────────

FVector ADinosaurAIController::PickPatrolDestination() const
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        return SpawnLocation;
    }

    FNavLocation NavLocation;
    bool bFound = NavSys->GetRandomReachablePointInRadius(SpawnLocation, PatrolRadius, NavLocation);

    return bFound ? NavLocation.Location : SpawnLocation;
}

void ADinosaurAIController::MoveToDestination(const FVector& Destination)
{
    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(Destination);
    MoveRequest.SetAcceptanceRadius(100.0f);
    MoveRequest.SetUsePathfinding(true);
    MoveTo(MoveRequest);
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::PerformAttack(APawn* Target)
{
    if (!Target || !GetPawn())
    {
        return;
    }

    // Apply damage via UE5 damage system — works with any AActor that handles TakeDamage
    UGameplayStatics::ApplyDamage(
        Target,
        AttackDamage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );
}
