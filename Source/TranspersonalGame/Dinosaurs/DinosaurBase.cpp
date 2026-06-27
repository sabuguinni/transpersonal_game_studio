// DinosaurBase.cpp
// Prehistoric survival game — base dinosaur pawn with patrol/idle/attack state machine
// Agent #03 — Core Systems Programmer — PROD_CYCLE_AUTO_20260627_009

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule sizing — default raptor-scale, override in subclasses
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Mesh — subclasses assign actual skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Defaults
    CurrentState = EDinoState::Idle;
    Health = MaxHealth;
    PatrolRadius = 1500.0f;
    PatrolSpeed = 200.0f;
    ChaseSpeed = 500.0f;
    AttackRange = 200.0f;
    AttackDamage = 25.0f;
    DetectionRange = 1200.0f;
    AttackCooldown = 2.0f;
    bCanAttack = true;
    PatrolOrigin = FVector::ZeroVector;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record patrol origin at spawn location
    PatrolOrigin = GetActorLocation();

    // Start idle/patrol loop
    GetWorldTimerManager().SetTimer(
        StateUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        1.0f,
        true
    );

    // Pick first patrol target
    PickNewPatrolTarget();

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: BeginPlay — Species=%s, Health=%.0f"),
        *GetName(), *SpeciesName, Health);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentState)
    {
        case EDinoState::Idle:
            TickIdle(DeltaTime);
            break;
        case EDinoState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case EDinoState::Chase:
            TickChase(DeltaTime);
            break;
        case EDinoState::Attack:
            TickAttack(DeltaTime);
            break;
        case EDinoState::Flee:
            TickFlee(DeltaTime);
            break;
        case EDinoState::Dead:
            // No tick when dead
            break;
    }
}

// ─── STATE MACHINE ────────────────────────────────────────────────────────────

void ADinosaurBase::UpdateBehaviorState()
{
    if (CurrentState == EDinoState::Dead) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    // Determine new state based on distance and species behaviour
    if (DistToPlayer <= AttackRange && CurrentState != EDinoState::Dead)
    {
        SetState(EDinoState::Attack);
    }
    else if (DistToPlayer <= DetectionRange && bIsAggressive)
    {
        SetState(EDinoState::Chase);
    }
    else if (CurrentState == EDinoState::Chase && DistToPlayer > DetectionRange * 1.5f)
    {
        // Lost the player — return to patrol
        SetState(EDinoState::Patrol);
        PickNewPatrolTarget();
    }
    else if (CurrentState == EDinoState::Idle)
    {
        // Occasionally start patrolling from idle
        if (FMath::RandBool())
        {
            SetState(EDinoState::Patrol);
            PickNewPatrolTarget();
        }
    }
}

void ADinosaurBase::SetState(EDinoState NewState)
{
    if (CurrentState == NewState) return;

    EDinoState OldState = CurrentState;
    CurrentState = NewState;

    // Adjust movement speed per state
    switch (NewState)
    {
        case EDinoState::Idle:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        case EDinoState::Patrol:
            GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
            break;
        case EDinoState::Chase:
        case EDinoState::Flee:
            GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
            break;
        case EDinoState::Attack:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        case EDinoState::Dead:
            GetCharacterMovement()->DisableMovement();
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GetMesh()->SetSimulatePhysics(true);
            break;
    }

    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase [%s]: State %d → %d"),
        *GetName(), (int32)OldState, (int32)NewState);
}

// ─── TICK IMPLEMENTATIONS ─────────────────────────────────────────────────────

void ADinosaurBase::TickIdle(float DeltaTime)
{
    // Idle: stand still, look around occasionally (handled by animation)
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    // Move toward current patrol target
    FVector ToTarget = PatrolTarget - GetActorLocation();
    float Dist = ToTarget.Size();

    if (Dist < 150.0f)
    {
        // Reached patrol target — pick a new one or go idle briefly
        if (FMath::RandRange(0, 2) == 0)
        {
            SetState(EDinoState::Idle);
        }
        else
        {
            PickNewPatrolTarget();
        }
        return;
    }

    // Move toward target
    FVector Direction = ToTarget.GetSafeNormal();
    AddMovementInput(Direction, 1.0f);
}

void ADinosaurBase::TickChase(float DeltaTime)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
    {
        SetState(EDinoState::Patrol);
        return;
    }

    FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    float Dist = ToPlayer.Size();

    if (Dist <= AttackRange)
    {
        SetState(EDinoState::Attack);
        return;
    }

    FVector Direction = ToPlayer.GetSafeNormal();
    AddMovementInput(Direction, 1.0f);
}

void ADinosaurBase::TickAttack(float DeltaTime)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
    {
        SetState(EDinoState::Patrol);
        return;
    }

    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    // Face the player
    FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator TargetRot = ToPlayer.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f));

    if (Dist > AttackRange)
    {
        // Player escaped — chase
        SetState(EDinoState::Chase);
        return;
    }

    // Perform attack if cooldown ready
    if (bCanAttack)
    {
        PerformAttack(Player);
    }
}

void ADinosaurBase::TickFlee(float DeltaTime)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return;

    // Move away from player
    FVector AwayFromPlayer = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
    AddMovementInput(AwayFromPlayer, 1.0f);

    // Stop fleeing if far enough
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    if (Dist > DetectionRange * 2.0f)
    {
        SetState(EDinoState::Patrol);
        PickNewPatrolTarget();
    }
}

// ─── COMBAT ───────────────────────────────────────────────────────────────────

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !bCanAttack) return;

    bCanAttack = false;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Attack on %s — Damage=%.0f"),
        *GetName(), *Target->GetName(), AttackDamage);

    // Reset attack cooldown
    GetWorldTimerManager().SetTimer(
        AttackCooldownTimer,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: TakeDamage=%.0f, Health=%.0f/%.0f"),
        *GetName(), ActualDamage, Health, MaxHealth);

    if (Health <= 0.0f)
    {
        Die();
    }
    else if (!bIsAggressive && Health < MaxHealth * 0.5f)
    {
        // Herbivores flee when wounded
        SetState(EDinoState::Flee);
    }
    else if (bIsAggressive && CurrentState != EDinoState::Chase && CurrentState != EDinoState::Attack)
    {
        // Predators retaliate
        SetState(EDinoState::Chase);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (CurrentState == EDinoState::Dead) return;

    SetState(EDinoState::Dead);

    // Stop all timers
    GetWorldTimerManager().ClearTimer(StateUpdateTimer);
    GetWorldTimerManager().ClearTimer(AttackCooldownTimer);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: DEAD — Species=%s"), *GetName(), *SpeciesName);

    // Despawn after 30 seconds (corpse persistence)
    GetWorldTimerManager().SetTimer(
        StateUpdateTimer,
        [this]() { Destroy(); },
        30.0f,
        false
    );
}

// ─── PATROL HELPERS ───────────────────────────────────────────────────────────

void ADinosaurBase::PickNewPatrolTarget()
{
    // Pick a random point within PatrolRadius of origin
    FVector RandomOffset = FMath::VRand() * FMath::RandRange(200.0f, PatrolRadius);
    RandomOffset.Z = 0.0f; // Keep on ground plane
    PatrolTarget = PatrolOrigin + RandomOffset;

    // Clamp to world bounds (basic)
    PatrolTarget.X = FMath::Clamp(PatrolTarget.X, PatrolOrigin.X - PatrolRadius, PatrolOrigin.X + PatrolRadius);
    PatrolTarget.Y = FMath::Clamp(PatrolTarget.Y, PatrolOrigin.Y - PatrolRadius, PatrolOrigin.Y + PatrolRadius);
}
