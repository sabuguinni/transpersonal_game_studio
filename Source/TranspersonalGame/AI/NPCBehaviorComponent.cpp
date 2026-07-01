// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of dinosaur AI state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"

// ============================================================
// Constructor
// ============================================================
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI

    // Default species: T-Rex
    Species = ENPC_DinoSpecies::TRex;
    CurrentState = ENPC_BehaviorState::Idle;
    PreviousState = ENPC_BehaviorState::Idle;

    // Detection ranges
    SightRange = 3000.0f;
    AttackRange = 300.0f;
    HearingRange = 1500.0f;
    SmellRange = 800.0f;

    // Movement speeds
    PatrolSpeed = 250.0f;
    ChaseSpeed = 650.0f;
    FleeSpeed = 700.0f;

    // Combat
    AttackDamage = 75.0f;
    AttackCooldown = 2.0f;
    bCanAttack = true;

    // Survival stats
    Hunger = 50.0f;
    MaxHunger = 100.0f;
    HungerDecayRate = 0.5f; // per second

    // Patrol
    PatrolRadius = 5000.0f;
    CurrentWaypointIndex = 0;
    WaypointTolerance = 200.0f;

    // Memory
    LastKnownPlayerLocation = FVector::ZeroVector;
    bPlayerSpotted = false;
    TimeSincePlayerSeen = 0.0f;
    MemoryDuration = 30.0f;

    // Pack behavior
    bIsPackLeader = false;
    PackCommunicationRange = 2000.0f;
}

// ============================================================
// BeginPlay
// ============================================================
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific configuration
    ApplySpeciesConfig();

    // Start idle timer — transition to patrol after brief delay
    GetWorld()->GetTimerManager().SetTimer(
        IdleTimerHandle,
        this,
        &UNPCBehaviorComponent::OnIdleComplete,
        FMath::RandRange(2.0f, 5.0f),
        false
    );

    // Start hunger decay timer
    GetWorld()->GetTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &UNPCBehaviorComponent::UpdateHunger,
        1.0f,
        true
    );
}

// ============================================================
// TickComponent
// ============================================================
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update memory decay
    if (bPlayerSpotted)
    {
        TimeSincePlayerSeen += DeltaTime;
        if (TimeSincePlayerSeen > MemoryDuration)
        {
            bPlayerSpotted = false;
            TimeSincePlayerSeen = 0.0f;
            OnPlayerLost();
        }
    }

    // Run current state logic
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            TickAlert(DeltaTime);
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
        case ENPC_BehaviorState::Feed:
            TickFeed(DeltaTime);
            break;
        case ENPC_BehaviorState::Rest:
            TickRest(DeltaTime);
            break;
        case ENPC_BehaviorState::Dead:
            // No tick logic for dead state
            break;
    }

    // Sense player every tick
    SensePlayer();
}

// ============================================================
// State Machine — Transitions
// ============================================================
void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return; // Dead is terminal

    PreviousState = CurrentState;
    CurrentState = NewState;

    OnStateChanged(PreviousState, NewState);
}

