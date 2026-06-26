// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of base dinosaur pawn class.
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in subclasses
    GetCapsuleComponent()->InitCapsuleSize(80.f, 180.f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->RotationRate = FRotator(0.f, Stats.TurnRate, 0.f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale = 1.5f;
        MoveComp->JumpZVelocity = 0.f; // Dinosaurs don't jump by default
        MoveComp->AirControl = 0.1f;
    }

    // No controller rotation yaw — movement component handles orientation
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

// -----------------------------------------------------------------------
// BeginPlay
// -----------------------------------------------------------------------

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialise health to max
    Stats.CurrentHealth = Stats.MaxHealth;

    // Apply walk speed from stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->RotationRate = FRotator(0.f, Stats.TurnRate, 0.f);
    }

    // Start in Idle state
    BehaviorState = EEng_DinoState::Idle;
    WanderTimer = FMath::RandRange(2.f, WanderInterval);
}

// -----------------------------------------------------------------------
// Tick
// -----------------------------------------------------------------------

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Advance attack cooldown
    TimeSinceLastAttack += DeltaTime;

    // Run behavior update
    UpdateBehavior(DeltaTime);
}

// -----------------------------------------------------------------------
// TakeDamage
// -----------------------------------------------------------------------

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Stats.CurrentHealth = FMath::Clamp(Stats.CurrentHealth - ActualDamage, 0.f, Stats.MaxHealth);

    // React to damage — switch to hunting or fleeing
    if (Stats.CurrentHealth <= 0.f)
    {
        Die();
    }
    else if (DamageCauser)
    {
        CurrentTarget = DamageCauser;
        if (Diet == EEng_DinoDiet::Herbivore || Stats.CurrentHealth < Stats.MaxHealth * 0.25f)
        {
            SetBehaviorState(EEng_DinoState::Fleeing);
        }
        else
        {
            SetBehaviorState(EEng_DinoState::Hunting);
        }
    }

    return ActualDamage;
}

// -----------------------------------------------------------------------
// SetBehaviorState
// -----------------------------------------------------------------------

void ADinosaurBase::SetBehaviorState(EEng_DinoState NewState)
{
    if (NewState == BehaviorState) return;

    const EEng_DinoState OldState = BehaviorState;
    BehaviorState = NewState;

    // Update movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (NewState == EEng_DinoState::Hunting || NewState == EEng_DinoState::Fleeing || NewState == EEng_DinoState::Attacking)
        {
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
        }
        else
        {
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        }
    }

    OnDinoStateChanged(OldState, NewState);
}

// -----------------------------------------------------------------------
// CanAttack
// -----------------------------------------------------------------------

bool ADinosaurBase::CanAttack() const
{
    return !bIsDead && TimeSinceLastAttack >= Stats.AttackCooldown;
}

// -----------------------------------------------------------------------
// PerformAttack
// -----------------------------------------------------------------------

void ADinosaurBase::PerformAttack()
{
    if (!CanAttack()) return;
    if (!CurrentTarget) return;

    TimeSinceLastAttack = 0.f;

    // Apply damage to target
    if (IsTargetInRange(CurrentTarget, Stats.AttackRange))
    {
        FDamageEvent DamageEvent;
        CurrentTarget->TakeDamage(Stats.AttackDamage, DamageEvent, GetController(), this);
        OnDinoAttack();
    }
}

// -----------------------------------------------------------------------
// UpdateBehavior
// -----------------------------------------------------------------------

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    switch (BehaviorState)
    {
        case EEng_DinoState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case EEng_DinoState::Wandering:
            HandleWanderingState(DeltaTime);
            break;
        case EEng_DinoState::Hunting:
        case EEng_DinoState::Foraging:
            HandleHuntingState(DeltaTime);
            break;
        case EEng_DinoState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case EEng_DinoState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------
// FindNearestThreat
// -----------------------------------------------------------------------

AActor* ADinosaurBase::FindNearestThreat() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), OverlappingActors);

    AActor* Nearest = nullptr;
    float NearestDist = Stats.DetectionRadius;

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this) continue;
        const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            Nearest = Actor;
        }
    }

    return Nearest;
}

// -----------------------------------------------------------------------
// FindNearestPrey
// -----------------------------------------------------------------------

AActor* ADinosaurBase::FindNearestPrey() const
{
    // For carnivores: find nearest pawn that is not the same species
    if (Diet != EEng_DinoDiet::Carnivore) return nullptr;
    return FindNearestThreat(); // Simplified: nearest pawn is prey
}

