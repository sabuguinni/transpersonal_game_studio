#include "TRexBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for large creature AI
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetOwner()->GetActorLocation();
    PickNewPatrolTarget();
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay attack cooldown
    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
    }

    // Decay alert level over time
    if (Memory.AlertLevel > 0.0f && CurrentState == ENPC_TRexState::Patrol)
    {
        Memory.AlertLevel = FMath::Max(0.0f, Memory.AlertLevel - AlertDecayRate * DeltaTime);
        Memory.TimeSinceLastSighting += DeltaTime;
    }

    AActor* Player = FindPlayer();

    switch (CurrentState)
    {
    case ENPC_TRexState::Patrol:
        UpdatePatrol(DeltaTime);
        // Check for player detection
        if (Player && CanSeePlayer(Player))
        {
            Memory.LastKnownPlayerLocation = Player->GetActorLocation();
            Memory.bPlayerEverSeen = true;
            Memory.AlertLevel = 100.0f;
            Memory.TimeSinceLastSighting = 0.0f;
            SetState(ENPC_TRexState::Roar); // T-Rex roars before charging
        }
        break;

    case ENPC_TRexState::Alerted:
        UpdateAlerted(DeltaTime);
        if (Player && CanSeePlayer(Player))
        {
            Memory.LastKnownPlayerLocation = Player->GetActorLocation();
            Memory.AlertLevel = 100.0f;
            SetState(ENPC_TRexState::Chase);
        }
        break;

    case ENPC_TRexState::Chase:
        if (Player)
        {
            UpdateChase(DeltaTime, Player);
            float Dist = DistanceToPlayer(Player);
            if (Dist <= AttackRange)
            {
                SetState(ENPC_TRexState::Attack);
            }
            else if (Dist > DetectionRange * 1.5f)
            {
                // Lost player — go alerted then patrol
                SetState(ENPC_TRexState::Alerted);
            }
        }
        else
        {
            SetState(ENPC_TRexState::Alerted);
        }
        break;

    case ENPC_TRexState::Attack:
        if (Player)
        {
            UpdateAttack(DeltaTime, Player);
            float Dist = DistanceToPlayer(Player);
            if (Dist > AttackRange * 1.5f)
            {
                SetState(ENPC_TRexState::Chase);
            }
        }
        else
        {
            SetState(ENPC_TRexState::Patrol);
        }
        break;

    case ENPC_TRexState::Roar:
        UpdateRoar(DeltaTime);
        break;

    case ENPC_TRexState::Rest:
        // Idle — periodically check for threats
        if (Player && CanSeePlayer(Player))
        {
            SetState(ENPC_TRexState::Roar);
        }
        break;
    }
}

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist2D(OwnerLoc, CurrentPatrolTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — pick a new one
        PickNewPatrolTarget();
    }

    // Move toward patrol target
    FVector Direction = (CurrentPatrolTarget - OwnerLoc).GetSafeNormal2D();
    FVector NewLocation = OwnerLoc + Direction * PatrolSpeed * DeltaTime;

    // Rotate to face direction
    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRot = Direction.Rotation();
        FRotator CurrentRot = Owner->GetActorRotation();
        FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 2.0f);
        Owner->SetActorRotation(SmoothedRot);
    }

    Owner->SetActorLocation(NewLocation, true);
}

void UTRexBehaviorComponent::UpdateAlerted(float DeltaTime)
{
    // Move toward last known player location
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToLastKnown = FVector::Dist2D(OwnerLoc, Memory.LastKnownPlayerLocation);

    if (DistToLastKnown < 300.0f || Memory.TimeSinceLastSighting > 15.0f)
    {
        // Gave up searching — return to patrol
        Memory.AlertLevel = FMath::Max(0.0f, Memory.AlertLevel - 50.0f);
        SetState(ENPC_TRexState::Patrol);
        PickNewPatrolTarget();
        return;
    }

    FVector Direction = (Memory.LastKnownPlayerLocation - OwnerLoc).GetSafeNormal2D();
    FVector NewLocation = OwnerLoc + Direction * (PatrolSpeed * 1.5f) * DeltaTime;

    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRot = Direction.Rotation();
        FRotator CurrentRot = Owner->GetActorRotation();
        FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 3.0f);
        Owner->SetActorRotation(SmoothedRot);
    }

    Owner->SetActorLocation(NewLocation, true);
    Memory.TimeSinceLastSighting += DeltaTime;
}

void UTRexBehaviorComponent::UpdateChase(float DeltaTime, AActor* Player)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Player) return;

    Memory.LastKnownPlayerLocation = Player->GetActorLocation();

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector Direction = (Memory.LastKnownPlayerLocation - OwnerLoc).GetSafeNormal2D();
    FVector NewLocation = OwnerLoc + Direction * ChaseSpeed * DeltaTime;

    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRot = Direction.Rotation();
        FRotator CurrentRot = Owner->GetActorRotation();
        FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.0f);
        Owner->SetActorRotation(SmoothedRot);
    }

    Owner->SetActorLocation(NewLocation, true);
}

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime, AActor* Player)
{
    if (!Player) return;

    if (AttackCooldownRemaining <= 0.0f)
    {
        // Execute attack
        OnTRexAttack(Player);
        AttackCooldownRemaining = AttackCooldown;

        // Apply damage if player has health component (interface call)
        // Damage application handled via Blueprint event OnTRexAttack
    }
}

void UTRexBehaviorComponent::UpdateRoar(float DeltaTime)
{
    RoarTimer += DeltaTime;
    if (RoarTimer >= RoarDuration)
    {
        RoarTimer = 0.0f;
        SetState(ENPC_TRexState::Chase);
    }
}

void UTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;

    if (NewState == ENPC_TRexState::Roar)
    {
        RoarTimer = 0.0f;
        OnTRexRoar();
    }

    OnTRexStateChanged(NewState);
}

void UTRexBehaviorComponent::PickNewPatrolTarget()
{
    // Random point within patrol radius of origin
    float Angle = FMath::FRandRange(0.0f, 360.0f);
    float Dist = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);
    CurrentPatrolTarget = PatrolOrigin + FVector(FMath::Cos(RadAngle) * Dist, FMath::Sin(RadAngle) * Dist, 0.0f);
}

AActor* UTRexBehaviorComponent::FindPlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    return PC->GetPawn();
}

bool UTRexBehaviorComponent::CanSeePlayer(AActor* Player) const
{
    if (!Player) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Dist = DistanceToPlayer(Player);
    if (Dist > DetectionRange) return false;

    // Line of sight check
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    Params.AddIgnoredActor(Player);

    FVector Start = Owner->GetActorLocation() + FVector(0, 0, 150.0f); // Eye height
    FVector End = Player->GetActorLocation() + FVector(0, 0, 90.0f);   // Player chest

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    return !bBlocked;
}

float UTRexBehaviorComponent::DistanceToPlayer(AActor* Player) const
{
    if (!Player || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

void UTRexBehaviorComponent::ForceState(ENPC_TRexState NewState)
{
    SetState(NewState);
}
