// TRexBehavior.cpp
// NPC Behavior Agent #11 — T-Rex AI full implementation
// Patrol radius: 5000 units | Chase range: 3000 units | Attack range: 300 units

#include "TRexBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_TRexState::Idle;
    TimeSinceLastRoar = 0.0f;
    AttackCooldown = 0.0f;
    StateTimer = 0.0f;
    PatrolAnchor = FVector::ZeroVector;
    CurrentWaypoint = FVector::ZeroVector;
    PlayerActor = nullptr;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Record patrol anchor at spawn location
    if (AActor* Owner = GetOwner())
    {
        PatrolAnchor = Owner->GetActorLocation();
        CurrentWaypoint = PatrolAnchor;
    }

    // Cache player reference
    PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    // Start patrolling
    SetState(ENPC_TRexState::Patrol);
    PickNewPatrolWaypoint();
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update timers
    TimeSinceLastRoar += DeltaTime;
    if (AttackCooldown > 0.0f) AttackCooldown -= DeltaTime;
    StateTimer += DeltaTime;

    // Refresh player reference if lost
    if (!PlayerActor)
    {
        PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        return;
    }

    // State machine transitions
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            if (StateTimer > 2.0f)
            {
                SetState(ENPC_TRexState::Patrol);
                PickNewPatrolWaypoint();
            }
            break;

        case ENPC_TRexState::Patrol:
            UpdatePatrol(DeltaTime);
            // Transition to Chase if player enters chase range
            if (IsPlayerInChaseRange())
            {
                TriggerRoar();
                SetState(ENPC_TRexState::Chase);
            }
            break;

        case ENPC_TRexState::Investigate:
            // Move toward last known player position
            if (IsPlayerInChaseRange())
            {
                SetState(ENPC_TRexState::Chase);
            }
            else if (StateTimer > 8.0f)
            {
                SetState(ENPC_TRexState::Patrol);
                PickNewPatrolWaypoint();
            }
            break;

        case ENPC_TRexState::Chase:
            UpdateChase(DeltaTime);
            // Transition to Attack if close enough
            if (IsPlayerInAttackRange())
            {
                SetState(ENPC_TRexState::Attack);
            }
            // Give up chase if player escapes beyond 1.5x chase range
            else if (GetDistanceToPlayer() > Config.ChaseRange * 1.5f)
            {
                SetState(ENPC_TRexState::Investigate);
            }
            break;

        case ENPC_TRexState::Attack:
            UpdateAttack(DeltaTime);
            // Return to Chase if player moves out of attack range
            if (!IsPlayerInAttackRange())
            {
                SetState(ENPC_TRexState::Chase);
            }
            break;

        case ENPC_TRexState::Roar:
            // Roar lasts 2 seconds then transitions to Chase
            if (StateTimer > 2.0f)
            {
                SetState(ENPC_TRexState::Chase);
            }
            break;

        default:
            break;
    }
}

void UTRexBehaviorComponent::SetState(ENPC_TRexState NewState)
{
    CurrentState = NewState;
    StateTimer = 0.0f;

    // Adjust movement speed based on state
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* TRexChar = Cast<ACharacter>(Owner);
    if (!TRexChar) return;

    UCharacterMovementComponent* MoveComp = TRexChar->GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewState)
    {
        case ENPC_TRexState::Patrol:
        case ENPC_TRexState::Investigate:
            MoveComp->MaxWalkSpeed = Config.PatrolSpeed;
            break;
        case ENPC_TRexState::Chase:
        case ENPC_TRexState::Attack:
            MoveComp->MaxWalkSpeed = Config.ChaseSpeed;
            break;
        case ENPC_TRexState::Idle:
        case ENPC_TRexState::Roar:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            break;
    }
}

float UTRexBehaviorComponent::GetDistanceToPlayer() const
{
    if (!PlayerActor || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
}

bool UTRexBehaviorComponent::IsPlayerInChaseRange() const
{
    return GetDistanceToPlayer() <= Config.ChaseRange;
}

bool UTRexBehaviorComponent::IsPlayerInAttackRange() const
{
    return GetDistanceToPlayer() <= Config.AttackRange;
}

void UTRexBehaviorComponent::PerformAttack()
{
    if (AttackCooldown > 0.0f) return;
    if (!PlayerActor) return;

    // Apply damage to player
    UGameplayStatics::ApplyDamage(
        PlayerActor,
        Config.AttackDamage,
        nullptr,
        GetOwner(),
        UDamageType::StaticClass()
    );

    AttackCooldown = 2.0f; // 2 second attack cooldown
    UE_LOG(LogTemp, Warning, TEXT("TRex attacked player for %.0f damage!"), Config.AttackDamage);
}

void UTRexBehaviorComponent::TriggerRoar()
{
    if (TimeSinceLastRoar < Config.RoarCooldown) return;
    TimeSinceLastRoar = 0.0f;
    SetState(ENPC_TRexState::Roar);
    UE_LOG(LogTemp, Warning, TEXT("TRex ROARS!"));
}

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Move toward current waypoint
    FVector OwnerLoc = Owner->GetActorLocation();
    FVector Direction = (CurrentWaypoint - OwnerLoc).GetSafeNormal();
    Owner->SetActorLocation(OwnerLoc + Direction * Config.PatrolSpeed * DeltaTime, true);

    // Pick new waypoint when reached
    if (HasReachedWaypoint())
    {
        PickNewPatrolWaypoint();
    }
}

void UTRexBehaviorComponent::UpdateChase(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !PlayerActor) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector PlayerLoc = PlayerActor->GetActorLocation();
    FVector Direction = (PlayerLoc - OwnerLoc).GetSafeNormal();
    Owner->SetActorLocation(OwnerLoc + Direction * Config.ChaseSpeed * DeltaTime, true);

    // Face the player
    FRotator LookAt = Direction.Rotation();
    Owner->SetActorRotation(FRotator(0.0f, LookAt.Yaw, 0.0f));
}

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime)
{
    // Perform attack on cooldown
    PerformAttack();
}

void UTRexBehaviorComponent::PickNewPatrolWaypoint()
{
    // Random point within patrol radius from anchor
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(Config.PatrolWaypointRadius, Config.PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentWaypoint = PatrolAnchor + FVector(
        FMath::Cos(RadAngle) * Radius,
        FMath::Sin(RadAngle) * Radius,
        0.0f
    );
}

bool UTRexBehaviorComponent::HasReachedWaypoint() const
{
    if (!GetOwner()) return false;
    float Dist = FVector::Dist2D(GetOwner()->GetActorLocation(), CurrentWaypoint);
    return Dist < 200.0f;
}
