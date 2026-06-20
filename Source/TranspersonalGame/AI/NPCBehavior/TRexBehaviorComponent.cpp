#include "TRexBehaviorComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Patrolling;
    TimeSinceLastAttack = 0.f;
    RestTimer = 0.f;
    InvestigateTimer = 0.f;
    PatrolTarget = FVector::ZeroVector;
    HomeLocation = FVector::ZeroVector;
    InvestigateTarget = FVector::ZeroVector;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        PatrolTarget = HomeLocation;
    }

    PickNewPatrolTarget();
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastAttack += DeltaTime;

    // Update memory — decay sighting timer
    if (Memory.bHasSeenPlayer)
    {
        Memory.TimeSinceLastSighting += DeltaTime;
        if (Memory.TimeSinceLastSighting > MemoryDuration)
        {
            Memory.bHasSeenPlayer = false;
            Memory.bIsAlerted = false;
            Memory.TimeSinceLastSighting = 0.f;
        }
    }

    // State machine
    switch (CurrentState)
    {
        case ENPC_TRexState::Patrolling:    UpdatePatrol(DeltaTime);      break;
        case ENPC_TRexState::Investigating: UpdateInvestigate(DeltaTime); break;
        case ENPC_TRexState::Chasing:       UpdateChase(DeltaTime);       break;
        case ENPC_TRexState::Attacking:     UpdateAttack(DeltaTime);      break;
        case ENPC_TRexState::Resting:       UpdateResting(DeltaTime);     break;
    }
}

// ─── State: Patrol ────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    // Check if player entered chase range
    if (IsPlayerInChaseRange() && HasLineOfSightToPlayer())
    {
        Memory.bHasSeenPlayer = true;
        Memory.bIsAlerted = true;
        Memory.TimeSinceLastSighting = 0.f;
        if (APawn* Player = GetPlayerPawn())
        {
            Memory.LastKnownPlayerLocation = Player->GetActorLocation();
        }
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    // Move toward patrol target
    MoveToward(PatrolTarget, PatrolSpeed, DeltaTime);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), PatrolTarget);
    if (DistToTarget < 150.f)
    {
        // Reached waypoint — rest briefly then pick new target
        RestTimer = FMath::RandRange(3.f, 8.f);
        TransitionTo(ENPC_TRexState::Resting);
    }
}

