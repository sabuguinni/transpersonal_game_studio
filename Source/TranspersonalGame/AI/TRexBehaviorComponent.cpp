#include "TRexBehaviorComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for AI
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Generate default patrol points around spawn location if none provided
    if (PatrolPoints.Num() == 0)
    {
        GenerateRandomPatrolPoints(PatrolRadius, 4);
    }

    TransitionToState(ENPC_TRexState::Patrol);
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent — main AI loop
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateSensoryData(DeltaTime);

    // Decrement roar cooldown
    if (RoarCooldown > 0.0f)
    {
        RoarCooldown -= DeltaTime;
    }

    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:    TickIdle(DeltaTime);    break;
        case ENPC_TRexState::Patrol:  TickPatrol(DeltaTime);  break;
        case ENPC_TRexState::Alert:   TickAlert(DeltaTime);   break;
        case ENPC_TRexState::Chase:   TickChase(DeltaTime);   break;
        case ENPC_TRexState::Attack:  TickAttack(DeltaTime);  break;
        case ENPC_TRexState::Roar:    TickRoar(DeltaTime);    break;
        case ENPC_TRexState::Resting: TickResting(DeltaTime); break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine — transitions
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::TransitionToState(ENPC_TRexState NewState)
{
    if (NewState == CurrentState) return;

    CurrentState = NewState;
    OnStateChanged.Broadcast(NewState);

    // State entry logic
    switch (NewState)
    {
        case ENPC_TRexState::Chase:
            ChaseTimer = 0.0f;
            TriggerRoar(); // T-Rex roars when it starts chasing
            break;

        case ENPC_TRexState::Resting:
            // Rest for a random duration after kill/exhaustion
            PatrolWaitTimer = FMath::RandRange(8.0f, 20.0f);
            break;

        case ENPC_TRexState::Patrol:
            ChaseTimer = 0.0f;
            SensoryData.bTargetVisible = false;
            SensoryData.TimeSinceLastSeen = 999.0f;
            break;

        default:
            break;
    }
}

void UTRexBehaviorComponent::ForceState(ENPC_TRexState NewState)
{
    TransitionToState(NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
// State tick implementations
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::TickIdle(float DeltaTime)
{
    PatrolWaitTimer -= DeltaTime;
    if (PatrolWaitTimer <= 0.0f)
    {
        TransitionToState(ENPC_TRexState::Patrol);
    }

    // Even while idle, check for nearby threats
    AActor* Player = FindNearestPlayer();
    if (Player && CanDetectVibrations(Player))
    {
        CurrentTarget = Player;
        TransitionToState(ENPC_TRexState::Alert);
    }
}

void UTRexBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Check for player detection
    AActor* Player = FindNearestPlayer();
    if (Player)
    {
        if (CanSeeTarget(Player))
        {
            CurrentTarget = Player;
            TransitionToState(ENPC_TRexState::Chase);
            return;
        }
        if (CanDetectVibrations(Player))
        {
            CurrentTarget = Player;
            TransitionToState(ENPC_TRexState::Alert);
            return;
        }
    }

    // Move to current patrol point
    const FNPC_TRexPatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    MoveToLocation(Target.Location, PatrolSpeed);

    // Check if we've reached the patrol point
    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), Target.Location);
    if (DistToTarget < 200.0f) // Within 2m
    {
        // Wait at this point
        PatrolWaitTimer = Target.WaitDuration;
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        TransitionToState(ENPC_TRexState::Idle);
    }
}

void UTRexBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: T-Rex has detected vibrations but not seen the player yet
    // It slows down, looks around, and transitions to Chase if it sees the player

    AActor* Player = FindNearestPlayer();
    if (Player && CanSeeTarget(Player))
    {
        CurrentTarget = Player;
        TransitionToState(ENPC_TRexState::Chase);
        return;
    }

    // After 5 seconds of alert without visual confirmation, return to patrol
    ChaseTimer += DeltaTime;
    if (ChaseTimer >= 5.0f)
    {
        ChaseTimer = 0.0f;
        TransitionToState(ENPC_TRexState::Patrol);
    }
}

void UTRexBehaviorComponent::TickChase(float DeltaTime)
{
    AActor* Target = CurrentTarget.Get();
    if (!Target)
    {
        TransitionToState(ENPC_TRexState::Patrol);
        return;
    }

    // Check if target is in attack range
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());

    if (DistToTarget <= AttackRange)
    {
        TransitionToState(ENPC_TRexState::Attack);
        return;
    }

    // Chase the target
    MoveToLocation(Target->GetActorLocation(), ChaseSpeed);

    // Update sensory data
    if (CanSeeTarget(Target))
    {
        SensoryData.LastKnownTargetLocation = Target->GetActorLocation();
        SensoryData.TimeSinceLastSeen = 0.0f;
        ChaseTimer = 0.0f; // Reset give-up timer when we can see target
    }
    else
    {
        ChaseTimer += DeltaTime;

        // Move to last known location
        if (SensoryData.TimeSinceLastSeen < 2.0f)
        {
            MoveToLocation(SensoryData.LastKnownTargetLocation, ChaseSpeed);
        }

        // Give up after ChaseGiveUpTime seconds without seeing target
        if (ChaseTimer >= ChaseGiveUpTime)
        {
            TransitionToState(ENPC_TRexState::Patrol);
        }
    }
}