// -----------------------------------------------------------------------
// Die
// -----------------------------------------------------------------------

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    Stats.CurrentHealth = 0.f;
    SetBehaviorState(EEng_DinoState::Dead);

    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OnDinoDeath();

    // Destroy actor after 10 seconds (corpse lingers briefly)
    SetLifeSpan(10.f);
}

// -----------------------------------------------------------------------
// GetHealthPercent
// -----------------------------------------------------------------------

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

// -----------------------------------------------------------------------
// IsAlive
// -----------------------------------------------------------------------

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && Stats.CurrentHealth > 0.f;
}

// -----------------------------------------------------------------------
// Internal: HandleIdleState
// -----------------------------------------------------------------------

void ADinosaurBase::HandleIdleState(float DeltaTime)
{
    WanderTimer -= DeltaTime;
    if (WanderTimer <= 0.f)
    {
        SetBehaviorState(EEng_DinoState::Wandering);
        WanderTimer = FMath::RandRange(WanderInterval * 0.5f, WanderInterval * 1.5f);

        // Pick a random wander destination within 1000 units
        const FVector Offset(
            FMath::RandRange(-1000.f, 1000.f),
            FMath::RandRange(-1000.f, 1000.f),
            0.f
        );
        WanderDestination = GetActorLocation() + Offset;
    }

    // Passive threat detection
    if (Diet == EEng_DinoDiet::Carnivore)
    {
        AActor* Prey = FindNearestPrey();
        if (Prey)
        {
            CurrentTarget = Prey;
            SetBehaviorState(EEng_DinoState::Hunting);
        }
    }
}

// -----------------------------------------------------------------------
// Internal: HandleWanderingState
// -----------------------------------------------------------------------

void ADinosaurBase::HandleWanderingState(float DeltaTime)
{
    const float DistToDestination = FVector::Dist2D(GetActorLocation(), WanderDestination);

    if (DistToDestination < 150.f)
    {
        // Reached destination — go idle
        SetBehaviorState(EEng_DinoState::Idle);
        WanderTimer = FMath::RandRange(3.f, 8.f);
        return;
    }

    // Move toward wander destination
    const FVector Direction = (WanderDestination - GetActorLocation()).GetSafeNormal2D();
    AddMovementInput(Direction, 1.f);

    // Passive threat detection while wandering
    if (Diet == EEng_DinoDiet::Carnivore)
    {
        AActor* Prey = FindNearestPrey();
        if (Prey)
        {
            CurrentTarget = Prey;
            SetBehaviorState(EEng_DinoState::Hunting);
        }
    }
}

// -----------------------------------------------------------------------
// Internal: HandleHuntingState
// -----------------------------------------------------------------------

void ADinosaurBase::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget || !CurrentTarget->IsValidLowLevel())
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoState::Idle);
        return;
    }

    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Lost target (too far)
    if (DistToTarget > Stats.DetectionRadius * 1.5f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoState::Idle);
        return;
    }

    // Close enough to attack
    if (DistToTarget <= Stats.AttackRange)
    {
        SetBehaviorState(EEng_DinoState::Attacking);
        return;
    }

    // Chase target
    MoveTowardTarget(CurrentTarget, DeltaTime);
}

// -----------------------------------------------------------------------
// Internal: HandleFleeingState
// -----------------------------------------------------------------------

void ADinosaurBase::HandleFleeingState(float DeltaTime)
{
    if (!CurrentTarget || !CurrentTarget->IsValidLowLevel())
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoState::Idle);
        return;
    }

    const float DistToThreat = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Safe distance reached
    if (DistToThreat > Stats.DetectionRadius * 2.f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoState::Idle);
        return;
    }

    // Move away from threat
    const FVector AwayDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal2D();
    AddMovementInput(AwayDirection, 1.f);
}

// -----------------------------------------------------------------------
// Internal: HandleAttackingState
// -----------------------------------------------------------------------

void ADinosaurBase::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget || !CurrentTarget->IsValidLowLevel())
    {
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoState::Idle);
        return;
    }

    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Target moved out of attack range — resume hunting
    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        SetBehaviorState(EEng_DinoState::Hunting);
        return;
    }

    // Face target and attack
    const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    const FRotator LookRot = ToTarget.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookRot, DeltaTime, 5.f));

    PerformAttack();
}

// -----------------------------------------------------------------------
// Internal: IsTargetInRange
// -----------------------------------------------------------------------

bool ADinosaurBase::IsTargetInRange(AActor* Target, float Range) const
{
    if (!Target) return false;
    return FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Range;
}

// -----------------------------------------------------------------------
// Internal: MoveTowardTarget
// -----------------------------------------------------------------------

void ADinosaurBase::MoveTowardTarget(AActor* Target, float DeltaTime)
{
    if (!Target) return;
    const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    AddMovementInput(Direction, 1.f);
}
