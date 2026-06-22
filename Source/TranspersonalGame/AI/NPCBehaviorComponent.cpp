// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_009
// Full implementation of modular NPC behavior state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
    CurrentHealth = MaxHealth;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location from owner spawn position
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Initialize health
    CurrentHealth = MaxHealth;

    // Generate patrol waypoints around home
    GeneratePatrolWaypoints();

    // Start patrolling
    CurrentState = ENPC_BehaviorState::Patrolling;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive())
    {
        return;
    }

    // Decay memory over time
    UpdateMemoryDecay(DeltaTime);
    TimeSinceLastAttack += DeltaTime;

    // State machine dispatch
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
        case ENPC_BehaviorState::Patrolling:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Alerted:
            TickAlert(DeltaTime);
            break;
        case ENPC_BehaviorState::Chasing:
            TickChase(DeltaTime);
            break;
        case ENPC_BehaviorState::Attacking:
            TickAttack(DeltaTime);
            break;
        case ENPC_BehaviorState::Fleeing:
            TickFlee(DeltaTime);
            break;
        default:
            break;
    }
}

// ─── State Machine ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    ENPC_BehaviorState OldState = CurrentState;
    CurrentState = NewState;
    OnStateChanged.Broadcast(OldState, NewState);
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    // Check if player is within detection radius — transition to Alerted
    float DistToPlayer = GetDistanceToPlayer();
    if (DistToPlayer > 0.0f && DistToPlayer <= CombatConfig.DetectionRadius)
    {
        AActor* Player = FindPlayerActor();
        if (Player && CanSeePlayer(Player))
        {
            UpdatePlayerSighting(Player->GetActorLocation());
            SetBehaviorState(ENPC_BehaviorState::Alerted);
            return;
        }
    }

    // Patrol logic — move toward current waypoint
    if (PatrolWaypoints.Num() == 0)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector TargetWaypoint = PatrolWaypoints[CurrentWaypointIndex];
    float DistToWaypoint = FVector::Dist(Owner->GetActorLocation(), TargetWaypoint);

    if (DistToWaypoint <= PatrolConfig.WaypointAcceptanceRadius)
    {
        // Reached waypoint — idle briefly then advance
        IdleTimer += DeltaTime;
        if (IdleTimer >= PatrolConfig.IdleWaitTime)
        {
            IdleTimer = 0.0f;
            AdvancePatrolWaypoint();
        }
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
    else
    {
        // Move toward waypoint via character movement
        ACharacter* OwnerChar = Cast<ACharacter>(Owner);
        if (OwnerChar)
        {
            FVector Direction = (TargetWaypoint - Owner->GetActorLocation()).GetSafeNormal();
            OwnerChar->AddMovementInput(Direction, 1.0f);
        }
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    if (DistToPlayer <= 0.0f || DistToPlayer > CombatConfig.DetectionRadius * 1.5f)
    {
        // Lost the player — return to patrol
        ClearPlayerMemory();
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        return;
    }

    if (DistToPlayer <= CombatConfig.AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attacking);
        return;
    }

    // Within detection range — chase
    SetBehaviorState(ENPC_BehaviorState::Chasing);
}

void UNPCBehaviorComponent::TickChase(float DeltaTime)
{
    AActor* Player = FindPlayerActor();
    AActor* Owner = GetOwner();

    if (!Player || !Owner)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());

    // Check flee condition — low health
    float HealthRatio = CurrentHealth / FMath::Max(MaxHealth, 1.0f);
    if (HealthRatio <= CombatConfig.FleeHealthThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        return;
    }

    // Lost player beyond detection range
    if (DistToPlayer > CombatConfig.DetectionRadius * 2.0f)
    {
        ClearPlayerMemory();
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        return;
    }

    // Within attack range
    if (DistToPlayer <= CombatConfig.AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attacking);
        return;
    }

    // Chase — move toward player
    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (OwnerChar)
    {
        FVector Direction = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
        OwnerChar->AddMovementInput(Direction, 1.0f);

        // Update movement speed for chase
        UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement();
        if (MovComp)
        {
            MovComp->MaxWalkSpeed = CombatConfig.ChaseSpeed;
        }
    }

    // Update memory with current player location
    UpdatePlayerSighting(Player->GetActorLocation());
}

