// TRexBehavior.cpp
// Agent #11 — NPC Behavior Agent
// T-Rex AI behavior component: patrol, investigate, chase, attack, roar states

#include "TRexBehavior.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTRexBehaviorComponent::UTRexBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz AI tick

    // Default config
    Config.PatrolRadius = 5000.0f;
    Config.ChaseRange = 3000.0f;
    Config.AttackRange = 300.0f;
    Config.PatrolSpeed = 300.0f;
    Config.ChaseSpeed = 800.0f;
    Config.AttackDamage = 150.0f;
    Config.RoarCooldown = 8.0f;
    Config.RoarRadius = 2000.0f;

    CurrentState = ENPC_TRexState::Idle;
    HomeLocation = FVector::ZeroVector;
    LastKnownPlayerLocation = FVector::ZeroVector;
    TimeSinceLastRoar = 0.0f;
    StateTimer = 0.0f;
    bIsInitialized = false;
}

void UTRexBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    APawn* Owner = Cast<APawn>(GetOwner());
    if (Owner)
    {
        HomeLocation = Owner->GetActorLocation();
        bIsInitialized = true;
        TransitionToState(ENPC_TRexState::Patrol);
    }
}

void UTRexBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized) return;

    TimeSinceLastRoar += DeltaTime;
    StateTimer += DeltaTime;

    UpdateBehavior(DeltaTime);
}

void UTRexBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    // Detect player every tick
    APawn* Player = DetectPlayer();

    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            UpdateIdle(DeltaTime, Player);
            break;
        case ENPC_TRexState::Patrol:
            UpdatePatrol(DeltaTime, Player);
            break;
        case ENPC_TRexState::Investigate:
            UpdateInvestigate(DeltaTime, Player);
            break;
        case ENPC_TRexState::Chase:
            UpdateChase(DeltaTime, Player);
            break;
        case ENPC_TRexState::Attack:
            UpdateAttack(DeltaTime, Player);
            break;
        case ENPC_TRexState::Roar:
            UpdateRoar(DeltaTime, Player);
            break;
        default:
            break;
    }
}

void UTRexBehaviorComponent::UpdateIdle(float DeltaTime, APawn* Player)
{
    // After 3 seconds idle, start patrolling
    if (StateTimer > 3.0f)
    {
        TransitionToState(ENPC_TRexState::Patrol);
        return;
    }

    // If player detected, investigate
    if (Player)
    {
        float Dist = GetDistanceToPlayer(Player);
        if (Dist < Config.ChaseRange)
        {
            LastKnownPlayerLocation = Player->GetActorLocation();
            TransitionToState(ENPC_TRexState::Investigate);
        }
    }
}

void UTRexBehaviorComponent::UpdatePatrol(float DeltaTime, APawn* Player)
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    // Check for player detection
    if (Player)
    {
        float Dist = GetDistanceToPlayer(Player);
        if (Dist < Config.ChaseRange)
        {
            LastKnownPlayerLocation = Player->GetActorLocation();
            // Roar before chasing if cooldown expired
            if (TimeSinceLastRoar > Config.RoarCooldown)
            {
                TransitionToState(ENPC_TRexState::Roar);
            }
            else
            {
                TransitionToState(ENPC_TRexState::Chase);
            }
            return;
        }
    }

    // Move toward current patrol waypoint
    FVector ToWaypoint = CurrentPatrolWaypoint - Owner->GetActorLocation();
    float DistToWaypoint = ToWaypoint.Size();

    if (DistToWaypoint < 200.0f || StateTimer > 15.0f)
    {
        // Pick new waypoint
        SelectNewPatrolWaypoint();
        StateTimer = 0.0f;
    }
    else
    {
        // Move toward waypoint
        FVector MoveDir = ToWaypoint.GetSafeNormal();
        Owner->AddMovementInput(MoveDir, 1.0f);
    }
}

void UTRexBehaviorComponent::UpdateInvestigate(float DeltaTime, APawn* Player)
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    // If player is visible and close, chase
    if (Player)
    {
        float Dist = GetDistanceToPlayer(Player);
        if (Dist < Config.ChaseRange)
        {
            LastKnownPlayerLocation = Player->GetActorLocation();
            TransitionToState(ENPC_TRexState::Chase);
            return;
        }
    }

    // Move toward last known location
    FVector ToTarget = LastKnownPlayerLocation - Owner->GetActorLocation();
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 300.0f || StateTimer > 10.0f)
    {
        // Lost the player, return to patrol
        TransitionToState(ENPC_TRexState::Patrol);
    }
    else
    {
        FVector MoveDir = ToTarget.GetSafeNormal();
        Owner->AddMovementInput(MoveDir, 0.8f);
    }
}