// ─── State: Investigate ───────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdateInvestigate(float DeltaTime)
{
    InvestigateTimer += DeltaTime;

    // Player re-enters range during investigation
    if (IsPlayerInChaseRange() && HasLineOfSightToPlayer())
    {
        Memory.bHasSeenPlayer = true;
        Memory.bIsAlerted = true;
        Memory.TimeSinceLastSighting = 0.f;
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    MoveToward(InvestigateTarget, PatrolSpeed * 1.3f, DeltaTime);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToInvestigate = FVector::Dist(Owner->GetActorLocation(), InvestigateTarget);
    if (DistToInvestigate < 200.f || InvestigateTimer > 12.f)
    {
        InvestigateTimer = 0.f;
        PickNewPatrolTarget();
        TransitionTo(ENPC_TRexState::Patrolling);
    }
}

// ─── State: Chase ─────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdateChase(float DeltaTime)
{
    APawn* Player = GetPlayerPawn();
    if (!Player)
    {
        TransitionTo(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToPlayer = GetDistanceToPlayer();

    // Attack range reached
    if (DistToPlayer <= AttackRange)
    {
        TransitionTo(ENPC_TRexState::Attacking);
        return;
    }

    // Lost sight — investigate last known position
    if (!HasLineOfSightToPlayer())
    {
        Memory.TimeSinceLastSighting += DeltaTime;
        if (Memory.TimeSinceLastSighting > 3.f)
        {
            InvestigateTarget = Memory.LastKnownPlayerLocation;
            InvestigateTimer = 0.f;
            TransitionTo(ENPC_TRexState::Investigating);
            return;
        }
    }
    else
    {
        Memory.LastKnownPlayerLocation = Player->GetActorLocation();
        Memory.TimeSinceLastSighting = 0.f;
    }

    MoveToward(Player->GetActorLocation(), ChaseSpeed, DeltaTime);
}

// ─── State: Attack ────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime)
{
    APawn* Player = GetPlayerPawn();
    if (!Player)
    {
        TransitionTo(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToPlayer = GetDistanceToPlayer();

    // Player escaped attack range — resume chase
    if (DistToPlayer > AttackRange * 1.5f)
    {
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    // Perform attack on cooldown
    if (TimeSinceLastAttack >= AttackCooldown)
    {
        PerformAttack();
    }

    // Face the player
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector Dir = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
        FRotator LookAt = Dir.Rotation();
        Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 5.f));
    }
}

// ─── State: Rest ─────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::UpdateResting(float DeltaTime)
{
    RestTimer -= DeltaTime;

    // Alert check even while resting
    if (IsPlayerInChaseRange() && HasLineOfSightToPlayer())
    {
        Memory.bHasSeenPlayer = true;
        Memory.bIsAlerted = true;
        Memory.TimeSinceLastSighting = 0.f;
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    if (RestTimer <= 0.f)
    {
        PickNewPatrolTarget();
        TransitionTo(ENPC_TRexState::Patrolling);
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void UTRexBehaviorComponent::TransitionTo(ENPC_TRexState NewState)
{
    CurrentState = NewState;
}

void UTRexBehaviorComponent::PickNewPatrolTarget()
{
    float Angle = FMath::RandRange(0.f, 360.f);
    float Radius = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    float X = HomeLocation.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = HomeLocation.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    PatrolTarget = FVector(X, Y, HomeLocation.Z);
}

void UTRexBehaviorComponent::MoveToward(const FVector& Target, float Speed, float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLoc = Owner->GetActorLocation();
    FVector Dir = (Target - CurrentLoc).GetSafeNormal2D();

    if (Dir.IsNearlyZero()) return;

    FVector NewLoc = CurrentLoc + Dir * Speed * DeltaTime;
    Owner->SetActorLocation(NewLoc, true);

    FRotator LookAt = Dir.Rotation();
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 4.f));
}

void UTRexBehaviorComponent::PerformAttack()
{
    TimeSinceLastAttack = 0.f;

    APawn* Player = GetPlayerPawn();
    if (!Player) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(Player, AttackDamage, nullptr, GetOwner(), nullptr);
}

void UTRexBehaviorComponent::ForceState(ENPC_TRexState NewState)
{
    TransitionTo(NewState);
}

// ─── Queries ─────────────────────────────────────────────────────────────────

bool UTRexBehaviorComponent::IsPlayerInChaseRange() const
{
    return GetDistanceToPlayer() <= ChaseRange;
}

bool UTRexBehaviorComponent::IsPlayerInAttackRange() const
{
    return GetDistanceToPlayer() <= AttackRange;
}

bool UTRexBehaviorComponent::HasLineOfSightToPlayer() const
{
    AActor* Owner = GetOwner();
    APawn* Player = GetPlayerPawn();
    if (!Owner || !Player) return false;

    UWorld* World = Owner->GetWorld();
    if (!World) return false;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bBlocked = World->LineTraceSingleByChannel(
        Hit,
        Owner->GetActorLocation() + FVector(0, 0, 100.f),
        Player->GetActorLocation() + FVector(0, 0, 80.f),
        ECC_Visibility,
        Params
    );

    // No blocking hit means clear line of sight
    return !bBlocked || Hit.GetActor() == Player;
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Owner = GetOwner();
    APawn* Player = GetPlayerPawn();
    if (!Owner || !Player) return TNumericLimits<float>::Max();

    return FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
}

APawn* UTRexBehaviorComponent::GetPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    return PC->GetPawn();
}
