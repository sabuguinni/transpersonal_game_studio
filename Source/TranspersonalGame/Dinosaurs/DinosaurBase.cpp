// DinosaurBase.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260620_004
// Base class for all dinosaur pawns in TranspersonalGame.
// Inherits ACharacter for movement + capsule collision + skeletal mesh.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule — overridden per species in child class
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — overridden per species
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->JumpZVelocity = 0.0f;       // Dinosaurs don't jump by default
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
    }

    // Survival stats defaults
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    DetectionRadius = 1500.0f;
    bIsAggressive = false;
    bIsAlive = true;
    Species = EDinosaurSpecies::Unknown;
    BehaviorState = EDinosaurBehavior::Idle;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    BehaviorState = EDinosaurBehavior::Idle;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // AI behavior tick — child classes or BehaviorTree override this
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        bIsAlive = false;
        OnDeath();
    }
    else
    {
        // Switch to aggressive if hit and not already fleeing
        if (bIsAggressive && BehaviorState == EDinosaurBehavior::Idle)
        {
            BehaviorState = EDinosaurBehavior::Attacking;
        }
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EDinosaurBehavior::Dead;
    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }
    // Ragdoll — enable physics on mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    // Destroy after 10 seconds (cleanup)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    BehaviorState = NewState;
}

EDinosaurBehavior ADinosaurBase::GetBehaviorState() const
{
    return BehaviorState;
}

bool ADinosaurBase::IsAlive() const
{
    return bIsAlive;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
