// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Full dinosaur AI state machine implementation
// 8 states: Idle, Patrol, Investigate, Chase, Attack, Flee, Graze, Sleep

#include "NPCBehaviorComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz AI tick for performance

    // Default config — T-Rex preset
    BehaviorConfig.PatrolRadius = 5000.0f;
    BehaviorConfig.DetectionRange = 3000.0f;
    BehaviorConfig.AttackRange = 300.0f;
    BehaviorConfig.ChaseSpeed = 1200.0f;
    BehaviorConfig.PatrolSpeed = 400.0f;
    BehaviorConfig.AttackDamage = 75.0f;
    BehaviorConfig.bIsPackHunter = false;
    BehaviorConfig.FleeHealthThreshold = 0.15f;
    BehaviorConfig.Species = ENPC_DinosaurSpecies::TRex;

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentHealth = 1000.0f;
    MaxHealth = 1000.0f;
    StateTimer = 0.0f;
    IdleDuration = 3.0f;

    bPlayerDetected = false;
    bIsAlerted = false;
    LastKnownPlayerLocation = FVector::ZeroVector;
    HomeLocation = FVector::ZeroVector;
    CurrentWaypointIndex = 0;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Generate initial patrol waypoints
    GeneratePatrolWaypoints();

    // Start in Idle state
    TransitionToState(ENPC_BehaviorState::Idle);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    // Scan for player every tick
    ScanForPlayer();

    // Execute current state logic
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Investigate:
            TickInvestigate(DeltaTime);
            break;
        case ENPC_BehaviorState::Chase:
            TickChase(DeltaTime);
            break;
        case ENPC_BehaviorState::Attack:
            TickAttack(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        case ENPC_BehaviorState::Graze:
            TickGraze(DeltaTime);
            break;
        case ENPC_BehaviorState::Sleep:
            TickSleep(DeltaTime);
            break;
    }
}

// ============================================================
// STATE MACHINE TRANSITIONS
// ============================================================

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    ENPC_BehaviorState OldState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;

    OnStateChanged.Broadcast(OldState, NewState);

    // State entry logic
    switch (NewState)
    {
        case ENPC_BehaviorState::Idle:
            IdleDuration = FMath::RandRange(2.0f, 5.0f);
            SetMovementSpeed(0.0f);
            break;

        case ENPC_BehaviorState::Patrol:
            SetMovementSpeed(BehaviorConfig.PatrolSpeed);
            MoveToNextWaypoint();
            break;

        case ENPC_BehaviorState::Investigate:
            SetMovementSpeed(BehaviorConfig.PatrolSpeed * 1.5f);
            bIsAlerted = true;
            break;

        case ENPC_BehaviorState::Chase:
            SetMovementSpeed(BehaviorConfig.ChaseSpeed);
            bIsAlerted = true;
            break;

        case ENPC_BehaviorState::Attack:
            SetMovementSpeed(0.0f);
            AttackCooldown = 0.0f;
            break;

        case ENPC_BehaviorState::Flee:
            SetMovementSpeed(BehaviorConfig.ChaseSpeed * 1.2f);
            break;

        case ENPC_BehaviorState::Graze:
            SetMovementSpeed(BehaviorConfig.PatrolSpeed * 0.3f);
            break;

        case ENPC_BehaviorState::Sleep:
            SetMovementSpeed(0.0f);
            break;
    }
}

// ============================================================
// IDLE STATE
// ============================================================

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // Check for player detection
    if (bPlayerDetected)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    // After idle duration, start patrolling
    if (StateTimer >= IdleDuration)
    {
        // Herbivores graze, carnivores patrol
        if (BehaviorConfig.Species == ENPC_DinosaurSpecies::Brachiosaurus ||
            BehaviorConfig.Species == ENPC_DinosaurSpecies::Triceratops)
        {
            TransitionToState(ENPC_BehaviorState::Graze);
        }
        else
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
    }
}

