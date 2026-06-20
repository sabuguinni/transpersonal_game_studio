// DinosaurBase.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260620_006
// Base class for all dinosaur pawns in the Transpersonal Game.
// Implements survival-realistic dinosaur behaviour: territory, hunger, threat response.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child BPs
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — realistic dino speeds (m/s → cm/s)
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 600.0f;       // 6 m/s default walk
        MoveComp->MaxWalkSpeedCrouched = 300.0f;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // Survival stats — realistic defaults
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 80.0f;
    HungerDrainRate = 0.5f;   // per second
    TerritoryRadius = 2000.0f; // 20m default territory
    DetectionRadius = 1500.0f;
    AttackRange = 200.0f;
    AttackDamage = 25.0f;
    bIsAggressive = false;
    bIsPredator = false;
    CurrentBehaviourState = EDinosaurBehaviourState::Idle;
    DinosaurSpecies = EDinosaurSpecies::Generic;
    MoveSpeed_Walk = 300.0f;
    MoveSpeed_Run = 700.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        1.0f,   // every 1 second
        true    // looping
    );

    // Start behaviour tick
    GetWorldTimerManager().SetTimer(
        BehaviourTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviour,
        0.5f,   // every 500ms
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

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (ActualDamage > 0.0f)
    {
        // Become aggressive when attacked
        bIsAggressive = true;
        SetBehaviourState(EDinosaurBehaviourState::Attacking);
    }

    return ActualDamage;
}

void ADinosaurBase::DrainHunger()
{
    CurrentHunger = FMath::Clamp(CurrentHunger - HungerDrainRate, 0.0f, MaxHunger);

    // Starving dino becomes more aggressive
    if (CurrentHunger < 20.0f && bIsPredator)
    {
        bIsAggressive = true;
    }
}

void ADinosaurBase::UpdateBehaviour()
{
    if (CurrentHealth <= 0.0f) return;

    switch (CurrentBehaviourState)
    {
        case EDinosaurBehaviourState::Idle:
            // Patrol or graze — implemented in child classes / BT
            break;

        case EDinosaurBehaviourState::Patrolling:
            // Movement handled by AI controller
            break;

        case EDinosaurBehaviourState::Fleeing:
            // Flee from threat — set max speed
            if (UCharacterMovementComponent* MC = GetCharacterMovement())
            {
                MC->MaxWalkSpeed = MoveSpeed_Run;
            }
            break;

        case EDinosaurBehaviourState::Attacking:
            if (UCharacterMovementComponent* MC = GetCharacterMovement())
            {
                MC->MaxWalkSpeed = MoveSpeed_Run;
            }
            break;

        case EDinosaurBehaviourState::Resting:
            if (UCharacterMovementComponent* MC = GetCharacterMovement())
            {
                MC->MaxWalkSpeed = 0.0f;
            }
            break;

        default:
            break;
    }
}

void ADinosaurBase::SetBehaviourState(EDinosaurBehaviourState NewState)
{
    if (CurrentBehaviourState == NewState) return;
    CurrentBehaviourState = NewState;
}

void ADinosaurBase::OnDeath()
{
    // Stop timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviourTimerHandle);

    // Disable movement
    if (UCharacterMovementComponent* MC = GetCharacterMovement())
    {
        MC->DisableMovement();
    }

    // Disable collision on capsule, keep mesh for ragdoll
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Enable ragdoll on skeletal mesh
    if (USkeletalMeshComponent* Mesh = GetMesh())
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

bool ADinosaurBase::IsHungry() const
{
    return CurrentHunger < 30.0f;
}
