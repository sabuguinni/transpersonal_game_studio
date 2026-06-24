// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full state-machine implementation: Idle → Patrol → Chase → Attack → Flee
// Compiles against UE5.5 with zero warnings.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per-species in child BPs
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 96.0f);

    // Movement defaults
    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->MaxWalkSpeed          = PatrolSpeed;
    Move->bOrientRotationToMovement = true;
    Move->RotationRate          = FRotator(0.0f, 540.0f, 0.0f);
    Move->GravityScale          = 1.0f;
    Move->JumpZVelocity         = 0.0f;     // Dinos don't jump by default
    Move->AirControl            = 0.0f;

    bUseControllerRotationYaw   = false;

    // Survival stats
    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;
    CurrentState   = EDinoState::Idle;

    // Patrol
    PatrolIndex    = 0;
    StateTimer     = 0.0f;
    IdleDuration   = FMath::RandRange(3.0f, 8.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location for territory checks
    HomeLocation = GetActorLocation();

    // Seed patrol waypoints around home if none were set in editor
    if (PatrolWaypoints.Num() == 0)
    {
        GeneratePatrolWaypoints(4, 800.0f);
    }

    TransitionToState(EDinoState::Idle);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    StateTimer += DeltaTime;
    UpdateStamina(DeltaTime);

    switch (CurrentState)
    {
        case EDinoState::Idle:    TickIdle(DeltaTime);    break;
        case EDinoState::Patrol:  TickPatrol(DeltaTime);  break;
        case EDinoState::Chase:   TickChase(DeltaTime);   break;
        case EDinoState::Attack:  TickAttack(DeltaTime);  break;
        case EDinoState::Flee:    TickFlee(DeltaTime);    break;
        case EDinoState::Dead:                            break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STATE MACHINE
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::TransitionToState(EDinoState NewState)
{
    if (CurrentState == EDinoState::Dead) return;

    CurrentState = NewState;
    StateTimer   = 0.0f;

    UCharacterMovementComponent* Move = GetCharacterMovement();

    switch (NewState)
    {
        case EDinoState::Idle:
            Move->MaxWalkSpeed = 0.0f;
            IdleDuration = FMath::RandRange(3.0f, 8.0f);
            break;

        case EDinoState::Patrol:
            Move->MaxWalkSpeed = PatrolSpeed;
            AdvancePatrolTarget();
            break;

        case EDinoState::Chase:
            Move->MaxWalkSpeed = ChaseSpeed;
            break;

        case EDinoState::Attack:
            Move->MaxWalkSpeed = 0.0f;
            AttackCooldownRemaining = 0.0f;
            break;

        case EDinoState::Flee:
            Move->MaxWalkSpeed = ChaseSpeed * 1.2f;
            break;

        case EDinoState::Dead:
            Move->MaxWalkSpeed = 0.0f;
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            OnDeath();
            break;
    }

    OnStateChanged(NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickIdle(float DeltaTime)
{
    // After idle duration, begin patrol
    if (StateTimer >= IdleDuration)
    {
        TransitionToState(EDinoState::Patrol);
        return;
    }

    // Passive threat detection
    AActor* Threat = DetectThreat();
    if (Threat)
    {
        TargetActor = Threat;
        if (bIsHerbivore)
            TransitionToState(EDinoState::Flee);
        else
            TransitionToState(EDinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickPatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0)
    {
        TransitionToState(EDinoState::Idle);
        return;
    }

    FVector Target = PatrolWaypoints[PatrolIndex];
    FVector MyLoc  = GetActorLocation();
    float   Dist   = FVector::Dist2D(MyLoc, Target);

    // Move toward waypoint
    FVector Dir = (Target - MyLoc).GetSafeNormal2D();
    AddMovementInput(Dir, 1.0f);

    // Reached waypoint?
    if (Dist < 150.0f)
    {
        AdvancePatrolTarget();
        TransitionToState(EDinoState::Idle);
    }

    // Threat check during patrol
    AActor* Threat = DetectThreat();
    if (Threat)
    {
        TargetActor = Threat;
        if (bIsHerbivore)
            TransitionToState(EDinoState::Flee);
        else
            TransitionToState(EDinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickChase(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        TransitionToState(EDinoState::Patrol);
        return;
    }

    FVector MyLoc     = GetActorLocation();
    FVector TargetLoc = TargetActor->GetActorLocation();
    float   Dist      = FVector::Dist2D(MyLoc, TargetLoc);

    // Lost target — too far
    if (Dist > DetectionRadius * 1.5f)
    {
        TargetActor = nullptr;
        TransitionToState(EDinoState::Patrol);
        return;
    }

    // Close enough to attack
    if (Dist <= AttackRange)
    {
        TransitionToState(EDinoState::Attack);
        return;
    }

    // Move toward target
    FVector Dir = (TargetLoc - MyLoc).GetSafeNormal2D();
    AddMovementInput(Dir, 1.0f);

    // Stamina depletion while chasing
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * 2.0f * DeltaTime));
    if (CurrentStamina <= 0.0f)
    {
        TransitionToState(EDinoState::Idle); // Exhausted — give up
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickAttack(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        TransitionToState(EDinoState::Patrol);
        return;
    }

    FVector MyLoc  = GetActorLocation();
    float   Dist   = FVector::Dist2D(MyLoc, TargetActor->GetActorLocation());

    // Target moved out of range
    if (Dist > AttackRange * 1.5f)
    {
        TransitionToState(EDinoState::Chase);
        return;
    }

    // Face target
    FVector Dir = (TargetActor->GetActorLocation() - MyLoc).GetSafeNormal2D();
    FRotator LookAt = Dir.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAt, DeltaTime, 8.0f));

    // Attack cooldown
    AttackCooldownRemaining -= DeltaTime;
    if (AttackCooldownRemaining <= 0.0f)
    {
        PerformAttack();
        AttackCooldownRemaining = AttackCooldown;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickFlee(float DeltaTime)
{
    if (!TargetActor || !IsValid(TargetActor))
    {
        TransitionToState(EDinoState::Patrol);
        return;
    }

    FVector MyLoc     = GetActorLocation();
    FVector ThreatLoc = TargetActor->GetActorLocation();
    float   Dist      = FVector::Dist2D(MyLoc, ThreatLoc);

    // Safe distance reached
    if (Dist > DetectionRadius * 2.0f)
    {
        TargetActor = nullptr;
        TransitionToState(EDinoState::Idle);
        return;
    }

    // Move away from threat
    FVector FleeDir = (MyLoc - ThreatLoc).GetSafeNormal2D();
    AddMovementInput(FleeDir, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// COMBAT
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::PerformAttack()
{
    if (!TargetActor) return;

    // Apply damage to target
    float Damage = FMath::RandRange(AttackDamage * 0.8f, AttackDamage * 1.2f);
    UGameplayStatics::ApplyDamage(TargetActor, Damage, GetController(), this, nullptr);

    OnAttackPerformed(Damage);
}

void ADinosaurBase::TakeDamageFromSource(float DamageAmount, AActor* DamageSource)
{
    if (CurrentState == EDinoState::Dead) return;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        TransitionToState(EDinoState::Dead);
        return;
    }

    // React to damage
    if (bIsHerbivore)
    {
        TargetActor = DamageSource;
        TransitionToState(EDinoState::Flee);
    }
    else
    {
        TargetActor = DamageSource;
        TransitionToState(EDinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DETECTION
// ─────────────────────────────────────────────────────────────────────────────

AActor* ADinosaurBase::DetectThreat() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Player) return nullptr;

    float Dist = FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
    if (Dist <= DetectionRadius)
    {
        return Player;
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// STAMINA
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::UpdateStamina(float DeltaTime)
{
    if (CurrentState == EDinoState::Chase || CurrentState == EDinoState::Flee)
    {
        // Drain handled per-state
        return;
    }

    // Recover stamina when not sprinting
    CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
}

// ─────────────────────────────────────────────────────────────────────────────
// PATROL HELPERS
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::GeneratePatrolWaypoints(int32 Count, float Radius)
{
    PatrolWaypoints.Empty();
    for (int32 i = 0; i < Count; ++i)
    {
        float Angle = (360.0f / Count) * i;
        float Rad   = FMath::DegreesToRadians(Angle);
        FVector WP  = HomeLocation + FVector(FMath::Cos(Rad) * Radius, FMath::Sin(Rad) * Radius, 0.0f);
        PatrolWaypoints.Add(WP);
    }
    PatrolIndex = 0;
}

void ADinosaurBase::AdvancePatrolTarget()
{
    if (PatrolWaypoints.Num() == 0) return;
    PatrolIndex = (PatrolIndex + 1) % PatrolWaypoints.Num();
}

// ─────────────────────────────────────────────────────────────────────────────
// BLUEPRINT EVENTS (empty native implementations — override in BP)
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurBase::OnStateChanged_Implementation(EDinoState NewState) {}
void ADinosaurBase::OnAttackPerformed_Implementation(float DamageDealt) {}
void ADinosaurBase::OnDeath_Implementation() {}