// ============================================================
// PATROL STATE
// ============================================================

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (bPlayerDetected)
    {
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Check if we reached current waypoint
    if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        FVector TargetWP = PatrolWaypoints[CurrentWaypointIndex];
        float DistToWP = FVector::Dist(Owner->GetActorLocation(), TargetWP);

        if (DistToWP < 200.0f)
        {
            // Reached waypoint — brief idle then next
            CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
            TransitionToState(ENPC_BehaviorState::Idle);
            return;
        }

        // Move toward waypoint
        MoveTowardTarget(TargetWP, DeltaTime);
    }
    else
    {
        GeneratePatrolWaypoints();
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ============================================================
// INVESTIGATE STATE
// ============================================================

void UNPCBehaviorComponent::TickInvestigate(float DeltaTime)
{
    if (bPlayerDetected)
    {
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToLastKnown = FVector::Dist(Owner->GetActorLocation(), LastKnownPlayerLocation);

    if (DistToLastKnown < 300.0f)
    {
        // Reached last known location — look around then return to patrol
        if (StateTimer > 4.0f)
        {
            bIsAlerted = false;
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
    }
    else
    {
        MoveTowardTarget(LastKnownPlayerLocation, DeltaTime);
    }

    // Timeout investigate after 10 seconds
    if (StateTimer > 10.0f)
    {
        bIsAlerted = false;
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

// ============================================================
// CHASE STATE
// ============================================================

void UNPCBehaviorComponent::TickChase(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    // Check flee condition (low health)
    if (CurrentHealth / MaxHealth <= BehaviorConfig.FleeHealthThreshold)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
        return;
    }

    // Attack if in range
    if (DistToPlayer <= BehaviorConfig.AttackRange)
    {
        TransitionToState(ENPC_BehaviorState::Attack);
        return;
    }

    // Lost player — investigate last known location
    if (!bPlayerDetected && StateTimer > 3.0f)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    // Update last known location and chase
    if (bPlayerDetected)
    {
        LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
    }

    MoveTowardTarget(LastKnownPlayerLocation, DeltaTime);
}

// ============================================================
// ATTACK STATE
// ============================================================

void UNPCBehaviorComponent::TickAttack(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    // Player escaped attack range
    if (DistToPlayer > BehaviorConfig.AttackRange * 1.5f)
    {
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    // Attack cooldown
    AttackCooldown += DeltaTime;
    float AttackInterval = (BehaviorConfig.Species == ENPC_DinosaurSpecies::TRex) ? 2.0f : 0.8f;

    if (AttackCooldown >= AttackInterval)
    {
        AttackCooldown = 0.0f;
        PerformAttack(PlayerPawn);
    }

    // Face the player
    FVector ToPlayer = (PlayerPawn->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    FRotator LookRot = ToPlayer.Rotation();
    Owner->SetActorRotation(FRotator(0.0f, LookRot.Yaw, 0.0f));
}

// ============================================================
// FLEE STATE
// ============================================================

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
        return;
    }

    // Move away from player
    FVector AwayFromPlayer = (Owner->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = Owner->GetActorLocation() + AwayFromPlayer * 5000.0f;

    MoveTowardTarget(FleeTarget, DeltaTime);

    // Stop fleeing if far enough
    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer > BehaviorConfig.DetectionRange * 2.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ============================================================
// GRAZE STATE (Herbivores)
// ============================================================

void UNPCBehaviorComponent::TickGraze(float DeltaTime)
{
    if (bPlayerDetected)
    {
        // Herbivores flee from player
        TransitionToState(ENPC_BehaviorState::Flee);
        return;
    }

    // Slowly wander while grazing
    if (StateTimer > 8.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ============================================================
// SLEEP STATE
// ============================================================

void UNPCBehaviorComponent::TickSleep(float DeltaTime)
{
    // Wake up if player very close
    if (bPlayerDetected)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
        return;
    }

    // Sleep for 10-20 seconds
    if (StateTimer > FMath::RandRange(10.0f, 20.0f))
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

void UNPCBehaviorComponent::ScanForPlayer()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        bPlayerDetected = false;
        return;
    }

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= BehaviorConfig.DetectionRange)
    {
        // Line of sight check
        FHitResult HitResult;
        FVector StartLoc = Owner->GetActorLocation() + FVector(0, 0, 100);
        FVector EndLoc = PlayerPawn->GetActorLocation() + FVector(0, 0, 100);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);

        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, QueryParams);

        if (!bHit || HitResult.GetActor() == PlayerPawn)
        {
            bPlayerDetected = true;
            LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
            StimulusMemory.LastKnownPlayerLocation = LastKnownPlayerLocation;
            StimulusMemory.TimeSinceLastSeen = 0.0f;
        }
        else
        {
            bPlayerDetected = false;
            StimulusMemory.TimeSinceLastSeen += PrimaryComponentTick.TickInterval;
        }
    }
    else
    {
        bPlayerDetected = false;
        StimulusMemory.TimeSinceLastSeen += PrimaryComponentTick.TickInterval;
    }
}

void UNPCBehaviorComponent::MoveTowardTarget(const FVector& Target, float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector Direction = (Target - Owner->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = Owner->GetActorLocation() + Direction * CurrentMoveSpeed * DeltaTime;

    Owner->SetActorLocation(NewLocation, true);

    // Face movement direction
    FRotator NewRot = Direction.Rotation();
    Owner->SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));
}

void UNPCBehaviorComponent::MoveToNextWaypoint()
{
    if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        // Already set, just move toward it
    }
    else
    {
        GeneratePatrolWaypoints();
        CurrentWaypointIndex = 0;
    }
}

void UNPCBehaviorComponent::GeneratePatrolWaypoints()
{
    PatrolWaypoints.Empty();

    int32 NumWaypoints = FMath::RandRange(3, 6);
    for (int32 i = 0; i < NumWaypoints; i++)
    {
        float Angle = (360.0f / NumWaypoints) * i + FMath::RandRange(-30.0f, 30.0f);
        float Radius = FMath::RandRange(BehaviorConfig.PatrolRadius * 0.3f, BehaviorConfig.PatrolRadius);
        float RadAngle = FMath::DegreesToRadians(Angle);

        FVector WP = HomeLocation + FVector(
            FMath::Cos(RadAngle) * Radius,
            FMath::Sin(RadAngle) * Radius,
            0.0f
        );
        PatrolWaypoints.Add(WP);
    }

    CurrentWaypointIndex = 0;
}

void UNPCBehaviorComponent::SetMovementSpeed(float Speed)
{
    CurrentMoveSpeed = Speed;

    // Also update CharacterMovementComponent if owner is a character
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetCharacterMovement())
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}