void UNPCBehaviorComponent::OnStateChanged(ENPC_BehaviorState OldState, ENPC_BehaviorState NewState)
{
    // Apply movement speed based on new state
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
    if (OwnerChar && OwnerChar->GetCharacterMovement())
    {
        switch (NewState)
        {
            case ENPC_BehaviorState::Patrol:
                OwnerChar->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
                break;
            case ENPC_BehaviorState::Chase:
            case ENPC_BehaviorState::Alert:
                OwnerChar->GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
                break;
            case ENPC_BehaviorState::Flee:
                OwnerChar->GetCharacterMovement()->MaxWalkSpeed = FleeSpeed;
                break;
            case ENPC_BehaviorState::Idle:
            case ENPC_BehaviorState::Feed:
            case ENPC_BehaviorState::Rest:
                OwnerChar->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// State Tick Implementations
// ============================================================
void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // Idle: stand still, look around
    // Transition to patrol handled by timer (OnIdleComplete)
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIC) return;

    FVector TargetWaypoint = PatrolWaypoints[CurrentWaypointIndex];
    float DistToWaypoint = FVector::Dist(OwnerPawn->GetActorLocation(), TargetWaypoint);

    if (DistToWaypoint < WaypointTolerance)
    {
        // Reached waypoint — advance to next
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();

        // Occasionally pause at waypoint
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            TransitionToState(ENPC_BehaviorState::Idle);
            GetWorld()->GetTimerManager().SetTimer(
                IdleTimerHandle,
                this,
                &UNPCBehaviorComponent::OnIdleComplete,
                FMath::RandRange(3.0f, 8.0f),
                false
            );
        }
    }
    else
    {
        AIC->MoveToLocation(TargetWaypoint, WaypointTolerance);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert: stop, look toward last known player location
    // After brief delay, transition to Chase
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    FVector ToPlayer = (LastKnownPlayerLocation - OwnerPawn->GetActorLocation()).GetSafeNormal();
    FRotator LookRot = ToPlayer.Rotation();
    OwnerPawn->SetActorRotation(FMath::RInterpTo(OwnerPawn->GetActorRotation(), LookRot, DeltaTime, 3.0f));

    // Transition to chase after 1.5s of alert (handled by timer set in SensePlayer)
}

void UNPCBehaviorComponent::TickChase(float DeltaTime)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIC) return;

    if (bPlayerSpotted)
    {
        float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownPlayerLocation);

        if (DistToPlayer <= AttackRange)
        {
            TransitionToState(ENPC_BehaviorState::Attack);
        }
        else
        {
            AIC->MoveToLocation(LastKnownPlayerLocation, AttackRange * 0.8f);
        }
    }
    else
    {
        // Lost player — go to last known location then return to patrol
        AIC->MoveToLocation(LastKnownPlayerLocation, 100.0f);
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickAttack(float DeltaTime)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownPlayerLocation);

    if (DistToPlayer > AttackRange * 1.5f)
    {
        // Player escaped attack range — resume chase
        TransitionToState(ENPC_BehaviorState::Chase);
        return;
    }

    if (bCanAttack)
    {
        ExecuteAttack();
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIC) return;

    // Move away from player
    FVector AwayFromPlayer = (OwnerPawn->GetActorLocation() - LastKnownPlayerLocation).GetSafeNormal();
    FVector FleeTarget = OwnerPawn->GetActorLocation() + AwayFromPlayer * 3000.0f;
    AIC->MoveToLocation(FleeTarget, 100.0f);

    // After fleeing far enough, return to patrol
    float DistToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), LastKnownPlayerLocation);
    if (DistToPlayer > SightRange * 2.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFeed(float DeltaTime)
{
    // Feeding: restore hunger
    Hunger = FMath::Min(Hunger + 5.0f * DeltaTime, MaxHunger);
    if (Hunger >= MaxHunger * 0.9f)
    {
        TransitionToState(ENPC_BehaviorState::Rest);
    }
}

void UNPCBehaviorComponent::TickRest(float DeltaTime)
{
    // Resting: fully idle, very low awareness
    // After rest duration, resume patrol
}

// ============================================================
// Sensing
// ============================================================
void UNPCBehaviorComponent::SensePlayer()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector OwnerLoc = OwnerPawn->GetActorLocation();
    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    float DistToPlayer = FVector::Dist(OwnerLoc, PlayerLoc);

    bool bCanSeePlayer = false;

    // Sight check
    if (DistToPlayer <= SightRange)
    {
        FVector ToPlayer = (PlayerLoc - OwnerLoc).GetSafeNormal();
        FVector OwnerForward = OwnerPawn->GetActorForwardVector();
        float DotProduct = FVector::DotProduct(OwnerForward, ToPlayer);

        // 120-degree field of view (dot > -0.5)
        if (DotProduct > -0.5f)
        {
            // Line of sight check
            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(OwnerPawn);
            bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                OwnerLoc + FVector(0, 0, 100),
                PlayerLoc + FVector(0, 0, 100),
                ECC_Visibility,
                Params
            );

            if (!bBlocked || HitResult.GetActor() == PlayerPawn)
            {
                bCanSeePlayer = true;
            }
        }
    }

    // Hearing check (closer range, no LoS required)
    if (!bCanSeePlayer && DistToPlayer <= HearingRange)
    {
        // Simplified: always hear within range (could add noise level later)
        bCanSeePlayer = true;
    }

    if (bCanSeePlayer)
    {
        LastKnownPlayerLocation = PlayerLoc;
        TimeSincePlayerSeen = 0.0f;

        if (!bPlayerSpotted)
        {
            bPlayerSpotted = true;
            OnPlayerDetected();
        }
    }
}