void UTRexBehaviorComponent::UpdateChase(float DeltaTime, APawn* Player)
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    if (!Player)
    {
        // Lost player, investigate last known location
        TransitionToState(ENPC_TRexState::Investigate);
        return;
    }

    float Dist = GetDistanceToPlayer(Player);
    LastKnownPlayerLocation = Player->GetActorLocation();

    // Attack if close enough
    if (Dist < Config.AttackRange)
    {
        TransitionToState(ENPC_TRexState::Attack);
        return;
    }

    // Chase player
    FVector ToPlayer = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    Owner->AddMovementInput(ToPlayer, 1.0f);

    // If player escaped beyond chase range + buffer
    if (Dist > Config.ChaseRange * 1.5f)
    {
        TransitionToState(ENPC_TRexState::Investigate);
    }
}

void UTRexBehaviorComponent::UpdateAttack(float DeltaTime, APawn* Player)
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    if (!Player)
    {
        TransitionToState(ENPC_TRexState::Patrol);
        return;
    }

    float Dist = GetDistanceToPlayer(Player);

    // If player moved away, chase again
    if (Dist > Config.AttackRange * 1.5f)
    {
        TransitionToState(ENPC_TRexState::Chase);
        return;
    }

    // Attack every 2 seconds
    if (StateTimer > 2.0f)
    {
        PerformAttack(Player);
        StateTimer = 0.0f;
    }
}

void UTRexBehaviorComponent::UpdateRoar(float DeltaTime, APawn* Player)
{
    // Roar lasts 2 seconds, then transition to chase
    if (StateTimer > 2.0f)
    {
        TimeSinceLastRoar = 0.0f;
        TransitionToState(ENPC_TRexState::Chase);
    }
}

void UTRexBehaviorComponent::TransitionToState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    // State entry actions
    switch (NewState)
    {
        case ENPC_TRexState::Patrol:
            SelectNewPatrolWaypoint();
            break;
        case ENPC_TRexState::Roar:
            // Broadcast roar to nearby dinosaurs
            BroadcastRoarAlert();
            break;
        default:
            break;
    }
}

APawn* UTRexBehaviorComponent::DetectPlayer() const
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner || !Owner->GetWorld()) return nullptr;

    APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    float Dist = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist > Config.ChaseRange * 1.2f) return nullptr;

    // Line of sight check (simplified — full version uses perception system)
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bBlocked = Owner->GetWorld()->LineTraceSingleByChannel(
        Hit,
        Owner->GetActorLocation() + FVector(0, 0, 100),
        PlayerPawn->GetActorLocation() + FVector(0, 0, 100),
        ECC_Visibility,
        Params
    );

    if (!bBlocked || Hit.GetActor() == PlayerPawn)
    {
        return PlayerPawn;
    }

    return nullptr;
}

float UTRexBehaviorComponent::GetDistanceToPlayer(APawn* Player) const
{
    if (!Player || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

void UTRexBehaviorComponent::SelectNewPatrolWaypoint()
{
    // Pick random point within patrol radius of home
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(Config.PatrolRadius * 0.3f, Config.PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolWaypoint = HomeLocation + FVector(
        FMath::Cos(RadAngle) * Radius,
        FMath::Sin(RadAngle) * Radius,
        0.0f
    );
}

void UTRexBehaviorComponent::PerformAttack(APawn* Target)
{
    if (!Target) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        Target,
        Config.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );
}

void UTRexBehaviorComponent::BroadcastRoarAlert()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner || !Owner->GetWorld()) return;

    // Find all pawns within roar radius and alert them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == Owner) continue;

        float Dist = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
        if (Dist < Config.RoarRadius)
        {
            // Alert nearby dinosaurs to player's last known location
            UTRexBehaviorComponent* OtherBehavior = Actor->FindComponentByClass<UTRexBehaviorComponent>();
            if (OtherBehavior && OtherBehavior->CurrentState == ENPC_TRexState::Patrol)
            {
                OtherBehavior->LastKnownPlayerLocation = LastKnownPlayerLocation;
                OtherBehavior->TransitionToState(ENPC_TRexState::Investigate);
            }
        }
    }
}

ENPC_TRexState UTRexBehaviorComponent::GetCurrentState() const
{
    return CurrentState;
}

FVector UTRexBehaviorComponent::GetLastKnownPlayerLocation() const
{
    return LastKnownPlayerLocation;
}