void UNPCBehaviorComponent::PerformAttack(AActor* Target)
{
    if (!Target) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        BehaviorConfig.AttackDamage,
        nullptr,
        GetOwner(),
        UDamageType::StaticClass()
    );

    OnAttackPerformed.Broadcast(Target);
}

void UNPCBehaviorComponent::ApplyDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        // Dinosaur dies
        OnDeath.Broadcast();
        TransitionToState(ENPC_BehaviorState::Flee); // Death handled by owner
    }
    else if (CurrentHealth / MaxHealth <= BehaviorConfig.FleeHealthThreshold)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
    }
    else if (CurrentState == ENPC_BehaviorState::Idle ||
             CurrentState == ENPC_BehaviorState::Patrol ||
             CurrentState == ENPC_BehaviorState::Graze ||
             CurrentState == ENPC_BehaviorState::Sleep)
    {
        // Attacked — become aggressive
        TransitionToState(ENPC_BehaviorState::Chase);
    }
}

void UNPCBehaviorComponent::SetSpeciesConfig(ENPC_DinosaurSpecies Species)
{
    BehaviorConfig.Species = Species;

    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            BehaviorConfig.PatrolRadius = 5000.0f;
            BehaviorConfig.DetectionRange = 3000.0f;
            BehaviorConfig.AttackRange = 350.0f;
            BehaviorConfig.ChaseSpeed = 1200.0f;
            BehaviorConfig.PatrolSpeed = 400.0f;
            BehaviorConfig.AttackDamage = 75.0f;
            BehaviorConfig.bIsPackHunter = false;
            BehaviorConfig.FleeHealthThreshold = 0.10f;
            MaxHealth = 1000.0f;
            break;

        case ENPC_DinosaurSpecies::Raptor:
            BehaviorConfig.PatrolRadius = 3000.0f;
            BehaviorConfig.DetectionRange = 2000.0f;
            BehaviorConfig.AttackRange = 200.0f;
            BehaviorConfig.ChaseSpeed = 1800.0f;
            BehaviorConfig.PatrolSpeed = 600.0f;
            BehaviorConfig.AttackDamage = 30.0f;
            BehaviorConfig.bIsPackHunter = true;
            BehaviorConfig.FleeHealthThreshold = 0.20f;
            MaxHealth = 300.0f;
            break;

        case ENPC_DinosaurSpecies::Triceratops:
            BehaviorConfig.PatrolRadius = 2000.0f;
            BehaviorConfig.DetectionRange = 1500.0f;
            BehaviorConfig.AttackRange = 400.0f;
            BehaviorConfig.ChaseSpeed = 900.0f;
            BehaviorConfig.PatrolSpeed = 300.0f;
            BehaviorConfig.AttackDamage = 50.0f;
            BehaviorConfig.bIsPackHunter = false;
            BehaviorConfig.FleeHealthThreshold = 0.15f;
            MaxHealth = 600.0f;
            break;

        case ENPC_DinosaurSpecies::Brachiosaurus:
            BehaviorConfig.PatrolRadius = 4000.0f;
            BehaviorConfig.DetectionRange = 1000.0f;
            BehaviorConfig.AttackRange = 500.0f;
            BehaviorConfig.ChaseSpeed = 600.0f;
            BehaviorConfig.PatrolSpeed = 200.0f;
            BehaviorConfig.AttackDamage = 40.0f;
            BehaviorConfig.bIsPackHunter = false;
            BehaviorConfig.FleeHealthThreshold = 0.05f;
            MaxHealth = 2000.0f;
            break;

        case ENPC_DinosaurSpecies::Pterodactyl:
            BehaviorConfig.PatrolRadius = 8000.0f;
            BehaviorConfig.DetectionRange = 4000.0f;
            BehaviorConfig.AttackRange = 250.0f;
            BehaviorConfig.ChaseSpeed = 2000.0f;
            BehaviorConfig.PatrolSpeed = 800.0f;
            BehaviorConfig.AttackDamage = 25.0f;
            BehaviorConfig.bIsPackHunter = false;
            BehaviorConfig.FleeHealthThreshold = 0.25f;
            MaxHealth = 200.0f;
            break;
    }

    CurrentHealth = MaxHealth;
}