void UNPCBehaviorComponent::TickAttack(float DeltaTime)
{
    AActor* Player = FindPlayerActor();
    AActor* Owner = GetOwner();

    if (!Player || !Owner)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());

    // Player escaped attack range — chase again
    if (DistToPlayer > CombatConfig.AttackRadius * 1.5f)
    {
        SetBehaviorState(ENPC_BehaviorState::Chasing);
        return;
    }

    // Attack cooldown check
    if (TimeSinceLastAttack >= CombatConfig.AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;

        // Apply damage to player via generic damage system
        UGameplayStatics::ApplyDamage(Player, CombatConfig.AttackDamage,
                                       nullptr, Owner, nullptr);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Player = FindPlayerActor();
    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return;
    }

    // Flee away from player toward home
    FVector FleeDirection = FVector::ZeroVector;

    if (Player)
    {
        FVector AwayFromPlayer = (Owner->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
        FVector TowardHome = (HomeLocation - Owner->GetActorLocation()).GetSafeNormal();
        FleeDirection = (AwayFromPlayer + TowardHome * 0.5f).GetSafeNormal();
    }
    else
    {
        FleeDirection = (HomeLocation - Owner->GetActorLocation()).GetSafeNormal();
    }

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (OwnerChar)
    {
        OwnerChar->AddMovementInput(FleeDirection, 1.0f);
        UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement();
        if (MovComp)
        {
            MovComp->MaxWalkSpeed = CombatConfig.ChaseSpeed * 0.8f;
        }
    }

    // Recover health slightly while fleeing (regeneration)
    CurrentHealth = FMath::Min(CurrentHealth + DeltaTime * 5.0f, MaxHealth);

    // If health recovered enough, return to patrol
    float HealthRatio = CurrentHealth / FMath::Max(MaxHealth, 1.0f);
    if (HealthRatio > CombatConfig.FleeHealthThreshold * 2.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

// ─── Memory System ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (Memory.bHasSeenPlayer)
    {
        Memory.TimeSinceLastSighting += DeltaTime;

        // Forget player after 30 seconds
        if (Memory.TimeSinceLastSighting > 30.0f)
        {
            ClearPlayerMemory();
        }
        else
        {
            // Threat level decays over time
            Memory.ThreatLevel = FMath::Max(0.0f,
                Memory.ThreatLevel - DeltaTime * 0.02f);
        }
    }
}

void UNPCBehaviorComponent::UpdatePlayerSighting(const FVector& PlayerLocation)
{
    Memory.LastKnownPlayerLocation = PlayerLocation;
    Memory.TimeSinceLastSighting = 0.0f;
    Memory.bHasSeenPlayer = true;
    Memory.ThreatLevel = FMath::Min(1.0f, Memory.ThreatLevel + 0.1f);
}

void UNPCBehaviorComponent::ClearPlayerMemory()
{
    Memory.bHasSeenPlayer = false;
    Memory.TimeSinceLastSighting = 0.0f;
    Memory.ThreatLevel = 0.0f;
    Memory.LastKnownPlayerLocation = FVector::ZeroVector;
}

// ─── Patrol Waypoints ─────────────────────────────────────────────────────────

void UNPCBehaviorComponent::GeneratePatrolWaypoints()
{
    PatrolWaypoints.Empty();

    int32 N = FMath::Max(PatrolConfig.NumWaypoints, 3);
    float Radius = PatrolConfig.PatrolRadius;

    for (int32 i = 0; i < N; ++i)
    {
        float Angle = (2.0f * PI / N) * i;
        float X = HomeLocation.X + Radius * FMath::Cos(Angle);
        float Y = HomeLocation.Y + Radius * FMath::Sin(Angle);
        // Add slight random variation to avoid perfect circles
        X += FMath::RandRange(-500.0f, 500.0f);
        Y += FMath::RandRange(-500.0f, 500.0f);
        PatrolWaypoints.Add(FVector(X, Y, HomeLocation.Z));
    }

    CurrentWaypointIndex = 0;
    bPatrolWaypointsGenerated = true;
}

FVector UNPCBehaviorComponent::GetNextPatrolWaypoint() const
{
    if (PatrolWaypoints.Num() == 0)
    {
        return HomeLocation;
    }
    return PatrolWaypoints[CurrentWaypointIndex % PatrolWaypoints.Num()];
}

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolWaypoints.Num() > 0)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    }
}

// ─── Combat ───────────────────────────────────────────────────────────────────

float UNPCBehaviorComponent::ApplyDamage(float DamageAmount)
{
    if (!IsAlive())
    {
        return 0.0f;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    // Raise threat level when hit
    Memory.ThreatLevel = FMath::Min(1.0f, Memory.ThreatLevel + 0.3f);

    if (!IsAlive())
    {
        SetBehaviorState(ENPC_BehaviorState::Dead);
        OnNPCDeath.Broadcast(GetOwner());
    }
    else if (CurrentHealth / FMath::Max(MaxHealth, 1.0f) <= CombatConfig.FleeHealthThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
    else if (CurrentState == ENPC_BehaviorState::Patrolling ||
             CurrentState == ENPC_BehaviorState::Idle)
    {
        // Getting hit triggers alert even without seeing player
        SetBehaviorState(ENPC_BehaviorState::Alerted);
    }

    return CurrentHealth;
}

bool UNPCBehaviorComponent::IsAlive() const
{
    return CurrentHealth > 0.0f && CurrentState != ENPC_BehaviorState::Dead;
}

bool UNPCBehaviorComponent::IsHostile() const
{
    return Species == ENPC_DinoSpecies::TRex ||
           Species == ENPC_DinoSpecies::Raptor ||
           Species == ENPC_DinoSpecies::Triceratops;
}

// ─── Perception ───────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::CanSeePlayer(AActor* Player) const
{
    AActor* Owner = GetOwner();
    if (!Owner || !Player)
    {
        return false;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return false;
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bBlocked = World->LineTraceSingleByChannel(
        HitResult,
        Owner->GetActorLocation() + FVector(0, 0, 100.0f),
        Player->GetActorLocation() + FVector(0, 0, 100.0f),
        ECC_Visibility,
        Params
    );

    // Can see player if line trace is not blocked (or hits the player)
    return !bBlocked || (HitResult.GetActor() == Player);
}

float UNPCBehaviorComponent::GetDistanceToPlayer() const
{
    AActor* Player = FindPlayerActor();
    AActor* Owner = GetOwner();

    if (!Player || !Owner)
    {
        return -1.0f;
    }

    return FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
}

AActor* UNPCBehaviorComponent::FindPlayerActor() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }

    return PC->GetPawn();
}