void UNPCBehaviorComponent::OnPlayerDetected()
{
    // Notify pack members if leader
    if (bIsPackLeader)
    {
        AlertPackMembers();
    }

    // Transition based on current state
    if (CurrentState == ENPC_BehaviorState::Idle ||
        CurrentState == ENPC_BehaviorState::Patrol ||
        CurrentState == ENPC_BehaviorState::Rest ||
        CurrentState == ENPC_BehaviorState::Feed)
    {
        TransitionToState(ENPC_BehaviorState::Alert);

        // After alert delay, chase
        GetWorld()->GetTimerManager().SetTimer(
            AlertTimerHandle,
            [this]()
            {
                if (bPlayerSpotted)
                {
                    TransitionToState(ENPC_BehaviorState::Chase);
                }
            },
            1.5f,
            false
        );
    }
}

void UNPCBehaviorComponent::OnPlayerLost()
{
    if (CurrentState == ENPC_BehaviorState::Chase ||
        CurrentState == ENPC_BehaviorState::Alert)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

// ============================================================
// Combat
// ============================================================
void UNPCBehaviorComponent::ExecuteAttack()
{
    if (!bCanAttack) return;

    bCanAttack = false;

    // Apply damage to player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        APawn* PlayerPawn = PC->GetPawn();
        float DistToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());

        if (DistToPlayer <= AttackRange)
        {
            UGameplayStatics::ApplyDamage(
                PlayerPawn,
                AttackDamage,
                Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr,
                GetOwner(),
                nullptr
            );
        }
    }

    // Reset attack cooldown
    GetWorld()->GetTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

void UNPCBehaviorComponent::TakeDamageReaction(float DamageAmount)
{
    // Herbivores flee when damaged
    if (Species == ENPC_DinoSpecies::Brachiosaurus)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
        return;
    }

    // Carnivores become more aggressive when damaged (unless low health)
    if (Species == ENPC_DinoSpecies::TRex || Species == ENPC_DinoSpecies::Raptor)
    {
        if (CurrentState != ENPC_BehaviorState::Attack)
        {
            TransitionToState(ENPC_BehaviorState::Chase);
        }
    }
}

void UNPCBehaviorComponent::Die()
{
    TransitionToState(ENPC_BehaviorState::Dead);

    // Clear all timers
    GetWorld()->GetTimerManager().ClearTimer(IdleTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(HungerTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AlertTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AttackCooldownHandle);

    // Disable tick
    PrimaryComponentTick.bCanEverTick = false;
}

// ============================================================
// Pack Behavior
// ============================================================
void UNPCBehaviorComponent::AlertPackMembers()
{
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > PackCommunicationRange) continue;

        UNPCBehaviorComponent* PackMember = Actor->FindComponentByClass<UNPCBehaviorComponent>();
        if (PackMember && PackMember->Species == Species)
        {
            PackMember->LastKnownPlayerLocation = LastKnownPlayerLocation;
            PackMember->bPlayerSpotted = true;
            PackMember->TransitionToState(ENPC_BehaviorState::Chase);
        }
    }
}

// ============================================================
// Survival
// ============================================================
void UNPCBehaviorComponent::UpdateHunger()
{
    Hunger = FMath::Max(0.0f, Hunger - HungerDecayRate);

    if (Hunger <= 10.0f && CurrentState == ENPC_BehaviorState::Patrol)
    {
        // Very hungry — seek food (simplified: transition to feed state near current location)
        TransitionToState(ENPC_BehaviorState::Feed);
    }
}

