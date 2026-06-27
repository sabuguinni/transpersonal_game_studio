// DinosaurBase.cpp — Base class for all dinosaur actors
// Engine Architect #02 — Transpersonal Game Studio

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 400.f;
    GetCharacterMovement()->JumpZVelocity = 400.f;

    // Species defaults
    Species = EDinoSpecies::Raptor;
    MaxHealth = 100.f;
    CurrentHealth = 100.f;
    AttackDamage = 20.f;
    AttackRange = 150.f;
    DetectionRadius = 1500.f;
    bIsAggressive = false;
    bIsAlive = true;
    CurrentBehaviorState = EDinoBehaviorState::Idle;

    // Patrol defaults
    PatrolRadius = 1000.f;
    PatrolWaitTime = 3.f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    HomeLocation = GetActorLocation();

    // Start idle behavior loop
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehavior,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!bIsAlive) return 0.f;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        Die();
    }
    else if (bIsAggressive && EventInstigator)
    {
        // React to attacker
        CurrentBehaviorState = EDinoBehaviorState::Attacking;
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    bIsAlive = false;
    CurrentBehaviorState = EDinoBehaviorState::Dead;
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll on skeletal mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Destroy after 10 seconds
    SetLifeSpan(10.f);
}

void ADinosaurBase::UpdateBehavior()
{
    if (!bIsAlive) return;

    switch (CurrentBehaviorState)
    {
        case EDinoBehaviorState::Idle:
            // Occasionally start patrolling
            if (FMath::RandBool())
            {
                CurrentBehaviorState = EDinoBehaviorState::Patrolling;
            }
            break;

        case EDinoBehaviorState::Patrolling:
            ChooseNewPatrolPoint();
            break;

        case EDinoBehaviorState::Fleeing:
            // Return to idle if far enough from threat
            CurrentBehaviorState = EDinoBehaviorState::Idle;
            break;

        default:
            break;
    }
}

void ADinosaurBase::ChooseNewPatrolPoint()
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.f
    );
    PatrolTargetLocation = HomeLocation + RandomOffset;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.f) return 0.f;
    return CurrentHealth / MaxHealth;
}