void UTRexBehaviorComponent::TickAttack(float DeltaTime)
{
    AActor* Target = CurrentTarget.Get();
    if (!Target)
    {
        TransitionToState(ENPC_TRexState::Patrol);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());

    // If target escaped attack range, resume chase
    if (DistToTarget > AttackRange * 1.5f)
    {
        TransitionToState(ENPC_TRexState::Chase);
    }
    // Otherwise stay in attack state — actual damage dealt by Combat AI Agent #12
}

void UTRexBehaviorComponent::TickRoar(float DeltaTime)
{
    // Roar lasts ~2 seconds then transitions to Chase
    PatrolWaitTimer -= DeltaTime;
    if (PatrolWaitTimer <= 0.0f)
    {
        TransitionToState(ENPC_TRexState::Chase);
    }
}

void UTRexBehaviorComponent::TickResting(float DeltaTime)
{
    PatrolWaitTimer -= DeltaTime;
    if (PatrolWaitTimer <= 0.0f)
    {
        TransitionToState(ENPC_TRexState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Sensory system
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdateSensoryData(float DeltaTime)
{
    SensoryData.TimeSinceLastSeen += DeltaTime;

    AActor* Target = CurrentTarget.Get();
    if (!Target) return;

    SensoryData.bTargetVisible = CanSeeTarget(Target);
    SensoryData.bVibrationsDetected = CanDetectVibrations(Target);

    if (SensoryData.bTargetVisible)
    {
        SensoryData.LastKnownTargetLocation = Target->GetActorLocation();
        SensoryData.TimeSinceLastSeen = 0.0f;
    }
}

bool UTRexBehaviorComponent::CanSeeTarget(AActor* Target) const
{
    if (!Target) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    FVector OwnerLocation = Owner->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();

    // Distance check
    float Distance = FVector::Dist(OwnerLocation, TargetLocation);
    if (Distance > SightRange) return false;

    // Cone check — dot product between forward vector and direction to target
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector OwnerForward = Owner->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(OwnerForward, DirectionToTarget);
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(SightConeHalfAngle));

    if (DotProduct < CosHalfAngle) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        HitResult, OwnerLocation, TargetLocation, ECC_Visibility, Params);

    // If we hit something that isn't the target, line of sight is blocked
    if (bHit && HitResult.GetActor() != Target)
    {
        return false;
    }

    return true;
}

bool UTRexBehaviorComponent::CanDetectVibrations(AActor* Target) const
{
    if (!Target) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    return Distance <= VibrationDetectionRange;
}

// ─────────────────────────────────────────────────────────────────────────────
// Movement
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::MoveToLocation(const FVector& TargetLocation, float Speed)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* Pawn = Cast<APawn>(Owner);
    if (!Pawn) return;

    AAIController* AIController = Cast<AAIController>(Pawn->GetController());
    if (!AIController) return;

    // Use AI MoveToLocation for NavMesh-aware pathfinding
    AIController->MoveToLocation(TargetLocation, 150.0f, true, true, false, true);

    // Set movement speed
    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Patrol point management
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::SetPatrolPoints(const TArray<FNPC_TRexPatrolPoint>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

void UTRexBehaviorComponent::GenerateRandomPatrolPoints(float Radius, int32 NumPoints)
{
    PatrolPoints.Empty();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector SpawnLocation = Owner->GetActorLocation();
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Owner->GetWorld());

    for (int32 i = 0; i < NumPoints; ++i)
    {
        // Generate random point within radius
        float Angle = (360.0f / NumPoints) * i + FMath::RandRange(-30.0f, 30.0f);
        float Distance = FMath::RandRange(Radius * 0.3f, Radius);
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        FVector CandidatePoint = SpawnLocation + Offset;
        FNavLocation NavLocation;

        // Snap to NavMesh if available
        if (NavSys && NavSys->ProjectPointToNavigation(CandidatePoint, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            CandidatePoint = NavLocation.Location;
        }

        FNPC_TRexPatrolPoint Point;
        Point.Location = CandidatePoint;
        Point.WaitDuration = FMath::RandRange(2.0f, 6.0f);
        PatrolPoints.Add(Point);
    }

    CurrentPatrolIndex = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat events
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::OnTargetEnteredAttackRange(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        TransitionToState(ENPC_TRexState::Attack);
    }
}

void UTRexBehaviorComponent::OnTargetLeftAttackRange(AActor* Target)
{
    if (CurrentState == ENPC_TRexState::Attack)
    {
        TransitionToState(ENPC_TRexState::Chase);
    }
}

void UTRexBehaviorComponent::OnKillConfirmed()
{
    CurrentTarget.Reset();
    SensoryData = FNPC_TRexSensoryData(); // Reset sensory data
    TransitionToState(ENPC_TRexState::Resting);
}

// ─────────────────────────────────────────────────────────────────────────────
// Roar
// ─────────────────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::TriggerRoar()
{
    if (RoarCooldown > 0.0f) return; // Don't roar if on cooldown

    // Broadcast roar event — PlayerAnimInstance.FearAlpha driven by this
    OnRoar.Broadcast(RoarFearIntensity);

    // Enter roar state briefly before chasing
    PatrolWaitTimer = 2.0f; // Roar lasts 2 seconds
    RoarCooldown = 30.0f;   // Can't roar again for 30 seconds

    // Temporarily override state to Roar (will return to Chase in TickRoar)
    CurrentState = ENPC_TRexState::Roar;
    OnStateChanged.Broadcast(ENPC_TRexState::Roar);
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility
// ─────────────────────────────────────────────────────────────────────────────

AActor* UTRexBehaviorComponent::FindNearestPlayer() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    UWorld* World = Owner->GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    return PlayerPawn;
}