// ============================================================
// Species Configuration
// ============================================================
void UNPCBehaviorComponent::ApplySpeciesConfig()
{
    switch (Species)
    {
        case ENPC_DinoSpecies::TRex:
            SightRange = 4000.0f;
            AttackRange = 350.0f;
            HearingRange = 2000.0f;
            SmellRange = 1200.0f;
            PatrolSpeed = 300.0f;
            ChaseSpeed = 700.0f;
            AttackDamage = 120.0f;
            AttackCooldown = 3.0f;
            PatrolRadius = 6000.0f;
            MemoryDuration = 45.0f;
            break;

        case ENPC_DinoSpecies::Raptor:
            SightRange = 2500.0f;
            AttackRange = 200.0f;
            HearingRange = 1500.0f;
            SmellRange = 800.0f;
            PatrolSpeed = 350.0f;
            ChaseSpeed = 900.0f;
            AttackDamage = 45.0f;
            AttackCooldown = 1.0f;
            PatrolRadius = 3000.0f;
            MemoryDuration = 60.0f;
            bIsPackLeader = true; // First raptor is pack leader
            PackCommunicationRange = 1500.0f;
            break;

        case ENPC_DinoSpecies::Brachiosaurus:
            SightRange = 2000.0f;
            AttackRange = 500.0f; // Tail swipe range
            HearingRange = 1000.0f;
            SmellRange = 600.0f;
            PatrolSpeed = 200.0f;
            ChaseSpeed = 400.0f;
            FleeSpeed = 500.0f;
            AttackDamage = 80.0f;
            AttackCooldown = 4.0f;
            PatrolRadius = 4000.0f;
            MemoryDuration = 20.0f;
            break;

        case ENPC_DinoSpecies::Triceratops:
            SightRange = 1800.0f;
            AttackRange = 280.0f;
            HearingRange = 1200.0f;
            SmellRange = 700.0f;
            PatrolSpeed = 220.0f;
            ChaseSpeed = 550.0f;
            AttackDamage = 90.0f;
            AttackCooldown = 2.5f;
            PatrolRadius = 3500.0f;
            MemoryDuration = 25.0f;
            break;

        case ENPC_DinoSpecies::Pterodactyl:
            SightRange = 5000.0f; // Aerial advantage
            AttackRange = 150.0f;
            HearingRange = 800.0f;
            SmellRange = 400.0f;
            PatrolSpeed = 500.0f;
            ChaseSpeed = 1200.0f;
            FleeSpeed = 1400.0f;
            AttackDamage = 30.0f;
            AttackCooldown = 1.5f;
            PatrolRadius = 8000.0f;
            MemoryDuration = 15.0f;
            break;
    }
}

// ============================================================
// Patrol Waypoint Management
// ============================================================
void UNPCBehaviorComponent::SetPatrolWaypoints(const TArray<FVector>& Waypoints)
{
    PatrolWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
}

void UNPCBehaviorComponent::GenerateRandomPatrolWaypoints(FVector Center, float Radius, int32 NumWaypoints)
{
    PatrolWaypoints.Empty();

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    for (int32 i = 0; i < NumWaypoints; ++i)
    {
        FNavLocation NavLoc;
        FVector RandomOffset = FMath::VRand() * Radius;
        RandomOffset.Z = 0.0f;
        FVector TestPoint = Center + RandomOffset;

        if (NavSys && NavSys->GetRandomReachablePointInRadius(TestPoint, Radius * 0.5f, NavLoc))
        {
            PatrolWaypoints.Add(NavLoc.Location);
        }
        else
        {
            PatrolWaypoints.Add(TestPoint);
        }
    }

    CurrentWaypointIndex = 0;
}

// ============================================================
// Timer Callbacks
// ============================================================
void UNPCBehaviorComponent::OnIdleComplete()
{
    if (CurrentState == ENPC_BehaviorState::Idle)
    {
        if (PatrolWaypoints.Num() > 0)
        {
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
        else
        {
            // Generate random patrol if no waypoints set
            GenerateRandomPatrolWaypoints(GetOwner()->GetActorLocation(), PatrolRadius, 5);
            TransitionToState(ENPC_BehaviorState::Patrol);
        }
    }
}
